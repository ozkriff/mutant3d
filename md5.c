/*See LICENSE file for copyright and license details.*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <GL/glfw.h>
#include "bool.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"
#include "md5.h"
#include "gl.h"

V3f md5_joint_transform(Md5_joint *j, V3f v){
  return v3f_plus(quat_rot(j->orient, v), j->pos);
}

void md5_anim_debug_print(Md5_anim *a){
  int i;
  for(i = 0; i < a->num_joints; i++){
    Md5_hierarchy_item *h = a->hierarchy + i;
    printf("hierarchy %d %d %d\n",
      h->parent, h->flags, h->start_index);
  }
  for(i = 0; i < a->num_joints; i++){
    Md5_base_frame_joint *j = a->base_frame + i;
    printf("base_frame_joint (%f %f %f) (%f %f %f %f)\n",
      j->pos.x, j->pos.y, j->pos.z,
      j->orient.x, j->orient.y, j->orient.z, j->orient.w);
  }
  for(i = 0; i < a->num_frames; i++){
    int j;
    printf("frame:\n");
    for(j = 0; j < a->num_animated_components; j++){
      printf("%f ", a->frames[i][j]);
      if(j % 6 == 0)
        printf("\n");
    }
    printf("\n");
  }
  for(i = 0; i < a->num_joints; i++){
    Md5_joint *j = a->joints + i;
    printf("joint %i (%f %f %f) (%f %f %f %f)\n",
      j->parent_index,
      j->pos.x, j->pos.y, j->pos.z,
      j->orient.x, j->orient.y, j->orient.z, j->orient.w);
  }
}

void md5_mesh_debug_print(Md5_mesh *m){
  int i;
  for(i=0; i<m->num_vertices; i++){
    V3f *v = m->points + i;
    printf("  point: %d %f %f %f\n",
        i, v->x, v->y, v->z);
  }
  for(i=0; i<m->num_vertices; i++){
    Md5_vertex *v = m->vertices + i;
    printf("  vert: %d (%f %f) %d %d\n",
        i,
        v->tex.x,
        v->tex.y,
        v->weight_index,
        v->weight_count);
  }
  for(i=0; i<m->num_tris; i++){
    Md5_triangle *t = m->tris + i;
    printf("  tri %d  %d %d %d\n",
        i,
        t->index[0],
        t->index[1],
        t->index[2]);
  }
  for(i=0; i<m->num_weights; i++){
    Md5_weight *w = m->weights + i;
    printf("  weight %d %d %f (%f %f %f)\n",
        i,
        w->joint_index,
        w->weight,
        w->pos.x,
        w->pos.y,
        w->pos.z);
  }
}

void md5_model_debug_print(Md5_model *m){
  int i;
  for(i=0; i<m->num_joints; i++){
    Md5_joint *j = m->joints + i;
    printf("joint: %d %d  (%f %f %f) (%f %f %f %f)\n",
        i,
        j->parent_index,
        j->pos.x,
        j->pos.y,
        j->pos.z,
        j->orient.x,
        j->orient.y,
        j->orient.z,
        j->orient.w);
  }
  for(i=0; i<m->num_meshes; i++){
    Md5_mesh *mesh = m->meshes + i;
    printf("Md5_mesh %d:\n", i);
    md5_mesh_debug_print(mesh);
  }
}

/*Compute real points from bones data.*/
void md5_mesh_calc_points(Md5_mesh *m, Md5_joint *joints){
  int i;
  for(i = 0; i < m->num_vertices; i++){
    Md5_vertex *v = m->vertices + i; /*current vertex*/
    V3f p = {0, 0, 0};
    int k;
    for(k = 0; k < v->weight_count; k++){
      Md5_weight *w = m->weights + (v->weight_index + k);
      Md5_joint  *j  = joints + w->joint_index;
      /*Transform weight.pos by bone with weight.*/
      V3f p2 = md5_joint_transform(j, w->pos);
      p.x += p2.x * w->weight;
      p.y += p2.y * w->weight;
      p.z += p2.z * w->weight;
    }
    m->points[i] = p;
  }
}

