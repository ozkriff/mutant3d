/*See LICENSE file for copyright and license details.*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "bool.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"

float deg2rad(float deg){
  return (deg * (float)M_PI) / 180.0f;
}

float rad2deg(float rad){
  return (rad * 180.0f) / (float)M_PI;
}

Quat mk_quat(float x, float y, float z, float w){
  Quat q;
  q.x = x;
  q.y = y;
  q.z = z;
  q.w = w;
  return q;
}

Quat quat_mul(Quat a, Quat b){
  Quat q;
  q.x = a.y*b.z - a.z*b.y + a.w*b.x + a.x*b.w;
  q.y = a.z*b.x - a.x*b.z + a.w*b.y + a.y*b.w;
  q.z = a.x*b.y - a.y*b.x + a.w*b.z + a.z*b.w;
  q.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  return q;
}

V3f quat_rot(Quat q, V3f v){
  Quat p;
  Quat qConj;
  p = mk_quat(v.x, v.y, v.z, 0);
  qConj = mk_quat(-q.x, -q.y, -q.z, q.w);
  p = quat_mul(q, quat_mul(p, qConj));
  return mk_v3f(p.x, p.y, p.z);
}

V3f mk_v3f(float x, float y, float z){
  V3f v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

V3f v3f_plus(V3f a, V3f b){
   a.x += b.x;
   a.y += b.y;
   a.z += b.z;
   return a;
}

float v3f_dot(V3f a, V3f b){
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

V3f v3f_mul_float(V3f v, float f){
  v.x *= f;
  v.y *= f;
  v.z *= f;
  return v;
}

V3f v3f_subt(V3f a, V3f b){
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  return a;
}

float v3f_length(V3f v){
  return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

V2f mk_v2f(float x, float y){
  V2f v;
  v.x = x;
  v.y = y;
  return v;
}

V2f v2f_plus(V2f a, V2f b){
   a.x += b.x;
   a.y += b.y;
   return a;
}

float v2f_dot(V2f a, V2f b){
  return a.x * b.x + a.y * b.y;
}

V2f v2f_mul_float(V2f v, float f){
  v.x *= f;
  v.y *= f;
  return v;
}

V2f v2f_subt(V2f a, V2f b){
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

float v2f_length(V2f v){
  return (float)sqrt(v.x * v.x + v.y * v.y);
}

V2f v2f_rotate(V2f v, float angle){
  V2f v2;
  float theta = deg2rad(angle);
  float cs = (float)cos(theta);
  float sn = (float)sin(theta);
  v2.x = v.x * cs - v.y * sn;
  v2.y = v.x * sn + v.y * cs;
  return v2;
}

void quat_renormalize(Quat *q){
  double len;
  assert(q);
  len = 1.0 - q->x*q->x - q->y*q->y - q->z*q->z;
  if(len < 1e-8)
    q->w = 0;
  else
    q->w = -(float)sqrt(len);
}

V3i mk_v3i(int x, int y, int z){
  V3i pos;
  pos.x = x;
  pos.y = y;
  pos.z = z;
  return pos;
}
