/*See LICENSE file for copyright and license details.*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "bool.h"
#include "list.h"
#include "math.h"
#include "core.h"
#include "misc.h"
#include "path.h"

Block3 *map[MAP_Z][MAP_Y][MAP_X];
V3i active_block_pos = {0, 0, 0};
Unit_mode unit_mode = UM_NORMAL;
List units = {NULL, NULL, 0}; /*Unit*/
Unit *selected_unit = NULL;
Mode mode = M_NORMAL;

Unit *unit_at(V3i p){
  Node *node;
  FOR_EACH_NODE(units, node){
    Unit *u = node->data;
    if(v3i_is_equal(u->p, p))
      return u;
  }
  return NULL;
}

void add_unit(V3i p){
  Unit *u = ALLOCATE(1, Unit);
  u->p = p;
  push_node(&units, mk_node(u));
}

bool inboard(V3i p){
  return p.x >= 0 && p.y >= 0 && p.z >= 0
      && p.x < MAP_X && p.y < MAP_Y && p.z < MAP_Z;
}

Block3 *block(V3i p){
  assert(inboard(p));
  return map[p.z][p.y][p.x];
}

/*TODO check if unit can stand here (enough vertical space)*/
bool is_block_walkable(V3i pos){
  Block3 *b;
  if(!inboard(pos))
    return false;
  b = block(pos);
  if(!b)
    return false;
  return b->t == B_FLOOR;
}

bool check_height_diff(V3i p1, V3i p2, int max_diff){
  Block3 *b1;
  Block3 *b2;
  int h1, h2;
  if(!inboard(p1) || !inboard(p2))
    return false;
  b1 = block(p1);
  b2 = block(p2);
  if(!b1 || !b2)
    return false;
  h1 = (p1.z * BLOCK_HEIGHT) + b1->h;
  h2 = (p2.z * BLOCK_HEIGHT) + b2->h;
  return abs(h1 - h2) <= max_diff;
}

static bool check_wall(V3i a_pos, V3i b_pos){
  ushort *a;
  ushort *b;
  Dir d;
  {
    int dx = b_pos.x - a_pos.x;
    int dy = b_pos.y - a_pos.y;
    if(dx > 1) dx = 1;
    if(dx < -1) dx = -1;
    if(dy > 1) dy = 1;
    if(dy < -1) dy = -1;
    a_pos.x = b_pos.x - dx;
    a_pos.y = b_pos.y - dy;
  }
  a = block(a_pos)->walls;
  b = block(b_pos)->walls;
  d = m2dir(a_pos, b_pos);
  if(d == D_ERROR)
    exit(0);
  if(d == D_F && (a[0] || b[1] || b[2])) return false;
  if(d == D_R && (a[1] || a[0] || b[3])) return false;
  if(d == D_FR && (a[0] || a[1] || b[2] || b[3])) return false;
  return true;
}

bool check_xxx(V3i orig_pos, V3i pos, int height){
  return check_height_diff(orig_pos, pos, height)
      && is_block_walkable(pos)
      && check_wall(orig_pos, pos);
}

ushort calc_block_clearence(V3i p, ushort max_size){
  ushort i;
  ushort j;
  ushort h = MAX_HEIGHT_DIFF;
  assert(inboard(p));
  if(block(p) && block(p)->t != B_FLOOR)
    return 0;
  for(i = 1; i < max_size; i++){
    for(j = 0; j <= i; j++){
      bool  b11, b12, b21, b22, b31, b32;
      V3i p11, p12, p21, p22, p31, p32;
      p11 = mk_v3i(p.x + j, p.y + i, p.z + 0);
      p12 = mk_v3i(p.x + i, p.y + j, p.z + 0);
      b11 = check_xxx(p, p11, h);
      b12 = check_xxx(p, p12, h);
      p21 = mk_v3i(p.x + j, p.y + i, p.z + 1);
      p22 = mk_v3i(p.x + i, p.y + j, p.z + 1);
      b21 = check_xxx(p, p21, h);
      b22 = check_xxx(p, p22, h);
      p31 = mk_v3i(p.x + j, p.y + i, p.z - 1);
      p32 = mk_v3i(p.x + i, p.y + j, p.z - 1);
      b31 = check_xxx(p, p31, h);
      b32 = check_xxx(p, p32, h);
      if(!(b11 || b21 || b31) || !(b12 || b22 || b32))
        return i;
    }
  }
  return max_size;
}

