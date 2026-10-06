#include "Triangle.h"

bool Triangle::intersect(const ray &r, float t_min, float &t_max, HitStruct &hit) const
{
  double a = vertex_a[0] - vertex_b[0];
  double b = vertex_a[1] - vertex_b[1];
  double c = vertex_a[2] - vertex_b[2];

  double d = vertex_a[0] - vertex_c[0];
  double e = vertex_a[1] - vertex_c[1];
  double f = vertex_a[2] - vertex_c[2];

  double g = r.direction()[0];
  double h = r.direction()[1];
  double i = r.direction()[2];

  double j = vertex_a[0] - r.origin()[0];
  double k = vertex_a[1] - r.origin()[1];
  double l = vertex_a[2] - r.origin()[2];

  double ei_hf = e * i - h * f;
  double gf_id = g * f - d * i;
  double dh_eg = d * h - e * g;

  double M = a * ei_hf + b * gf_id + c * dh_eg;

  double t = -1.0, gamma = -1.0, beta = -1.0;

  double ak_jb = a * k - j * b;
  double jc_al = j * c - a * l;
  double bl_kc = b * l - k * c;

  t = -(f * ak_jb + e * jc_al + d * bl_kc) / M;
  if (t < t_min || t > t_max) {
    return false;
  }

  gamma = (i * ak_jb + h * jc_al + g * bl_kc) / M;
  if (gamma < 0.0 || gamma > 1.0) {
    return false;
  }

  beta = (j * ei_hf + k * gf_id + l * dh_eg) / M;
  if (beta < 0.0 || beta > 1.0 - gamma) {
    return false;
  }

  t_max = t;
  hit.t = t;
  hit.point = r.at(t);
  hit.shape = this;

  return true;
}

vec3 Triangle::getColor() const
{
  return color;
}