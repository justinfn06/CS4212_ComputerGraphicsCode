"""Adjust DEFAULTS below, then run this file to export an STL (millimetres).

Optional overrides: python build_model.py --shaft-length 75 --shaft-width 4
    --spring-width 1.8 --body-thickness 10 --head-width 70 --head-height 80
No packages required for STL. Pillow is optional for the preview.
"""
import argparse
import json
import math
import struct
from collections import Counter
from pathlib import Path

# ====================== EDITABLE SETTINGS ======================
# All lengths/widths/thicknesses: mm. Angles: degrees, counterclockwise
# from +X (right). XY is the bed; Z is front-to-back thickness.
DEFAULTS = {
    'shaft_length': 150.0,
    'shaft_width': 2.0,
    'shaft_thickness': None, # Front-to-back Z of shaft ONLY. Set e.g. 12.0; None inherits body_thickness.
    'spring_width': 1.8,
    'head_width': 70.0,
    'head_height': 80.0,
    'body_thickness': 15.0,
    'return_width': 4.5,
    'foot_length': 28.0,
    'foot_width': 25.0,
    'foot_thickness': None, # Constant block thickness in Z; None inherits body_thickness.
    'foot_join_length': 3.0, # Shaft length used to blend from block Z to shaft Z.
    'shaft_angle': math.degrees(math.atan2(-542*80/319, -27*70/270)),
    'foot_angle': 180.0,
    'head_rotation': 0.0,
    'end_bar_length_scale': 1,
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
    'foot_thickness_mm': None,      # Legacy override for the WHOLE block; inherits foot_thickness.
    'shaft_base_thickness_mm': None, # Inherits shaft_thickness; overrides it for taper.
    'shaft_top_thickness_mm': None,  # Inherits shaft_thickness; overrides it for taper.
}
OPTIONS = {
    'width_blend': 'smooth',       # 'smooth' or 'linear' on curve segments.
    'thickness_blend': 'smooth',   # 'smooth' or 'linear' on curve segments.
    'end_caps': 'butt',            # 'butt' or 'square' for free END BAR tip; foot is a fixed block.
    'output_name': 'swirl_item_v6',
    'write_preview': True,
    'write_diagrams': True,
}
# ===================== END EDITABLE SETTINGS ====================
OUT = Path(__file__).resolve().parent

def cross(a, b, c):
    return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])

def centreline(p):
    def xy(uv):
        if len(uv)!=2 or not all(math.isfinite(v) for v in uv):
            raise ValueError('Coordinates must contain two finite values (u, v).')
        x,y=uv[0]*p['head_width'],uv[1]*p['head_height']
        r=math.radians(p['head_rotation'])
        return (x*math.cos(r)-y*math.sin(r),x*math.sin(r)+y*math.cos(r))
    def value(v,default):
        v=default if v is None else v
        if not math.isfinite(v) or v<=0: raise ValueError('Local widths and thicknesses must be positive.')
        return v
    z=p['body_thickness']; hx,hy=xy(HEAD_START)
    shaft_z=value(p.get('shaft_thickness'),z)
    r=math.radians(p['shaft_angle'])
    bx,by=hx+math.cos(r)*p['shaft_length'],hy+math.sin(r)*p['shaft_length']
    r=math.radians(p['foot_angle'])
    inset=min(p['foot_width']/2,p['foot_length']/2)
    samples=[(bx+math.cos(r)*(p['foot_length']-inset),by+math.sin(r)*(p['foot_length']-inset),p['foot_width'],value(LOCAL['foot_thickness_mm'],value(p.get('foot_thickness'),z))),
             (bx,by,value(LOCAL['shaft_base_width_mm'],p['shaft_width']),value(LOCAL['shaft_base_thickness_mm'],shaft_z)),
             (hx,hy,value(LOCAL['shaft_top_width_mm'],p['shaft_width']),value(LOCAL['shaft_top_thickness_mm'],shaft_z))]
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
        if ('c1' in point) != ('c2' in point):
            raise ValueError('A curved return segment needs both c1 and c2.')
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
        for i,j in [(-1,-2)]:
            x,y,w,z0=samples[i]; dx=x-samples[j][0]; dy=y-samples[j][1]; d=math.hypot(dx,dy)
            samples[i]=(x+dx/d*w/2,y+dy/d*w/2,w,z0)
    return samples,marks,handles

