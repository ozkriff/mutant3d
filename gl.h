/*See LICENSE file for copyright and license details.*/

typedef struct {
  float *v; /*vertices*/
  float *t; /*[opt] texture coordinates*/
  GLubyte *ub_c; /*[opt]colors*/
  int count; /*vertices count*/
} Va;

bool load_texture(char *filename, GLuint *id);
void win2world(int x, int y, V3f *p);
void va_rotate(Va *va, Quat q);

void set_xy(float *coords, int n, int i, int vi, float x, float y);
void set_xyz(float *verts, int n, int i, int vi, float x, float y, float z);
void set_rgb(GLubyte *colors, int n, int i, int vi, GLubyte r, GLubyte g, GLubyte b);
