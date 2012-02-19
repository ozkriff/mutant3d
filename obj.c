#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <GL/glfw.h>
#include "bool.h"
#include "math.h"
#include "obj.h"
#include "misc.h"

/*TODO "usemtl Material_CUBE_WARRIOR.bmp"*/
void obj_read (Obj_model *m, char *filename){
  char buffer[100];
  FILE *file;
  int v_i = 0;
  int n_i = 0;
  int t_i = 0;
  int f_i = 0;
  file = fopen(filename, "r");
  if(!file)
    die("can't find file: %s", filename);
  while(fgets(buffer, 100, file)){
    if(buffer[0] == 'v' && buffer[1] == ' ')
      m->v_count++;
    else if(buffer[0] == 'v' && buffer[1] == 'n')
      m->n_count++;
    else if(buffer[0] == 'v' && buffer[1] == 't')
      m->t_count++;
    else if(buffer[0] == 'f' && buffer[1] == ' ') 
      m->f_count++;
  }
  rewind(file);
  m->vertexes = malloc(m->v_count * sizeof(Vec3));
  m->normals = malloc(m->n_count * sizeof(Vec3));
  m->text_coords = malloc(m->t_count * sizeof(Vec2));
  m->faces = malloc(m->f_count * sizeof(Obj_triangle));
  while(fgets(buffer, 100, file)){
    if(buffer[0] == 'v' && buffer[1] == ' '){
      /*Vertex coords*/
      int items;
      Vec3 *v;
      v = m->vertexes + v_i;
      items = sscanf(buffer, "v %f %f %f",
             &v->x, &v->y, &v->z);
      if(items != 3)
        die("v: items != 3\n");
#if 1
#define resize_coefficient (7.) /*TODO: remove later*/
      m->vertexes[v_i].x *= resize_coefficient;
      m->vertexes[v_i].y *= resize_coefficient;
      m->vertexes[v_i].z *= resize_coefficient;
#endif
      v_i++;
    }else if(buffer[0] == 'v' && buffer[1] == 'n'){
      /*Vertex normals*/
      int items;
      Vec3 *norm = m->normals + n_i;
      items = sscanf(buffer, "vn %f %f %f",
          &norm->x, &norm->y, &norm->z);
      if(items != 3)
        die("vn: items != 3\n");
      n_i++;
    }else if (buffer[0] == 'v' && buffer[1] == 't'){
      /*Texture coords*/
      int items;
      Vec2 *tex = m->text_coords + t_i;
      items = sscanf(buffer, "vt %f %f", &tex->x, &tex->y);
      if(items != 2)
        die("vt: items != 2\n");
      t_i++;
    }else if (buffer[0] == 'f' && buffer [1] == ' '){
      /*Faces*/
      int items;
      Obj_triangle *t;
      int slash_count = 0;
      int i;
      for (i = 2; buffer[i] != ' '; i++)
        if (buffer[i] == '/')
          slash_count++;
      t = m->faces + f_i;
      if (slash_count == 1){
        items = sscanf(buffer, "f %d/%d %d/%d %d/%d",
          &t->v[0],
          &t->t[0],
          &t->v[1],
          &t->t[1],
          &t->v[2],
          &t->t[2]);
        if(items != 6)
          die("f: items != 6\n");
      }else if (slash_count == 2){
        items = sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d",
          &t->v[0],
          &t->t[0],
          &t->n[0],
          &t->v[1],
          &t->t[1],
          &t->n[1],
          &t->v[2],
          &t->t[2],
          &t->n[2]);
        if(items != 9)
          die("f: items != 9\n");
      }else{
        die("f: error\n");
      }
      f_i++;
    }
  }
  fclose(file);
}

void obj_debug_print (Obj_model *m){
  int i;
  for(i = 0; i <= m->v_count; i++){
    Vec3 *v = m->vertexes + i;
    printf("v: %f %f %f\n", v->x, v->y, v->z);
  }
}

void obj_draw (GLuint tex_id, Obj_model *m){
  int i, j;
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glBegin(GL_TRIANGLES);
  for(i = 0; i <= m->f_count; i++){
    Obj_triangle *t = m->faces + i;
    for(j = 0; j < 3; j++){
      int tex_id = t->t[j] - 1;
      int vert_id = t->v[j] - 1;
      Vec2 *tex = m->text_coords + tex_id;
      Vec3 *vert = m->vertexes + vert_id;
      glTexCoord2f(tex->x, tex->y);
      glVertex3f(vert->x, vert->y, vert->z);
    }
  }
  glEnd();
}