void md5_model_compute(Md5_model *m, Md5_joint *joints){
  int i;
  for(i = 0; i < m->num_meshes; i++ )
    md5_mesh_calc_points(m->meshes + i, joints);
}

void md5_read_mesh(FILE *f, Md5_mesh *m){
  char s[200];
  while(fgets(s, 200, f) != NULL){
    if(s[0] == '}'){
      int i;
      m->max_joints_per_vert = 0;
      /*Calculate max joints per vertex*/
      for(i = 0; i < m->num_vertices; i++)
        if(m->vertices[i].weight_count > m->max_joints_per_vert)
          m->max_joints_per_vert = m->vertices[i].weight_count;
      return;
    }else if(strcmp_sp(s+1, "shader %s\n")){
      char name[40];
      sscanf(s+1, "shader \"%[^\"]s\"\n", name);
      m->shader = my_strdup(name);
      {
        /*TODO: check if this texture is already loaded*/
        char tex_name[40];
        sprintf(tex_name, "%s.tga", name);
        if(!load_texture(tex_name, &m->texture)){
          printf("No texture '%s'\n", tex_name);
          exit(1);
        }
      }
    }else if(strcmp_sp(s+1, "numverts %d\n")){
      sscanf(s+1, "numverts %d\n", &m->num_vertices);
      m->vertices = ALLOCATE(m->num_vertices, Md5_vertex);
      m->points = ALLOCATE(m->num_vertices, Md5_vertex);
    }else if(strcmp_sp(s+1, "numtris %d\n")){
      sscanf(s+1, "numtris %d\n", &m->num_tris);
      m->tris = ALLOCATE(m->num_tris, Md5_triangle);
    }else if(strcmp_sp(s+1, "numweights %d\n")){
      sscanf(s+1, "numweights %d\n", &m->num_weights);
      m->weights = ALLOCATE(m->num_weights, Md5_weight);
    }else if(strcmp_sp(s+1, "vert ")){
      Md5_vertex *v;
      int index;
      V2f tex;
      int weightIndex;
      int weightCount;
      sscanf(s+1, "vert %d ( %f %f ) %d %d\n",
          &index,
          &tex.x,
          &tex.y,
          &weightIndex,
          &weightCount);
      v = m->vertices + index;
      v->tex.x = tex.x;
      v->tex.y = tex.y;
      v->weight_index = weightIndex;
      v->weight_count = weightCount;
    }else if(strcmp_sp(s+1, "tri ")){
      Md5_triangle *t;
      int index, i0, i1, i2;
      sscanf(s+1, "tri %d %d %d %d\n",
          &index, &i0, &i1, &i2);
      t = m->tris + index;
      t->index[0] = i0;
      t->index[1] = i1;
      t->index[2] = i2;
    }else if(strcmp_sp(s+1, "weight ")){
      Md5_weight *w;
      int index;
      int joint;
      float bias;
      V3f pos;
      sscanf(s+1, "weight %d %d %f ( %f %f %f )\n",
          &index, &joint, &bias, &pos.x, &pos.y, &pos.z);
      w = m->weights + index;
      w->joint_index = joint;
      w->weight = bias;
      w->pos.x = pos.x;
      w->pos.y = pos.y;
      w->pos.z = pos.z;
    }
  }
}

void md5_read_joints(FILE *f, Md5_joint *joints){
  char s[200];
  int no = 0;
  while(fgets(s, 200, f) != NULL){
    if(s[0] == '}'){
      /*puts("}..");*/
      return;
    }else{
      /*puts("\tjoint...");*/
      Md5_joint *j;
      V3f pos;
      int index;
      Quat q;
      char name[40];
      sscanf(s+1, "%s %d ( %f %f %f ) ( %f %f %f )\n",
          name,
          &index,
          &pos.x, &pos.y, &pos.z,
          &(q.x), &(q.y), &(q.z));
      quat_renormalize(&q);
      j = joints + no;
      j->name        = my_strdup(name);
      j->parent_index = index;
      j->parent      = (index >= 0) ? (joints+index) : NULL;
      j->pos.x       = pos.x;
      j->pos.y       = pos.y;
      j->pos.z       = pos.z;
      j->orient.x    = q.x;
      j->orient.y    = q.y;
      j->orient.z    = q.z;
      j->orient.w    = q.w;
      no++;
    }
  }
}

