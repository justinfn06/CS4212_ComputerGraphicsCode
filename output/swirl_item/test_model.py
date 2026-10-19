import copy
import contextlib
import io
import unittest
from pathlib import Path
from unittest.mock import patch
import build_model as m

class GeometryTests(unittest.TestCase):
    def setUp(self):
        self.saved={k:copy.deepcopy(getattr(m,k)) for k in ['CURVE_SEGMENTS','RETURN_POINTS','LOCAL','OPTIONS']}
        self.p=m.DEFAULTS.copy()
        # Reference dimensions for regression checks, independent of user tuning.
        self.p.update(shaft_length=75.0,end_bar_length_scale=1.0,body_thickness=10.0,foot_length=18.0,foot_width=7.0,shaft_width=4.0,shaft_thickness=None,foot_thickness=None)
    def tearDown(self):
        for k,v in self.saved.items(): setattr(m,k,v)
    def checked(self):
        poly,area,z,*_=m.geometry(self.p)
        vertices,faces,volume=m.mesh(poly,z)
        self.assertGreater(volume,0)
        return poly,z,volume
    def test_default_dimensions(self):
        poly,z,volume=self.checked()
        self.assertAlmostEqual(max(x for x,y in poly),69.793,places=3)
        self.assertGreater(max(y for x,y in poly),120)
        self.assertLess(max(y for x,y in poly),125)
        self.assertEqual(set(z),{10.0})
        self.assertGreater(volume,0)
    def test_foot_is_constant_section_block(self):
        for angle in [0,150,170,180,190,210]:
            for thickness in [6,14]:
                with self.subTest(angle=angle,thickness=thickness):
                    self.p.update(foot_angle=angle,foot_thickness=thickness)
                    poly,area,z,*_=m.geometry(self.p)
                    m.mesh(poly,z)
                    foot=[poly[i] for i in poly.regions[1]]
                    self.assertEqual({z[i] for i in poly.regions[1]},{thickness})
                    foot_area=sum(foot[i][0]*foot[(i+1)%len(foot)][1]-foot[(i+1)%len(foot)][0]*foot[i][1] for i in range(len(foot)))/2
                    self.assertAlmostEqual(foot_area,self.p['foot_length']*self.p['foot_width'],places=7)
    def test_foot_grows_only_inward_and_back_is_flush(self):
        import math
        anchors=[]
        for length,width in [(18,7),(28,7),(28,12),(40,15)]:
            self.p.update(foot_length=length,foot_width=width,foot_angle=180)
            poly,area,z,marks,_=m.geometry(self.p)
            m.mesh(poly,z)
            foot_ids=poly.regions[1]; shaft_ids=poly.regions[0]
            shared=set(foot_ids)&set(shaft_ids)
            self.assertEqual(len(shared),2)
            outer_exit=max(shared,key=lambda i:poly[i][0])
            # Bottom rear anchor is fixed relative to the shaft base B.
            only=[i for i in foot_ids if i not in shared]
            anchor=max(only,key=lambda i:poly[i][0])
            anchors.append(tuple(poly[anchor][k]-marks['B'][k] for k in (0,1)))
            a,b=poly[anchor],poly[outer_exit]
            # Every foot point lies inside the shaft's extended outside edge.
            for i in foot_ids:
                self.assertGreaterEqual(m.cross(a,b,poly[i]),-1e-7)
            # The next shaft edge is collinear: no step or rear protrusion.
            neighbors=[(outer_exit-1)%len(poly),(outer_exit+1)%len(poly)]
            nxt=next(i for i in neighbors if i in shaft_ids and i!=anchor)
            self.assertAlmostEqual(m.cross(a,b,poly[nxt]),0,places=7)
        for a in anchors[1:]:
            for v,ref in zip(a,anchors[0]): self.assertAlmostEqual(v,ref,places=7)
    def test_global_thickness_only_changes_z(self):
        poly,z,vol=self.checked(); self.p['body_thickness']=12
        poly2,z2,vol2=self.checked()
        self.assertEqual(poly,poly2); self.assertEqual(set(z2),{12.0})
        self.assertAlmostEqual(vol2/vol,1.2)
    def test_local_thickness(self):
        poly,_,vol=self.checked()
        m.CURVE_SEGMENTS[3]['thickness_mm']=6
        m.RETURN_POINTS[-1]['thickness_mm']=12
        m.LOCAL['foot_thickness_mm']=8
        poly2,z,vol2=self.checked()
        self.assertEqual(poly,poly2); self.assertEqual(min(z),6); self.assertEqual(max(z),12)
        self.assertNotAlmostEqual(vol,vol2)
    def test_shaft_thickness(self):
        poly,_,_=self.checked()
        self.p['shaft_thickness']=14
        poly2,z,_=self.checked()
        self.assertEqual(poly,poly2)
        samples,_,_=m.centreline(self.p)
        self.assertEqual(samples[1][3],14)
        self.assertEqual(samples[2][3],14)
        self.assertEqual(samples[0][3],10)
        self.assertEqual(samples[34][3],10)
        self.assertEqual(samples[-1][3],10)
        m.LOCAL['shaft_top_thickness_mm']=12
        samples,_,_=m.centreline(self.p)
        self.assertEqual(samples[1][3],14)
        self.assertEqual(samples[2][3],12)
    def test_shape_and_width_controls(self):
        poly,_,_=self.checked()
        m.CURVE_SEGMENTS[1]['end']=(0.25,0.95)
        m.CURVE_SEGMENTS[3]['width_mm']=1.6
        m.LOCAL['shaft_base_width_mm']=3
        self.p['foot_angle']=175; self.p['end_bar_length_scale']=0.9
        poly2,_,_=self.checked(); self.assertNotEqual(poly,poly2)
    def test_curved_return_and_square_caps(self):
        m.RETURN_POINTS[-1].update(c1=(0.60,0.34),c2=(0.35,0.13),steps=40)
        m.OPTIONS['end_caps']='square'
        m.OPTIONS['width_blend']='linear'
        self.checked()
    def test_local_thickness_export(self):
        m.CURVE_SEGMENTS[3]['thickness_mm']=7
        m.OPTIONS.update(write_preview=False,write_diagrams=False)
        captured={}
        def capture(path,data,*args,**kwargs):
            captured[path.name]=data
            return len(data)
        with patch.object(Path,'write_bytes',capture), patch.object(Path,'write_text',capture):
            with contextlib.redirect_stdout(io.StringIO()): m.build(self.p,'test')
        self.assertGreater(len(captured['test.stl']),84)
        self.assertIn('polyhedron',captured['test.scad'])
    def test_invalid_controls_rejected(self):
        m.CURVE_SEGMENTS[0]['steps']=1
        with self.assertRaises(ValueError): self.checked()
        m.CURVE_SEGMENTS[0]['steps']=32
        self.p['spring_width']=100
        with self.assertRaises(ValueError): self.checked()

if __name__=='__main__': unittest.main(verbosity=2)