class Profile(list):
    """Outline with a constrained split so the foot top stays perfectly flat."""
    regions = None

def attach_block(left,right,heights,samples,p):
    """One-sided block: its rear face is collinear with the outer shaft edge."""
    angle=math.radians(p['foot_angle'])
    u=(math.cos(angle),math.sin(angle))
    # The outside is the shaft side opposite the direction of foot extension.
    left_is_inner=sum((left[0][i]-right[0][i])*u[i] for i in (0,1))>0
    outer=right if left_is_inner else left
    inner=left if left_is_inner else right
    anchor=outer[0]
    dv=(outer[1][0]-anchor[0],outer[1][1]-anchor[1])
    span=math.hypot(*dv); direction=(dv[0]/span,dv[1]/span)
    sine=abs(u[0]*direction[1]-u[1]*direction[0])
    if sine<1e-6:
        raise ValueError('Foot angle must not be parallel to the shaft.')
    # Width is perpendicular to the foot length. Grow only toward the head.
    root_distance=p['foot_width']/sine
    root_top=(anchor[0]+direction[0]*root_distance,anchor[1]+direction[1]*root_distance)
    root_t=root_distance/span
    tip_bottom=tuple(anchor[i]+u[i]*p['foot_length'] for i in (0,1))
    tip_top=tuple(root_top[i]+u[i]*p['foot_length'] for i in (0,1))
    # Intersect the inner shaft edge with the top long face of the block.
    v=(inner[1][0]-inner[0][0],inner[1][1]-inner[0][1])
    q=(root_top[0]-inner[0][0],root_top[1]-inner[0][1])
    det=v[0]*u[1]-v[1]*u[0]
    if abs(det)<1e-10: raise ValueError('Foot and shaft edges are parallel.')
    inner_t=(q[0]*u[1]-q[1]*u[0])/det
    inner_exit=tuple(inner[0][i]+inner_t*v[i] for i in (0,1))
    opening=sum((inner_exit[i]-root_top[i])*u[i] for i in (0,1))
    if not (0<opening<p['foot_length']):
        raise ValueError('Foot length must extend past the inner shaft edge at this angle.')
    if min(root_t,inner_t)<0 or max(root_t,inner_t)>=1:
        raise ValueError('Foot width is too large for the available shaft length.')
    foot_z=LOCAL['foot_thickness_mm']
    if foot_z is None: foot_z=p.get('foot_thickness')
    if foot_z is None: foot_z=p['body_thickness']
    t=max(root_t,inner_t)+p['foot_join_length']/p['shaft_length']
    if t>=1: raise ValueError('Reduce foot width/join length or increase shaft_length.')
    def interp(a,b,t): return tuple(x+(y-x)*t for x,y in zip(a,b))
    shaft_z=heights[0]+(heights[1]-heights[0])*t
    lp,rp=(inner_exit,root_top) if left_is_inner else (root_top,inner_exit)
    left=[lp,interp(left[0],left[1],t)]+left[1:]
    right=[rp,interp(right[0],right[1],t)]+right[1:]
    heights=[foot_z,shaft_z]+heights[1:]
    path=[root_top,anchor,tip_bottom,tip_top,inner_exit]
    if not left_is_inner: path.reverse()
    poly=left+right[::-1]
    z=heights+heights[::-1]; count=len(poly)
    poly+=path[1:-1]; z += [foot_z]*(len(path)-2)
    regions=[list(range(count)),[0]+list(range(count-1,len(poly)))]
    return poly,z,regions

def geometry(p):
    samples,marks,handles=centreline(p)
    # Foot is a separate constant-section rectangle, not a tapered ribbon.
    samples=samples[1:]
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
    poly,heights,regions=attach_block(left,right,heights,samples,p)
    # Three added perimeter vertices: outer base and the two free-tip corners.
    angle=math.radians(p['foot_angle']); u=(math.cos(angle),math.sin(angle))
    added=poly[-3:]
    along=[sum(pt[i]*u[i] for i in (0,1)) for pt in added]
    tip_indices=sorted(range(3),key=lambda i:along[i],reverse=True)[:2]
    marks['F']=tuple(sum(added[j][i] for j in tip_indices)/2 for i in (0,1))
    minx=min(x for x,y in poly); miny=min(y for x,y in poly)
    poly=Profile((x-minx,y-miny) for x,y in poly)
    marks={k:(x-minx,y-miny) for k,(x,y) in marks.items()}
    handles=[tuple((x-minx,y-miny) for x,y in h) for h in handles]
    area=sum(poly[i][0]*poly[(i+1)%len(poly)][1]-poly[(i+1)%len(poly)][0]*poly[i][1] for i in range(len(poly)))/2
    if area<0:
        poly.reverse(); heights.reverse()
        regions=[[len(poly)-1-i for i in reversed(region)] for region in regions]
    poly.regions=regions
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

