/*See LICENSE file for copyright and license details.*/

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

typedef unsigned int uint;

typedef struct {
  float x, y, z, w;
} Quat;
typedef struct {
  float x, y, z;
} V3f;
typedef struct {
  int x, y, z;
} V3i;
typedef struct {
  float x, y;
} V2f;
typedef struct {
  int x, y;
} V2i;

float deg2rad(float deg);
float rad2deg(float rad);

Quat mk_quat(float x, float y, float z, float w);
Quat quat_mul(Quat a, Quat b);
V3f quat_rot(Quat q, V3f v);
void quat_renormalize(Quat *q);

V2f mk_v2f(float x, float y);
V2f v2f_plus(V2f a, V2f b);
float v2f_dot(V2f a, V2f b);
V2f v2f_mul_float(V2f v, float f);
V2f v2f_subt(V2f a, V2f b);
float v2f_length(V2f v);
V2f v2f_rotate(V2f v, float angle);

V3f mk_v3f(float x, float y, float z);
V3f v3f_plus(V3f a, V3f b);
V3f v3f_subt(V3f a, V3f b);
float v3f_dot(V3f a, V3f b);
V3f v3f_mul_float(V3f v, float f);
V3f v3f_divide_float(V3f v, float f);
float v3f_length(V3f v);
V3f v3f_norm(V3f v);

V2i mk_v2i(int x, int y);

V3i mk_v3i(int x, int y, int z);
bool v3i_is_equal(V3i a, V3i b);

float get_rot_angle(V3f a, V3f b);

bool line_tri_intersec(V3f p1, V3f p2, V3f pa, V3f pb, V3f pc, V3f *p);
bool line_quad_intersec(V3f p1, V3f p2, V3f pa, V3f pb, V3f pc, V3f pd, V3f *p);