void md5_load_model(Md5_model *m, char *filename){
  FILE *f = fopen(filename, "r");
  char s[200];
  int mesh_n = 0;
  while(fgets(s, 200, f) != NULL){
    if(strcmp_sp(s, "numJoints %d\n")){
      /*puts("numjoints");*/
      sscanf(s, "numJoints %d\n", &m->num_joints);
      m->joints = ALLOCATE(m->num_joints, Md5_joint);
    }else if(strcmp_sp(s, "numMeshes %d\n")){
      /*puts("nummesh");*/
      sscanf(s, "numMeshes %d\n", &m->num_meshes);
      m->meshes = ALLOCATE(m->num_meshes, Md5_mesh);
    }else if(strcmp_sp(s, "joints {\n")){
      /*puts("joints {");*/
      md5_read_joints(f, m->joints);
    }else if(strcmp_sp(s, "mesh {\n")){
      /*puts("mesh {");*/
      md5_read_mesh(f, m->meshes + mesh_n);
      mesh_n++;
    }else{
      /*puts("...");*/
    }
  }
  fclose(f);
}

void md5_load_hierarchy(FILE *f, Md5_anim *a){
  char s[200];
  int i = 0;
  while(fgets(s, 200, f) != NULL){
    if(s[0] == '}'){
      return;
    }else{
      Md5_hierarchy_item *h;
      char name[40];
      int parent;
      int flags;
      int startIndex;
      /*int n = sscanf(s+1, "\"%[^\"]s\"\t%d %d %d\n",*/
      int n = sscanf(s+1, "%s\t%d %d %d\n",
          name, &parent, &flags, &startIndex);
      if(n != 4){
        printf("load_hier! %d\n", n);
        exit(1);
      }
      h = a->hierarchy + i;
      h->name = my_strdup(name);
      h->parent = parent;
      h->flags = flags;
      h->start_index = startIndex;
    }
    i++;
  }
}

void md5_load_base_frame(FILE *f, Md5_anim *a){
  char s[200];
  int i = 0;
  while(fgets(s, 200, f) != NULL){
    Md5_base_frame_joint *b = a->base_frame + i;
    if(s[0] == '}'){
      return;
    }else{
      V3f pos, rot;
      sscanf(s+1, "( %f %f %f ) ( %f %f %f )\n",
          &pos.x, &pos.y, &pos.z,
          &rot.x, &rot.y, &rot.z);
      b->pos = pos;
      b->orient.x = rot.x;
      b->orient.y = rot.y;
      b->orient.z = rot.z;
      b->orient.w = 0;
    }
    quat_renormalize(&b->orient);
    i++;
  }
}

void md5_load_frame(FILE *f, Md5_anim *a, int n){
  char s[200];
  int i = 0;
  a->frames[n] = ALLOCATE(a->num_animated_components, float);
  while(fgets(s, 200, f) != NULL){
    if(s[0] == '}'){
      return;
    }else{
      /*V3f pos, rot;*/
      /*TODO*/
      sscanf(s+1, "%f %f %f %f %f %f\n",
          &a->frames[n][i+0],
          &a->frames[n][i+1],
          &a->frames[n][i+2],
          &a->frames[n][i+3],
          &a->frames[n][i+4],
          &a->frames[n][i+5]);
      i += 6;
    }
  }
}