def mesh(poly,depth):
    n=len(poly); pending=[list(r) for r in (getattr(poly,'regions',None) or [range(n)])]
    triangles=[]
    for ids in pending:
        triangles.extend(triangulate(poly,ids))
    return mesh_from_triangles(poly,depth,triangles)

def triangulate(poly,ids):
    triangles=[]
    while len(ids)>3:
        for k,b in enumerate(ids):
            a=ids[k-1]; c=ids[(k+1)%len(ids)]
            if cross(poly[a],poly[b],poly[c])<=1e-10: continue
            if any(cross(poly[a],poly[b],poly[p])>=-1e-10 and
                   cross(poly[b],poly[c],poly[p])>=-1e-10 and
                   cross(poly[c],poly[a],poly[p])>=-1e-10
                   for p in ids if p not in (a,b,c)): continue
            triangles.append((a,b,c)); ids.pop(k); break
        else: raise ValueError('Cannot triangulate these dimensions. No files exported.')
    triangles.append(tuple(ids))
    return triangles

def mesh_from_triangles(poly,depth,triangles):
    n=len(poly)
    heights=[depth]*n if isinstance(depth,(int,float)) else depth
    vertices=[(x,y,0.) for x,y in poly]+[(x,y,z) for (x,y),z in zip(poly,heights)]
    faces=[(c,b,a) for a,b,c in triangles]+[(a+n,b+n,c+n) for a,b,c in triangles]
    for a in range(n):
        b=(a+1)%n; faces += [(a,b,b+n),(a,b+n,a+n)]
    edges=Counter(tuple(sorted((f[i],f[(i+1)%3]))) for f in faces for i in range(3))
    assert all(v==2 for v in edges.values()), 'Non-manifold mesh'
    directed=Counter((f[i],f[(i+1)%3]) for f in faces for i in range(3))
    assert all(directed[b,a]==v for (a,b),v in directed.items())
    assert len(vertices)-len(edges)+len(faces)==2
    volume=0.
    for a,b,c in faces:
        p,q,r=vertices[a],vertices[b],vertices[c]
        volume+=(p[0]*(q[1]*r[2]-q[2]*r[1])+p[1]*(q[2]*r[0]-q[0]*r[2])+p[2]*(q[0]*r[1]-q[1]*r[0]))/6
    expected=sum(cross(poly[a],poly[b],poly[c])*(heights[a]+heights[b]+heights[c])/6 for a,b,c in triangles)
    assert abs(volume-expected)<1e-5
    return vertices,faces,volume

def preview(poly,p,path,heights=None):
    try:
        from PIL import Image,ImageDraw,ImageFont
    except ImportError: return
    im=Image.new('RGB',(1400,1050),'#edf1f5'); d=ImageDraw.Draw(im)
    font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',23)
    title=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',34)
    w=max(x for x,y in poly); h=max(y for x,y in poly)
    heights=heights or [p['body_thickness']]*len(poly)
    d.text((40,25),'Adjustable spring swirl — version 6',fill='#192332',font=title)
    d.text((40,78),f'{w:.1f} × {h:.1f} mm footprint | {p["body_thickness"]:g} mm front-to-back thickness',fill='#526174',font=font)
    d.text((40,115),f'Shaft: {p["shaft_length"]:g} mm long / {p["shaft_width"]:g} mm wide    Spring strip: {p["spring_width"]:g} mm',fill='#526174',font=font)
    s=min(690/h,490/w)
    d.polygon([(70+x*s,890-y*s) for x,y in poly],fill='#304357')
    def project(x,y,z): return (780+s*(x+z*.65),865-s*y+s*.19*x-s*.38*z)
    d.polygon([project(x,y,0) for x,y in poly],fill='#192332')
    for i in sorted(range(len(poly)),key=lambda i:poly[i][0]+poly[i][1]):
        j=(i+1)%len(poly)
        d.polygon([project(*poly[k],z) for k,z in [(i,0),(j,0),(j,heights[j]),(i,heights[i])]],fill='#6b8299')
    d.polygon([project(x,y,z) for (x,y),z in zip(poly,heights)],fill='#304357')
    d.text((70,945),'Flat on bed (XY)',fill='#192332',font=font)
    d.text((780,945),'Angled view',fill='#192332',font=font)
    im.save(path)

