#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bool.h"
#include "math.h"
#include "misc.h"

Quat quat_mul (Quat a, Quat b){
  Quat q;
  q.x = a.y*b.z - a.z*b.y + a.w*b.x + a.x*b.w;
  q.y = a.z*b.x - a.x*b.z + a.w*b.y + a.y*b.w;
  q.z = a.x*b.y - a.y*b.x + a.w*b.z + a.z*b.w;
  q.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  return(q);
}

Vec3 quat_rot (Quat q, Vec3 v){
  Quat p;
  Quat qConj;
  Vec3 v2;
  p.x = v.x;
  p.y = v.y;
  p.z = v.z;
  p.w = 0;
  qConj.x = -q.x;
  qConj.y = -q.y;
  qConj.z = -q.z;
  qConj.w = q.w;
  p = quat_mul(q, quat_mul(p, qConj));
  v2.x = p.x;
  v2.y = p.y;
  v2.z = p.z;
  return(v2);
}

Vec3 vec3d_plus (Vec3 a, Vec3 b){
   a.x += b.x;
   a.y += b.y;
   a.z += b.z;
   return(a);
}

void renormalize (Quat *q){
  double len = 1.0 - q->x*q->x - q->y*q->y - q->z*q->z;
  if(len < 1e-8)
    q->w = 0;
  else
    q->w = -sqrt(len);
}