void calc_map_clearence(ushort max_size){
  V3i p = {0, 0, 0};
  assert(max_size > 0);
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b)
      b->clearence = calc_block_clearence(p, max_size);
    inc_v3i(&p);
  }
}

void print_block(FILE *f, Block3 *b, int i){
  if(!b){
    fprintf(f, ".");
    return;
  }
  fprintf(f, "%d:: type:%d h:%d walls:(%d %d %d %d) h:(%d %d %d %d)",
      i,
      b->t,
      b->h,
      (int)b->walls[0],
      (int)b->walls[1],
      (int)b->walls[2],
      (int)b->walls[3],
      b->heights[0],
      b->heights[1],
      b->heights[2],
      b->heights[3]);
}

void map_to_file(const char *filename){
  V3i p = {0, 0, 0};
  int i = 0;
  FILE *f = fopen(filename, "w");
  if(!f)
    exit(1);
  while(is_able_to_inc_v3i(&p)){
    print_block(f, block(p), i);
    fprintf(f, "\n");
    i++;
    inc_v3i(&p);
  }
  fclose(f);
}

void calc_block_height(Block3 *b){
  ushort *h = b->heights;
  ushort max = h[0];
  ushort min = h[0];
  int i;
  for(i = 0; i < 4; i++){
    if(h[i] < min)
      min = h[i];
    if(h[i] > max)
      max = h[i];
  }
  b->h = (ushort)(min + max) / 2;
}

void map_from_file(const char *filename){
  V3i p = {0, 0, 0};
  FILE *f = fopen(filename, "r");
  if(!f)
    exit(1);
  while(is_able_to_inc_v3i(&p)){
    char s[200];
    fgets(s, 200, f);
    if(s[0] == '.'){
      map[p.z][p.y][p.x] = NULL;
    }else{
      Block3 *b = map[p.z][p.y][p.x];
      int height, h[4], w[4], t, n;
      if(!b)
        b = ALLOCATE(1, Block3);
      sscanf(s, "%d:: type:%d h:%d walls:(%d %d %d %d) h:(%d %d %d %d)",
          &n, &t, &height,
          &w[0], &w[1], &w[2], &w[3],
          &h[0], &h[1], &h[2], &h[3]);
      b->t = (ushort)t;
      b->h = (ushort)height;
      b->walls[0] = (ushort)w[0];
      b->walls[1] = (ushort)w[1];
      b->walls[2] = (ushort)w[2];
      b->walls[3] = (ushort)w[3];
      b->heights[0] = (ushort)h[0];
      b->heights[1] = (ushort)h[1];
      b->heights[2] = (ushort)h[2];
      b->heights[3] = (ushort)h[3];
      calc_block_height(b);
      b->parent = D_NONE;
      map[p.z][p.y][p.x] = b;
    }
    inc_v3i(&p);
  }
  fclose(f);
}

void select_unit(Unit *u){
  selected_unit = u;
  fill_map(u->p);
}

void map_init(void){
  V3i p = {0, 0, 0};
  while(is_able_to_inc_v3i(&p)){
    map[p.z][p.y][p.x] = NULL;
    inc_v3i(&p);
  }
}

int get_blocks_count(void){
  V3i p = {0, 0, 0};
  int n = 0;
  while(is_able_to_inc_v3i(&p)){
    if(block(p))
      n++;
    inc_v3i(&p);
  }
  return n;
}

int get_walls_count(void){
  V3i p = {0, 0, 0};
  int n = 0;
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b){
      if(b->walls[0]) n++;
      if(b->walls[1]) n++;
      if(b->walls[2]) n++;
      if(b->walls[3]) n++;
    }
    inc_v3i(&p);
  }
  return n;
}

int get_path_lines_count(void){
  V3i p = {0, 0, 0};
  int n = 0;
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b && b->parent != D_NONE)
      n++;
    inc_v3i(&p);
  }
  return n;
}