def build(p,name):
    angles={'shaft_angle','foot_angle','head_rotation','end_bar_angle_offset'}
    if not all((k in ('shaft_thickness','foot_thickness') and v is None) or (math.isfinite(v) and (k in angles or v>0)) for k,v in p.items()):
        raise ValueError('All dimensions must be positive finite millimetres.')
    if OPTIONS['width_blend'] not in ('smooth','linear') or OPTIONS['thickness_blend'] not in ('smooth','linear'):
        raise ValueError('Blend must be smooth or linear.')
    if OPTIONS['end_caps'] not in ('butt','square'): raise ValueError('End caps must be butt or square.')
    poly,area,heights,marks,handles=geometry(p)
    vertices,faces,volume=mesh(poly,heights)
    blob=bytearray(b'Adjustable spring swirl. Units: mm'.ljust(80,b'\0'))
    blob.extend(struct.pack('<I',len(faces)))
    for face in faces:
        a,b,c=[vertices[i] for i in face]
        u=[b[i]-a[i] for i in range(3)]; v=[c[i]-a[i] for i in range(3)]
        normal=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]]
        norm=math.sqrt(sum(x*x for x in normal)); assert norm>1e-10
        blob.extend(struct.pack('<12fH',*[x/norm for x in normal],*a,*b,*c,0))
    # Validate the actual float32 STL representation, not just the source mesh.
    edges=Counter()
    for i in range(len(faces)):
        t=struct.unpack_from('<12fH',blob,84+50*i); a,b,c=t[3:6],t[6:9],t[9:12]
        assert len({a,b,c})==3
        edges.update([tuple(sorted((a,b))),tuple(sorted((b,c))),tuple(sorted((c,a)))])
    assert all(v==2 for v in edges.values())
    (OUT/f'{name}.stl').write_bytes(blob)
    scad='// Generated profile; use build_model.py for independent dimensional controls.\n'
    scad+='polyhedron(points='+json.dumps(vertices)+', faces='+json.dumps([list(reversed(f)) for f in faces])+', convexity=10);\n'
    (OUT/f'{name}.scad').write_text(scad)
    report={'settings':p,'curve_segments':CURVE_SEGMENTS,'return_points':RETURN_POINTS,'head_start':HEAD_START,'local_overrides':LOCAL,'options':OPTIONS,'actual_dimensions_mm':[round(max(x for x,y in poly),3),round(max(y for x,y in poly),3),max(heights)],
            'triangles':len(faces),'watertight':True,'consistent_winding':True,'connected_components':1,
            'outline_self_intersections':0,'volume_mm3':round(volume,3)}
    (OUT/f'{name}_settings.json').write_text(json.dumps(report,indent=2))
    if OPTIONS['write_preview']: preview(poly,p,OUT/f'{name}_preview.png',heights)
    if OPTIONS['write_diagrams']:
        from diagram import draw_diagrams
        draw_diagrams(poly,marks,handles,p,OUT,name)
    print(json.dumps({k:report[k] for k in ['actual_dimensions_mm','triangles','watertight','volume_mm3']},indent=2))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    for key,value in DEFAULTS.items():
        if key == 'body_thickness':
            parser.add_argument('--body-thickness', '--depth', dest=key, type=float, default=value,
                                help=f'Front-to-back thickness of the entire item (Z); default {value} mm')
        else:
            parser.add_argument('--'+key.replace('_','-'),type=float,default=value,help=f'mm; default {value}')
    parser.add_argument('--name',default=OPTIONS['output_name'],help='Output filename stem; use a new name to keep earlier trials')
    args=vars(parser.parse_args()); name=args.pop('name')
    if Path(name).name!=name or not name or any(c in name for c in '<>:"/\\|?*'):
        parser.error('--name must be a plain filename stem')
    try: build(args,name)
    except (ValueError,KeyError,TypeError) as e: parser.error(str(e))
