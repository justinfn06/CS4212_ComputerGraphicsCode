"""One-time migration of the version 3 generator."""
from pathlib import Path
p=Path(__file__).with_name('build_model.py')
s=p.read_text()
a=s.index('# EDIT THESE VALUES.'); b=s.index('\ndef cross',a)
s=s[:a]+'''# ====================== EDITABLE SETTINGS ======================
# All lengths/widths/thicknesses: mm. Angles: degrees, counterclockwise
# from +X (right). XY is the bed; Z is front-to-back thickness.
DEFAULTS = {
    'shaft_length': 75.0,
    'shaft_width': 4.0,
    'spring_width': 1.8,
    'head_width': 70.0,
    'head_height': 80.0,
    'body_thickness': 10.0,
    'return_width': 4.5,
    'foot_length': 18.0,
    'foot_width': 7.0,
    'shaft_angle': math.degrees(math.atan2(-542*80/319, -27*70/270)),
    'foot_angle': 180.0,
    'head_rotation': 0.0,
    'end_bar_length_scale': 1.0,
    'end_bar_angle_offset': 0.0,
}

# The drawing frame is normalized: u=0..1 left to right; v=0..1 bottom
# to top. Values outside 0..1 are allowed. Actual position before rotation:
# x = u * head_width; y = v * head_height.
# Fractions below retain the traced proportions; replace with decimals.
HEAD_START = (253/270, 132/319)  # S0: shaft joins the outer curl here.

# Seven cubic Bezier segments, S1..S7. Each starts at the previous end.
# c1/c2 move the two curve handles; end moves the segment endpoint.
# width_key chooses a DEFAULTS width; width_mm overrides it if not None.
# thickness_mm overrides body_thickness locally at this endpoint.
# steps controls smoothness. Add/remove segments to change the curl count.
CURVE_SEGMENTS = [
    dict(c1=(270/270,209/319), c2=(244/270,272/319), end=(195/270,300/319), width_key='spring_width', width_mm=None, thickness_mm=None, steps=32),
    dict(c1=(149/270,331/319), c2=(99/270,319/319), end=(64/270,299/319), width_key='spring_width', width_mm=None, thickness_mm=None, steps=32),
    dict(c1=(21/270,274/319), c2=(-3/270,235/319), end=(0/270,187/319), width_key='spring_width', width_mm=None, thickness_mm=None, steps=32),
    dict(c1=(0/270,141/319), c2=(24/270,112/319), end=(67/270,109/319), width_key='spring_width', width_mm=None, thickness_mm=None, steps=32),
    dict(c1=(108/270,104/319), c2=(138/270,126/319), end=(142/270,159/319), width_key='spring_width', width_mm=None, thickness_mm=None, steps=32),
    dict(c1=(148/270,185/319), c2=(132/270,203/319), end=(114/270,196/319), width_key='spring_width', width_mm=None, thickness_mm=None, steps=32),
    dict(c1=(94/270,191/319), c2=(93/270,174/319), end=(100/270,162/319), width_key='return_width', width_mm=None, thickness_mm=None, steps=20),
]

# Inner angular corners R1..R4 and free end R5. Edit each xy independently.
# Add/remove points for more/fewer corners. Optional c1/c2 keys turn a
# straight segment into a cubic curve (same coordinate system as above).
RETURN_POINTS = [
    dict(xy=(76/270,148/319), width_mm=None, thickness_mm=None),
    dict(xy=(41/270,186/319), width_mm=None, thickness_mm=None),
    dict(xy=(147/270,260/319), width_mm=None, thickness_mm=None),
    dict(xy=(213/270,144/319), width_mm=None, thickness_mm=None),
    dict(xy=(46/270,14/319), width_mm=None, thickness_mm=None),
]

# Optional absolute local overrides. None inherits the named global value.
LOCAL = {
    'shaft_base_width_mm': None,    # Inherits shaft_width.
    'shaft_top_width_mm': None,     # Inherits shaft_width.
    'foot_thickness_mm': None,      # Inherits body_thickness.
    'shaft_base_thickness_mm': None,
    'shaft_top_thickness_mm': None,
}
OPTIONS = {
    'width_blend': 'smooth',       # 'smooth' or 'linear' on curve segments.
    'thickness_blend': 'smooth',   # 'smooth' or 'linear' on curve segments.
    'end_caps': 'butt',            # 'butt' or 'square' (extends tips half-width).
    'output_name': 'swirl_item_v4',
    'write_preview': True,
    'write_diagrams': True,
}
# ===================== END EDITABLE SETTINGS ====================
OUT = Path(__file__).resolve().parent
''' +s[b:]
a=s.index('def make_profile('); b=s.index('def mesh(',a)
s=s[:a]+'''def centreline(p):
    def xy(uv):
        x,y=uv[0]*p['head_width'],uv[1]*p['head_height']
        r=math.radians(p['head_rotation'])
        return (x*math.cos(r)-y*math.sin(r),x*math.sin(r)+y*math.cos(r))
    def value(v,default):
        v=default if v is None else v
        if not math.isfinite(v) or v<=0: raise ValueError('Local widths and thicknesses must be positive.')
        return v
    z=p['body_thickness']; hx,hy=xy(HEAD_START)
    r=math.radians(p['shaft_angle'])
    bx,by=hx+math.cos(r)*p['shaft_length'],hy+math.sin(r)*p['shaft_length']
    r=math.radians(p['foot_angle'])
    samples=[(bx+math.cos(r)*p['foot_length'],by+math.sin(r)*p['foot_length'],p['foot_width'],value(LOCAL['foot_thickness_mm'],z)),
             (bx,by,value(LOCAL['shaft_base_width_mm'],p['shaft_width']),value(LOCAL['shaft_base_thickness_mm'],z)),
             (hx,hy,value(LOCAL['shaft_top_width_mm'],p['shaft_width']),value(LOCAL['shaft_top_thickness_mm'],z))]
    marks={'F':samples[0][:2],'B':samples[1][:2],'S0':(hx,hy)}
    handles=[]
    def blend(t,kind): return t*t*(3-2*t) if kind=='smooth' else t
    def bez(c1,c2,end,width,depth,count):
        if not isinstance(count,int) or count<2: raise ValueError('Curve steps must be an integer >= 2.')
        x,y,w,z0=samples[-1]
        handles.append(((x,y),c1,c2,end))
        for i in range(1,count+1):
            t=i/count; u=1-t
            samples.append((u**3*x+3*u*u*t*c1[0]+3*u*t*t*c2[0]+t**3*end[0],
                            u**3*y+3*u*u*t*c1[1]+3*u*t*t*c2[1]+t**3*end[1],
                            w+(width-w)*blend(t,OPTIONS['width_blend']),
                            z0+(depth-z0)*blend(t,OPTIONS['thickness_blend'])))
    for i,seg in enumerate(CURVE_SEGMENTS,1):
        end=xy(seg['end'])
        bez(xy(seg['c1']),xy(seg['c2']),end,value(seg['width_mm'],p[seg['width_key']]),value(seg['thickness_mm'],z),seg['steps'])
        marks[f'S{i}']=end
    for i,point in enumerate(RETURN_POINTS,1):
        end=xy(point['xy'])
        if i==len(RETURN_POINTS):
            start=samples[-1][:2]; dx=end[0]-start[0]; dy=end[1]-start[1]
            angle=math.atan2(dy,dx)+math.radians(p['end_bar_angle_offset'])
            length=math.hypot(dx,dy)*p['end_bar_length_scale']
            end=(start[0]+length*math.cos(angle),start[1]+length*math.sin(angle))
        width=value(point.get('width_mm'),p['return_width']); depth=value(point.get('thickness_mm'),z)
        if 'c1' in point and 'c2' in point:
            bez(xy(point['c1']),xy(point['c2']),end,width,depth,point.get('steps',32))
        else: samples.append((*end,width,depth))
        marks[f'R{i}']=end
    if OPTIONS['end_caps']=='square':
        for i,j in [(0,1),(-1,-2)]:
            x,y,w,z0=samples[i]; dx=x-samples[j][0]; dy=y-samples[j][1]; d=math.hypot(dx,dy)
            samples[i]=(x+dx/d*w/2,y+dy/d*w/2,w,z0)
    return samples,marks,handles

def geometry(p):
    samples,marks,handles=centreline(p)
    left=[]; right=[]; heights=[]
    for i,(x,y,w,z) in enumerate(samples):
        normals=[]
        for a,b in [(max(0,i-1),i),(i,min(len(samples)-1,i+1))]:
            if a==b: continue
            dx=samples[b][0]-samples[a][0]; dy=samples[b][1]-samples[a][1]
            d=math.hypot(dx,dy)
            if d<1e-9: raise ValueError('Coincident centreline points. Separate the editable endpoints.')
            normals.append((-dy/d,dx/d))
        nx=sum(v[0] for v in normals); ny=sum(v[1] for v in normals)
        d=math.hypot(nx,ny)
        if d<1e-9: raise ValueError('A corner doubles back 180 degrees.')
        nx/=d; ny/=d
        factor=w/2/(nx*normals[0][0]+ny*normals[0][1])
        left.append((x+nx*factor,y+ny*factor)); right.append((x-nx*factor,y-ny*factor)); heights.append(z)
    poly=left+right[::-1]; heights=heights+heights[::-1]
    minx=min(x for x,y in poly); miny=min(y for x,y in poly)
    poly=[(x-minx,y-miny) for x,y in poly]
    marks={k:(x-minx,y-miny) for k,(x,y) in marks.items()}
    handles=[tuple((x-minx,y-miny) for x,y in h) for h in handles]
    area=sum(poly[i][0]*poly[(i+1)%len(poly)][1]-poly[(i+1)%len(poly)][0]*poly[i][1] for i in range(len(poly)))/2
    if area<0: poly.reverse(); heights.reverse()
    n=len(poly)
    def on(a,b,p):
        return abs(cross(a,b,p))<1e-9 and min(a[0],b[0])-1e-9<=p[0]<=max(a[0],b[0])+1e-9 and min(a[1],b[1])-1e-9<=p[1]<=max(a[1],b[1])+1e-9
    for i in range(n):
        a,b=poly[i],poly[(i+1)%n]
        for j in range(i+2,n):
            if i==0 and j==n-1: continue
            c,d=poly[j],poly[(j+1)%n]
            if (cross(a,b,c)*cross(a,b,d)<0 and cross(c,d,a)*cross(c,d,b)<0) or any([on(a,b,c),on(a,b,d),on(c,d,a),on(c,d,b)]):
                raise ValueError('Profile crosses or touches itself. Adjust points, widths or head size. No files exported.')
    return poly,abs(area),heights,marks,handles

def make_profile(p):
    poly,area,*_=geometry(p)
    return poly,area

''' +s[b:]
s=s.replace("vertices=[(x,y,z) for z in (0.,depth) for x,y in poly]", "heights=[depth]*n if isinstance(depth,(int,float)) else depth\n    vertices=[(x,y,0.) for x,y in poly]+[(x,y,z) for (x,y),z in zip(poly,heights)]")
s=s.replace("    return vertices,faces,volume", "    expected=sum(cross(poly[a],poly[b],poly[c])*(heights[a]+heights[b]+heights[c])/6 for a,b,c in triangles)\n    assert abs(volume-expected)<1e-5\n    return vertices,faces,volume")
s=s.replace('def preview(poly,p,path):','def preview(poly,p,path,heights=None):')
s=s.replace('version 3','version 4')
s=s.replace("    w=max(x for x,y in poly); h=max(y for x,y in poly)", "    w=max(x for x,y in poly); h=max(y for x,y in poly)\n    heights=heights or [p['body_thickness']]*len(poly)")
s=s.replace('(j,p[\'body_thickness\']),(i,p[\'body_thickness\'])', '(j,heights[j]),(i,heights[i])')
s=s.replace("[project(x,y,p['body_thickness']) for x,y in poly]", "[project(x,y,z) for (x,y),z in zip(poly,heights)]")
s=s.replace("    if not all(math.isfinite(v) and v>0 for v in p.values()):", "    angles={'shaft_angle','foot_angle','head_rotation','end_bar_angle_offset'}\n    if not all(math.isfinite(v) and (k in angles or v>0) for k,v in p.items()):")
s=s.replace("    poly,area=make_profile(p); vertices,faces,volume=mesh(poly,p['body_thickness'])\n    assert abs(volume-area*p['body_thickness'])<1e-5", "    if OPTIONS['width_blend'] not in ('smooth','linear') or OPTIONS['thickness_blend'] not in ('smooth','linear'):\n        raise ValueError('Blend must be smooth or linear.')\n    if OPTIONS['end_caps'] not in ('butt','square'): raise ValueError('End caps must be butt or square.')\n    poly,area,heights,marks,handles=geometry(p)\n    vertices,faces,volume=mesh(poly,heights)")
s=s.replace("    scad+=f'linear_extrude(height={p[\"body_thickness\"]}) polygon(points='+json.dumps(poly)+');\\n'", "    scad+='polyhedron(points='+json.dumps(vertices)+', faces='+json.dumps([list(reversed(f)) for f in faces])+', convexity=10);\\n'")
s=s.replace("'settings_mm':p", "'settings':p,'curve_segments':CURVE_SEGMENTS,'return_points':RETURN_POINTS,'head_start':HEAD_START,'local_overrides':LOCAL,'options':OPTIONS")
s=s.replace("round(max(y for x,y in poly),3),p['body_thickness']]", "round(max(y for x,y in poly),3),max(heights)]")
s=s.replace("    preview(poly,p,OUT/f'{name}_preview.png')", "    if OPTIONS['write_preview']: preview(poly,p,OUT/f'{name}_preview.png',heights)\n    if OPTIONS['write_diagrams']:\n        from diagram import draw_diagrams\n        draw_diagrams(poly,marks,handles,p,OUT,name)")
s=s.replace("default='swirl_item_v3'", "default=OPTIONS['output_name']")
s=s.replace("    except ValueError as e: parser.error(str(e))", "    except (ValueError,KeyError,TypeError) as e: parser.error(str(e))")
p.write_text(s)
