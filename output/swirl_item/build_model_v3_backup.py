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

# EDIT THESE VALUES. Widths change the flat XY outline.
# body_thickness changes front-to-back thickness of EVERY section (Z when flat).
DEFAULTS = {
    'shaft_length': 75.0,  # Centreline length from foot to start of outer curl.
    'shaft_width': 4.0,    # Long shaft, side to side on the bed.
    'spring_width': 1.8,   # Curved spring strip, side to side on the bed.
    'head_width': 70.0,    # Horizontal scale of swirl + angular return.
    'head_height': 80.0,   # Vertical scale of swirl + angular return on bed.
    'body_thickness': 10.0, # Front-to-back thickness of the WHOLE item; above bed (Z).
    'return_width': 4.5,   # Angular inner return and diagonal end bar.
    'foot_length': 18.0,   # Foot centreline length.
    'foot_width': 7.0,     # Foot side-to-side thickness.
}
OUT = Path(__file__).resolve().parent

def cross(a, b, c):
    return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])

def make_profile(p):
    # Source-photo coordinates preserve the original asymmetric outline.
    def xy(x,y):
        return ((x-156)*p['head_width']/270, (454-y)*p['head_height']/319)
    hx,hy=xy(409,322)
    dx,dy=-27*p['head_width']/270,-542*p['head_height']/319
    d=math.hypot(dx,dy)
    bx,by=hx+dx/d*p['shaft_length'],hy+dy/d*p['shaft_length']
    samples=[(bx-p['foot_length'],by,p['foot_width']),
             (bx,by,p['shaft_width']), (hx,hy,p['shaft_width'])]
    def bez(c1,c2,end,width,count=32):
        x,y,w=samples[-1]; c1=xy(*c1); c2=xy(*c2); end=xy(*end)
        for i in range(1,count+1):
            t=i/count; u=1-t
            # Smooth width blend avoids abrupt shoulder at spring root.
            blend=t*t*(3-2*t)
            samples.append((u**3*x+3*u*u*t*c1[0]+3*u*t*t*c2[0]+t**3*end[0],
                            u**3*y+3*u*u*t*c1[1]+3*u*t*t*c2[1]+t**3*end[1],
                            w+(width-w)*blend))
    sw=p['spring_width']
    bez((426,245),(400,182),(351,154),sw)
    bez((305,123),(255,135),(220,155),sw)
    bez((177,180),(153,219),(156,267),sw)
    bez((156,313),(180,342),(223,345),sw)
    bez((264,350),(294,328),(298,295),sw)
    bez((304,269),(288,251),(270,258),sw)
    bez((250,263),(249,280),(256,292),p['return_width'],20)
    samples += [(*xy(x,y),p['return_width']) for x,y in
                [(232,306),(197,268),(303,194),(369,310),(202,440)]]
    left=[]; right=[]
    for i,(x,y,w) in enumerate(samples):
        normals=[]
        for a,b in [(max(0,i-1),i),(i,min(len(samples)-1,i+1))]:
            if a==b: continue
            dx=samples[b][0]-samples[a][0]; dy=samples[b][1]-samples[a][1]
            d=math.hypot(dx,dy)
            if d<1e-9: raise ValueError('Coincident centreline points')
            normals.append((-dy/d,dx/d))
        nx=sum(v[0] for v in normals); ny=sum(v[1] for v in normals)
        d=math.hypot(nx,ny); nx/=d; ny/=d
        factor=w/2/(nx*normals[0][0]+ny*normals[0][1])
        left.append((x+nx*factor,y+ny*factor))
        right.append((x-nx*factor,y-ny*factor))
    poly=left+right[::-1]
    minx=min(x for x,y in poly); miny=min(y for x,y in poly)
    poly=[(x-minx,y-miny) for x,y in poly]
    area=sum(poly[i][0]*poly[(i+1)%len(poly)][1]-poly[(i+1)%len(poly)][0]*poly[i][1] for i in range(len(poly)))/2
    if area<0: poly.reverse()
    n=len(poly)
    for i in range(n):
        a,b=poly[i],poly[(i+1)%n]
        for j in range(i+2,n):
            if i==0 and j==n-1: continue
            c,d=poly[j],poly[(j+1)%n]
            if cross(a,b,c)*cross(a,b,d)<0 and cross(c,d,a)*cross(c,d,b)<0:
                raise ValueError('These settings make the profile cross itself. Reduce strip widths or increase head size. No files exported.')
    return poly,abs(area)