void md5_reset_joints(Md5_anim *a){
 int i;
 for (i = 0; i < a->num_joints; i++ ) {
  Md5_joint *j = a->joints + i;
  j->name = a->hierarchy[i].name; /* ? */
  j->parent_index = a->hierarchy[i].parent;
  if(j->parent_index == -1)
    j->parent = NULL;
  else
    j->parent = a->joints + j->parent_index;
  j->pos = a->base_frame[i].pos;
  j->orient = a->base_frame[i].orient;
 }
}

void md5_build_joints(Md5_anim *a){
  int i;
  for(i = 0; i < a->num_joints; i++ ){
    Md5_joint *j = a->joints + i;
    Md5_joint *p = j->parent;
    if(p != NULL){
      j->pos = v3f_plus(p->pos, quat_rot(p->orient, j->pos));
      j->orient = quat_mul(p->orient, j->orient);
    }
  }
}

void md5_load_anim(char *filename, Md5_anim *a){
  FILE *f = fopen(filename, "r");
  char s[200];
  while(fgets(s, 200, f) != NULL){
    if(strcmp_sp(s, "numFrames %d\n")){
      sscanf(s, "numFrames %d\n", &a->num_frames);
      a->frames = ALLOCATE(a->num_frames, float*);
    }else if(strcmp_sp(s, "numJoints %d\n")){
      sscanf(s, "numJoints %d\n", &a->num_joints);
      a->hierarchy = ALLOCATE(a->num_joints, Md5_hierarchy_item);
      a->base_frame = ALLOCATE(a->num_joints, Md5_base_frame_joint);
    }else if(strcmp_sp(s, "frameRate %d\n")){
      /*...*/
    }else if(strcmp_sp(s, "numAnimatedComponents %d\n")){
      sscanf(s, "numAnimatedComponents %d\n",
          &a->num_animated_components);
    }else if(strcmp_sp(s, "hierarchy {\n")){
      md5_load_hierarchy(f, a);
    }else if(strcmp_sp(s, "bounds {\n")){
      /*loadBounds(f, a);*/
    }else if(strcmp_sp(s, "baseframe {\n")){
      md5_load_base_frame(f, a);
    }else if(strcmp_sp(s, "frame %d {\n")){
      int n;
      sscanf(s, "frame %d{\n", &n);
      md5_load_frame(f, a, n);
    }
  }
  fclose(f);
  a->joints = ALLOCATE(a->num_joints, Md5_joint);
  md5_reset_joints(a);
  md5_build_joints(a);
}

void md5_mesh_draw(Md5_mesh *m){
  int i, k;
  glBindTexture(GL_TEXTURE_2D, m->texture);
  glBegin(GL_TRIANGLES);
  for(i = 0; i < m->num_tris; i++){
    for(k = 0; k < 3; k++){
      int index = m->tris[i].index[k];
      V2f tex = m->vertices[index].tex;
      V3f pos = m->points[index];
      glTexCoord2f(tex.x, tex.y);
      glVertex3f(pos.x, pos.y, pos.z);
    }
  }
  glEnd();
}

void md5_model_draw(Md5_model *m){
  int i;
  for(i = 0; i < m->num_meshes; i++)
    md5_mesh_draw(m->meshes + i);
}

void md5_set_frame(Md5_model *m, Md5_anim *a, int n){
  int i;
  if(n < 0)
    n = a->num_frames - 1;
  if(n >= a->num_frames)
    n = 0;
  a->frame = n;
  md5_reset_joints(a);
  for(i = 0; i < a->num_joints; i++){
    int flags = a->hierarchy[i].flags;
    int pos   = a->hierarchy[i].start_index;
    Md5_joint *j = a->joints + i;
    if(flags & 1) j->pos.x = a->frames[n][pos++];
    if(flags & 2) j->pos.y = a->frames[n][pos++];
    if(flags & 4) j->pos.z = a->frames[n][pos++];
    if(flags & 8) j->orient.x = a->frames[n][pos++];
    if(flags & 16) j->orient.y = a->frames[n][pos++];
    if(flags & 32) j->orient.z = a->frames[n][pos++];
    quat_renormalize(&j->orient);
  }
  md5_build_joints(a);
  md5_model_compute(m, a->joints);
}
