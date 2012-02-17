typedef struct { float x, y, z, w; } Quat;
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y; } Vec2;

Quat quat_mul (Quat a, Quat b);
Vec3 quat_rot (Quat q, Vec3 v);
Vec3 vec3d_plus (Vec3 a, Vec3 b);
void renormalize (Quat *q);
