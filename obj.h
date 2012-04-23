/*See LICENSE file for copyright and license details.*/

/*vertex, texture, normal*/
typedef struct {
  int v[3];
  int t[3];
  int n[3];
} Obj_triangle;

typedef struct {
  V3f *vertexes;
  V3f *normals;
  V2f *text_coords;
  Obj_triangle *faces;
  int f_count;
  int v_count;
  int t_count;
  int n_count;
} Obj_model;

void obj_read(Obj_model *m, char *filename);
void obj_draw(GLuint tex_id, Obj_model *m);
void obj_debug_print(Obj_model *m);
