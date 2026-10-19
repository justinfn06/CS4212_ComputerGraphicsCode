"""Regenerated variable maps, drawn from the actual configured geometry."""
import math

def draw_diagrams(poly,marks,handles,p,out,name):
    try:
        from PIL import Image,ImageDraw,ImageFont
    except ImportError:
        print('Diagram skipped: install Pillow to generate PNG diagrams.'); return
    fonts={n:ImageFont.truetype('C:/Windows/Fonts/arial.ttf',n) for n in [20,23,25,28,32,42]}
    bg='#f7f8fb'; ink='#182636'; muted='#526477'; blue='#196cb2'; orange='#b65813'
    def canvas(title,subtitle):
        im=Image.new('RGB',(1800,1500),bg); d=ImageDraw.Draw(im)
        d.text((60,35),title,fill=ink,font=fonts[42]); d.text((60,95),subtitle,fill=muted,font=fonts[25])
        return im,d
    im,d=canvas('Swirl item | variable map','Edit the SETTINGS block in build_model.py. Dimensions are millimetres; angles are degrees.')
    w=max(x for x,y in poly); h=max(y for x,y in poly)
    scale=min(920/h,580/w)
    def pos(pt): return (590+pt[0]*scale,1170-pt[1]*scale)
    d.polygon([pos(pt) for pt in poly],fill='#cbd5e0')
    def dot(pt,color=blue):
        x,y=pos(pt); d.ellipse((x-5,y-5,x+5,y+5),fill=color)
    def callout(x,y,title,lines,target,right=False):
        d.text((x,y),title,fill=blue,font=fonts[28])
        for i,line in enumerate(lines): d.text((x,y+39+i*30),line,fill=ink,font=fonts[23])
        end=(x-15,y+18) if right else (x+420,y+18)
        tp=pos(target); elbow=(end[0]-28 if right else end[0]+28,tp[1])
        d.line([end,elbow,tp],fill=blue,width=2); dot(target)
    callout(60,220,'head_width / head_height',['Scale the head horizontally / vertically.','head_rotation rotates the whole head.'],marks.get('S2',marks['S0']))
    callout(60,470,'spring_width',['In-plane width of the curved strip.','CURVE_SEGMENTS sets its exact shape.'],marks.get('S4',marks['S0']))
    callout(60,760,'Shaft controls',['shaft_length / shaft_angle: S0 to B.','shaft_width: side-to-side width.','shaft_thickness: front-to-back Z.'],tuple((a+b)/2 for a,b in zip(marks['S0'],marks['B'])))
    callout(60,1050,'Foot block: fixed outside edge',['foot_length: grows toward the free tip.','foot_width: grows toward the head.','foot_thickness: constant front-to-back Z.'],marks['F'])
    target=marks.get('R3',marks['S0'])
    callout(1250,210,'return_width',['Width of the inner angular return.','RETURN_POINTS moves each corner.'],target,True)
    end=marks.get(f'R{len([k for k in marks if k.startswith("R")])}',marks['S0'])
    callout(1250,470,'end_bar_length_scale',['1 = traced length; 0.8 = 20% shorter.','end_bar_angle_offset turns its tip.'],end,True)
    d.text((1250,760),'body_thickness',fill=orange,font=fonts[28])
    d.text((1250,800),'Front-to-back thickness (Z).',fill=ink,font=fonts[23])
    # A separate side cross-section makes the third dimension unambiguous.
    x,y=1280,875
    d.polygon([(x,y),(x+200,y),(x+260,y-45),(x+60,y-45)],fill='#cbd5e0')
    d.polygon([(x,y),(x+200,y),(x+200,y+90),(x,y+90)],fill='#8ca5bc')
    d.polygon([(x+200,y),(x+260,y-45),(x+260,y+45),(x+200,y+90)],fill='#526477')
    d.line([(x-25,y),(x-25,y+90)],fill=orange,width=3)
    for yy in [y,y+90]: d.line([(x-35,yy),(x-15,yy)],fill=orange,width=3)
    d.text((x,y+110),f'Z = {p["body_thickness"]:g} mm by default',fill=orange,font=fonts[23])
    d.text((1250,1090),'Local overrides',fill=blue,font=fonts[28])
    for i,line in enumerate(['width_mm / thickness_mm at each point.','None = inherit the global setting.','A number = an absolute local dimension.']):
        d.text((1250,1130+i*31),line,fill=ink,font=fonts[23])
    for key in ['F','B','S0']:
        x,y=pos(marks[key]); d.ellipse((x-8,y-8,x+8,y+8),fill=orange)
        d.text((x+14,y+10),key,fill=orange,font=fonts[25])
    d.line([(60,1260),(1740,1260)],fill='#ccd3dc',width=2)
    for i,line in enumerate(['Angles: 0° = right, 90° = up, 180° = left, -90° = down in this front view.',
                             'LOCAL controls width/thickness at the foot tip, shaft base B and shaft top S0.',
                             'OPTIONS controls width/thickness blending, tip caps, output name, previews and diagrams.',
                             'Change values, save build_model.py, then run .\\run_model.ps1 to export the updated STL.']):
        d.text((60,1290+i*39),line,fill=ink,font=fonts[25])
    im.save(out/f'{name}_variable_map.png')

    im,d=canvas('Swirl item | curve and corner controls','S1–S7 map to CURVE_SEGMENTS[0]–[6]. R1–R5 map to RETURN_POINTS[0]–[4].')
    # Show only the head region with the same geometry and its actual handles.
    headpts=[pt for key,pt in marks.items() if key not in ('F','B')]
    xs=[pt[0] for pt in headpts]; ys=[pt[1] for pt in headpts]
    for hnd in handles:
        xs.extend(pt[0] for pt in hnd); ys.extend(pt[1] for pt in hnd)
    xmin,xmax=min(xs),max(xs); ymin,ymax=min(ys),max(ys)
    scale=min(780/(xmax-xmin),830/(ymax-ymin))
    def pos(pt): return (110+(pt[0]-xmin)*scale,1060-(pt[1]-ymin)*scale)
    # Crop the full-profile rendering to the head panel.
    layer=Image.new('RGB',im.size,bg); ld=ImageDraw.Draw(layer)
    ld.polygon([pos(pt) for pt in poly],fill='#e0e5ec')
    im.paste(layer.crop((50,160,1010,1110)),(50,160)); d=ImageDraw.Draw(im)
    for i,hnd in enumerate(handles,1):
        start,c1,c2,end=hnd
        for a,b in [(start,c1),(end,c2)]: d.line([pos(a),pos(b)],fill='#93a9bc',width=2)
        pts=[]
        for j in range(41):
            t=j/40; u=1-t
            pts.append(pos(tuple(u**3*start[k]+3*u*u*t*c1[k]+3*u*t*t*c2[k]+t**3*end[k] for k in (0,1))))
        d.line(pts,fill=blue,width=3)
        for key,pt in [('a',c1),('b',c2)]:
            x,y=pos(pt); d.rectangle((x-4,y-4,x+4,y+4),fill=orange)
            # Short handle IDs keep the dense curl readable.
            offset=(8,-25) if key=='a' else (8,5)
            d.text((x+offset[0],y+offset[1]),f'{i}{key}',fill=orange,font=fonts[20])
    offsets={'S0':(14,0),'S1':(12,-32),'S2':(-42,-35),'S3':(-43,0),'S4':(-12,16),
             'S5':(12,0),'S6':(5,-32),'S7':(12,0),'R1':(-45,8),'R2':(-45,-20),
             'R3':(12,-35),'R4':(12,5),'R5':(-30,15)}
    for key,pt in marks.items():
        if key in ('F','B'): continue
        x,y=pos(pt); d.ellipse((x-6,y-6,x+6,y+6),fill=blue)
        ox,oy=offsets.get(key,(10,10)); d.text((x+ox,y+oy),key,fill=ink,font=fonts[23])
    d.text((1080,190),'Move a curve',fill=blue,font=fonts[32])
    lines=['CURVE_SEGMENTS[i] contains:', 'end: where this segment finishes.',
           'c1: handle leaving its starting point.', 'c2: handle approaching its endpoint.',
           'steps: curve sampling resolution.', 'width_key: inherited width setting.',
           'width_mm: local width override.', 'thickness_mm: local Z override.',
           '', 'Squares 1a / 1b = segment 1 c1 / c2.',
           'Dots S1... = endpoints; S0 = HEAD_START.']
    for i,line in enumerate(lines): d.text((1080,245+i*37),line,fill=ink,font=fonts[25])
    d.text((1080,710),'Move a corner',fill=blue,font=fonts[32])
    for i,line in enumerate(['RETURN_POINTS[i]["xy"] moves Ri+1.',
                             'Each point can override width and Z.',
                             'Add c1 / c2 to make a segment curved.',
                             'Add/remove entries to change the shape.']):
        d.text((1080,765+i*37),line,fill=ink,font=fonts[25])
    d.text((1080,980),'Coordinates: (u, v)',fill=blue,font=fonts[32])
    for i,line in enumerate(['u increases to the right; v increases up.',
                             'x = u × head_width', 'y = v × head_height',
                             'Then head_rotation rotates the head.']):
        d.text((1080,1035+i*35),line,fill=ink,font=fonts[25])
    d.line([(60,1240),(1740,1240)],fill='#ccd3dc',width=2)
    for i,line in enumerate(['Example: CURVE_SEGMENTS[1]["end"] = (0.25, 0.95) moves S2.',
                             'Example: CURVE_SEGMENTS[3]["width_mm"] = 1.6 sets the width at S4.',
                             'Example: RETURN_POINTS[4]["thickness_mm"] = 12 sets the free bar tip to 12 mm thick.',
                             'Widths and thicknesses blend along segments. Intersecting outlines are rejected.']):
        d.text((60,1275+i*43),line,fill=ink,font=fonts[25])
    im.save(out/f'{name}_curve_controls.png')
