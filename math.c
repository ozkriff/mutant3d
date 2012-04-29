/*See LICENSE file for copyright and license details.*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "bool.h"
#include "math.h"
#include "list.h"
#include "core.h"
#include "mutant3d.h"
#include "misc.h"

float deg2rad(float deg)
{
  return (deg * (float)M_PI) / 180.0f;
}

float rad2deg(float rad)
{
  return (rad * 180.0f) / (float)M_PI;
}

Quat mk_quat(float x, float y, float z, float w)
{
  Quat q;
  q.x = x;
  q.y = y;
  q.z = z;
  q.w = w;
  return q;
}

Quat quat_mul(Quat a, Quat b)
{
  Quat q;
  q.x = a.y * b.z - a.z * b.y + a.w * b.x + a.x * b.w;
  q.y = a.z * b.x - a.x * b.z + a.w * b.y + a.y * b.w;
  q.z = a.x * b.y - a.y * b.x + a.w * b.z + a.z * b.w;
  q.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
  return q;
}

V3f quat_rot(Quat q, V3f v)
{
  Quat p = mk_quat(v.x, v.y, v.z, 0);
  Quat qConj = mk_quat(-q.x, -q.y, -q.z, q.w);
  p = quat_mul(q, quat_mul(p, qConj));
  return mk_v3f(p.x, p.y, p.z);
}

V3f mk_v3f(float x, float y, float z)
{
  V3f v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

V3f v3f_plus(V3f a, V3f b)
{
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  return a;
}

float v3f_dot(V3f a, V3f b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

V3f v3f_mul_float(V3f v, float f)
{
  v.x *= f;
  v.y *= f;
  v.z *= f;
  return v;
}

V3f v3f_divide_float(V3f v, float f)
{
  v.x /= f;
  v.y /= f;
  v.z /= f;
  return v;
}

V3f v3f_subt(V3f a, V3f b)
{
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  return a;
}

float v3f_length(V3f v)
{
  return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

V3f v3f_norm(V3f v)
{
  return v3f_mul_float(v, 1.0f / v3f_length(v));
}

V2f mk_v2f(float x, float y)
{
  V2f v;
  v.x = x;
  v.y = y;
  return v;
}

V2f v2f_plus(V2f a, V2f b)
{
  a.x += b.x;
  a.y += b.y;
  return a;
}

float v2f_dot(V2f a, V2f b)
{
  return a.x * b.x + a.y * b.y;
}

V2f v2f_mul_float(V2f v, float f)
{
  v.x *= f;
  v.y *= f;
  return v;
}

V2f v2f_subt(V2f a, V2f b)
{
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

float v2f_length(V2f v)
{
  return (float)sqrt(v.x * v.x + v.y * v.y);
}

V2f v2f_rotate(V2f v, float angle)
{
  V2f v2;
  float theta = deg2rad(angle);
  float cs = (float)cos(theta);
  float sn = (float)sin(theta);
  v2.x = v.x * cs - v.y * sn;
  v2.y = v.x * sn + v.y * cs;
  return v2;
}

void quat_renormalize(Quat *q)
{
  double len;
  assert(q);
  len = 1.0 - q->x * q->x - q->y * q->y - q->z * q->z;
  if (len < 1e-8) {
    q->w = 0;
  } else {
    q->w = -(float)sqrt(len);
  }
}

V2i mk_v2i(int x, int y)
{
  V2i pos;
  pos.x = x;
  pos.y = y;
  return pos;
}

V3i mk_v3i(int x, int y, int z)
{
  V3i pos;
  pos.x = x;
  pos.y = y;
  pos.z = z;
  return pos;
}

bool v3i_is_equal(V3i a, V3i b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

float get_rot_angle(V3f a, V3f b)
{
  float x_diff_2 = (float)pow(b.x - a.x, 2);
  float y_diff_2 = (float)pow(b.y - a.y, 2);
  float distance = (float)sqrt(x_diff_2 + y_diff_2);
  float angle = rad2deg((float)asin((b.x - a.x) / distance));
  if (b.y - a.y > 0) {
    angle = -(180 + angle);
  }
  return angle;
}

/*http://algolist.manual.ru/maths/geom/intersect/linefacet3d.php
  Determine whether or not the line segment [p1 p2]
  intersects the 3 vertex facet bounded by [pa pb pc].
  Return true/false and the intersection point 'p'.
  The equation of the line is "p = p1 + mu (p2 - p1)"
  The equation of the plane is
    "a x + b y + c z + d = 0
    n.x x + n.y y + n.z z + d = 0"
*/
bool line_tri_intersec(V3f p1, V3f p2, V3f pa, V3f pb, V3f pc, V3f *p)
{
  float eps = 0.1f; /*TODO*/
  float d;
  float a1, a2, a3;
  float total, denom, mu;
  V3f n, pa1, pa2, pa3;
  assert(p);
  /*Calculate the parameters for the plane*/
  n.x = (pb.y - pa.y) * (pc.z - pa.z) - (pb.z - pa.z) * (pc.y - pa.y);
  n.y = (pb.z - pa.z) * (pc.x - pa.x) - (pb.x - pa.x) * (pc.z - pa.z);
  n.z = (pb.x - pa.x) * (pc.y - pa.y) - (pb.y - pa.y) * (pc.x - pa.x);
  n = v3f_norm(n);
  d = - n.x * pa.x - n.y * pa.y - n.z * pa.z;
  /*Calculate the position on the line that intersects the plane*/
  denom = n.x * (p2.x - p1.x) + n.y * (p2.y - p1.y) + n.z * (p2.z - p1.z);
  /*Line and plane don't intersect.*/
  if ((float)fabs(denom) < eps) {
    return false;
  }
  mu = - (d + n.x * p1.x + n.y * p1.y + n.z * p1.z) / denom;
  *p = v3f_plus(p1, v3f_mul_float(v3f_subt(p2, p1), mu));
  /*Intersection not along line segment.*/
  if (mu < 0 || mu >= 1.0f) {
    return false;
  }
  /*Determine whether or not the intersection point is bounded by pa,pb,pc.*/
  pa1 = v3f_norm(v3f_subt(pa, *p));
  pa2 = v3f_norm(v3f_subt(pb, *p));
  pa3 = v3f_norm(v3f_subt(pc, *p));
  a1 = pa1.x * pa2.x + pa1.y * pa2.y + pa1.z * pa2.z;
  a2 = pa2.x * pa3.x + pa2.y * pa3.y + pa2.z * pa3.z;
  a3 = pa3.x * pa1.x + pa3.y * pa1.y + pa3.z * pa1.z;
  total = rad2deg((float)(acos(a1) + acos(a2) + acos(a3)));
  return fabs(total - 360) <= eps;
}

bool line_quad_intersec(V3f p1, V3f p2, V3f pa, V3f pb, V3f pc, V3f pd, V3f *p)
{
  if (line_tri_intersec(p1, p2, pa, pb, pc, p)) {
    return true;
  }
  if (line_tri_intersec(p1, p2, pa, pc, pd, p)) {
    return true;
  }
  return false;
}
