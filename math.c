#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bool.h"
#include "math.h"
#include "misc.h"

float deg2rad (float deg){
  return(deg * 180.0f / M_PI);
}

float rad2deg (float rad){
  return(rad * 180.0f / M_PI);
}

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

Vec3 mk_vec3 (float x, float y, float z){
  Vec3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return(v);
}

Vec3 vec3_plus (Vec3 a, Vec3 b){
   a.x += b.x;
   a.y += b.y;
   a.z += b.z;
   return(a);
}

float vec3_dot (Vec3 a, Vec3 b){
  return(a.x * b.x + a.y * b.y + a.z * b.z);
}

Vec3 vec3_mul_float (Vec3 v, float f){
  v.x *= f;
  v.y *= f;
  v.z *= f;
  return(v);
}

Vec3 vec3_subt (Vec3 a, Vec3 b){
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
  return(a);
}

float vec3_length (Vec3 v){
  return((float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
}

Vec2 mk_vec2 (float x, float y){
  Vec2 v;
  v.x = x;
  v.y = y;
  return(v);
}

Vec2 vec2_plus (Vec2 a, Vec2 b){
   a.x += b.x;
   a.y += b.y;
   return(a);
}

float vec2_dot (Vec2 a, Vec2 b){
  return(a.x * b.x + a.y * b.y);
}

Vec2 vec2_mul_float (Vec2 v, float f){
  v.x *= f;
  v.y *= f;
  return(v);
}

Vec2 vec2_subt (Vec2 a, Vec2 b){
  a.x -= b.x;
  a.y -= b.y;
  return(a);
}

float vec2_length (Vec2 v){
  return((float)sqrt(v.x * v.x + v.y * v.y));
}

Vec2 vec2_rotate (Vec2 v, float angle){
  Vec2 v2;
  float theta = deg2rad(angle);
  float cs = (float)cos(theta);
  float sn = (float)sin(theta);
  v2.x = v.x * cs - v.y * sn; 
  v2.y = v.x * sn + v.y * cs;
  return(v2);
}

void quat_renormalize (Quat *q){
  double len = 1.0 - q->x*q->x - q->y*q->y - q->z*q->z;
  if(len < 1e-8)
    q->w = 0;
  else
    q->w = -sqrt(len);
}