def mesh(poly,depth):
    n=len(poly); ids=list(range(n)); triangles=[]
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
    vertices=[(x,y,z) for z in (0.,depth) for x,y in poly]
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
    return vertices,faces,volume

def preview(poly,p,path):
    try:
        from PIL import Image,ImageDraw,ImageFont
    except ImportError: return
    im=Image.new('RGB',(1400,1050),'#edf1f5'); d=ImageDraw.Draw(im)
    font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',23)
    title=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',34)
    w=max(x for x,y in poly); h=max(y for x,y in poly)
    d.text((40,25),'Adjustable spring swirl — version 3',fill='#192332',font=title)
    d.text((40,78),f'{w:.1f} × {h:.1f} mm footprint | {p["body_thickness"]:g} mm front-to-back thickness',fill='#526174',font=font)
    d.text((40,115),f'Shaft: {p["shaft_length"]:g} mm long / {p["shaft_width"]:g} mm wide    Spring strip: {p["spring_width"]:g} mm',fill='#526174',font=font)
    s=min(690/h,490/w)
    d.polygon([(70+x*s,890-y*s) for x,y in poly],fill='#304357')
    def project(x,y,z): return (780+s*(x+z*.65),865-s*y+s*.19*x-s*.38*z)
    d.polygon([project(x,y,0) for x,y in poly],fill='#192332')
    for i in sorted(range(len(poly)),key=lambda i:poly[i][0]+poly[i][1]):
        j=(i+1)%len(poly)
        d.polygon([project(*poly[k],z) for k,z in [(i,0),(j,0),(j,p['body_thickness']),(i,p['body_thickness'])]],fill='#6b8299')
    d.polygon([project(x,y,p['body_thickness']) for x,y in poly],fill='#304357')
    d.text((70,945),'Flat on bed (XY)',fill='#192332',font=font)
    d.text((780,945),'Angled view',fill='#192332',font=font)
    im.save(path)

def build(p,name):
    if not all(math.isfinite(v) and v>0 for v in p.values()):
        raise ValueError('All dimensions must be positive finite millimetres.')
    poly,area=make_profile(p); vertices,faces,volume=mesh(poly,p['body_thickness'])
    assert abs(volume-area*p['body_thickness'])<1e-5
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
    scad+=f'linear_extrude(height={p["body_thickness"]}) polygon(points='+json.dumps(poly)+');\n'
    (OUT/f'{name}.scad').write_text(scad)
    report={'settings_mm':p,'actual_dimensions_mm':[round(max(x for x,y in poly),3),round(max(y for x,y in poly),3),p['body_thickness']],
            'triangles':len(faces),'watertight':True,'consistent_winding':True,'connected_components':1,
            'outline_self_intersections':0,'volume_mm3':round(volume,3)}
    (OUT/f'{name}_settings.json').write_text(json.dumps(report,indent=2))
    preview(poly,p,OUT/f'{name}_preview.png')
    print(json.dumps(report,indent=2))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    for key,value in DEFAULTS.items():
        if key == 'body_thickness':
            parser.add_argument('--body-thickness', '--depth', dest=key, type=float, default=value,
                                help=f'Front-to-back thickness of the entire item (Z); default {value} mm')
        else:
            parser.add_argument('--'+key.replace('_','-'),type=float,default=value,help=f'mm; default {value}')
    parser.add_argument('--name',default='swirl_item_v3',help='Output filename stem; use a new name to keep earlier trials')
    args=vars(parser.parse_args()); name=args.pop('name')
    if Path(name).name!=name or not name or any(c in name for c in '<>:"/\\|?*'):
        parser.error('--name must be a plain filename stem')
    try: build(args,name)
    except ValueError as e: parser.error(str(e))
