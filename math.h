#define M_PI 3.14159265358979323846264338327

typedef struct { float x, y, z, w; } Quat;
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y; } Vec2;

float deg2rad (float deg);
float rad2deg (float rad);

Quat quat_mul (Quat a, Quat b);
Vec3 quat_rot (Quat q, Vec3 v);
void quat_renormalize (Quat *q);

Vec2 mk_vec2 (float x, float y);
Vec2 vec2_plus (Vec2 a, Vec2 b);
float vec2_dot (Vec2 a, Vec2 b);
Vec2 vec2_mul_float (Vec2 v, float f);
Vec2 vec2_subt (Vec2 a, Vec2 b);
float vec2_length (Vec2 v);
Vec2 vec2_rotate (Vec2 v, float angle);

Vec3 mk_vec3 (float x, float y, float z);
Vec3 vec3_plus (Vec3 a, Vec3 b);
Vec3 vec3_subt (Vec3 a, Vec3 b);
float vec3_dot (Vec3 a, Vec3 b);
Vec3 vec3_mul_float (Vec3 v, float f);
float vec3_length (Vec3 v);
