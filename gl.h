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
