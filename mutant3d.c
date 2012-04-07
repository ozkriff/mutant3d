/*See LICENSE file for copyright and license details.*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_scancode.h>
#include <SDL_image.h>
#include "bool.h"
#include "list.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"
#include "md5.h"
#include "obj.h"
#include "gl.h"
#include "path.h"

float rotate_x = 45.0f;
float rotate_z = 180.0f;
const float rotations_per_tick = 1.5;
V3f map_pos = {0, 0, 0};
float zoom = 100.0f;
bool done = false;
V3i active_block_pos = {0, 0, 0};

typedef enum {
  M_NORMAL,
  M_SET_WALLS,
  M_SET_HEIGHTS,
  M_COUNT
} Mode;

Mode mode = M_NORMAL;

#define BLOCK_SIZE 1.0f

GLfloat LightAmbient[4] = {0.1f, 0.1f, 0.1f, 0.2f};
GLfloat LightDiffuse[4] = {0.5, 0.5, 0.5, 0.5};
GLfloat LightPosition[4] = {
  BLOCK_SIZE * (MAP_X / 2.0),
  BLOCK_SIZE * (MAP_Y / 2.0),
  BLOCK_SIZE * MAP_Z,
  1.0};

Block3 *map[MAP_Z][MAP_Y][MAP_X];

float *map_verts = NULL; /*ground*/
float *pick_verts = NULL;
float *walls_verts = NULL;
float *obj_verts = NULL;
float *path_verts = NULL;
int map_verts_count = 0;
int pick_verts_count = 0;
int walls_verts_count = 0;
int obj_verts_count = 0;
int path_verts_count = 0;

float *map_tex_coord = NULL;
float *wall_tex_coord = NULL;
float *obj_tex_coord = NULL;

GLubyte *pick_colors = NULL;

bool enabled_levels[20] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

Md5_model m;
Md5_anim anim;

Obj_model obj_m;
GLuint obj_tex;

GLuint floor_texture;
GLuint wall_texture;

static SDL_GLContext context;
SDL_Window *win = NULL;

V2i mouse_pos = {0, 0};

/*TODO: remove me*/
void build_map_array(void);
void build_walls_array(void);
void build_path_array(void);
bool pick_block(V3i *block);
void build_picking_blocks_array(void);

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

/*TODO стена должна отображаться сразу у двух клеток.*/
static bool check_wall(V3i a_pos, V3i b_pos){
  bool *a;
  bool *b;
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
#if 0
  if(d == D_F && (a[0] || b[1] || b[2])) return false;
  if(d == D_R && (a[1] || b[0] || b[3])) return false;
  if(d == D_FR && (a[0] || a[1] || b[2] || b[3])) return false;
#else
  if(d == D_F && (a[0] || b[1] || b[2])) return false;
  if(d == D_R && (a[1] || a[0] || b[3])) return false;
  if(d == D_FR && (a[0] || a[1] || b[2] || b[3])) return false;
#endif
  return true;
}

bool check_xxx(V3i orig_pos, V3i pos, int height){
  return check_height_diff(orig_pos, pos, height)
      && is_block_walkable(pos)
      && check_wall(orig_pos, pos);
}

/*TODO:
  Проверять разницу в высоте не с начальным блоком,
  а с прилегающими!*/
int calc_block_clearence(V3i p, int max_size){
  int i, j;
  int h = MAX_HEIGHT_DIFF;
  assert(inboard(p));
  assert(max_size >= 0);
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

void calc_map_clearence(int max_size){
  V3i p = {0, 0, 0};
  assert(max_size > 0);
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b)
      b->clearence = calc_block_clearence(p, max_size);
    inc_v3i(&p);
  }
}

void shut_down(int return_code){
  /*glfwTerminate();*/
  exit(return_code);
}

void set_camera(void){
  glTranslatef(0, 0, -zoom);
  glRotatef(rotate_x, -1, 0, 0);
  glRotatef(rotate_z, 0, 0, 1);
  glTranslatef(
      -BLOCK_SIZE * (MAP_X / 2),
      -BLOCK_SIZE * (MAP_Y / 2),
      -BLOCK_SIZE * (MAP_Z / 2));
  glTranslatef(map_pos.x, map_pos.y, 0);
}

void draw_active_block(V3i p){
  float n = BLOCK_SIZE / 2.0f;
  float x = (float)p.x * BLOCK_SIZE;
  float y = (float)p.y * BLOCK_SIZE;
  glColor3f(0.0f, 0.0f, 1.0f);
  glBegin(GL_LINE_STRIP);
  glVertex3f(x, y, 0);
  glVertex3f(x, y, (float)(MAP_Z - 1) * (BLOCK_SIZE*2) + 4*n);
  glEnd();
  glTranslatef(x, y,
      (float)p.z * BLOCK_SIZE * 2);
  glBegin(GL_LINES);
  glVertex3f(-n, -n, 0); glVertex3f(-n, -n, n*4);
  glVertex3f(+n, -n, 0); glVertex3f(+n, -n, n*4);
  glVertex3f(+n, +n, 0); glVertex3f(+n, +n, n*4);
  glVertex3f(-n, +n, 0); glVertex3f(-n, +n, n*4);
  glEnd();
}

void draw(void){
  glLoadIdentity();
  glPushMatrix();
  set_camera();
  glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  {
    glBindTexture(GL_TEXTURE_2D, floor_texture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoordPointer(2, GL_FLOAT, 0, map_tex_coord);
    glVertexPointer(3, GL_FLOAT, 0, map_verts);
    glDrawArrays(GL_QUADS, 0, map_verts_count);
  }
  {
    glBindTexture(GL_TEXTURE_2D, wall_texture);
    glColor3f(0.6f, 0.8f, 0.4f);
    glTexCoordPointer(2, GL_FLOAT, 0, wall_tex_coord);
    glVertexPointer(3, GL_FLOAT, 0, walls_verts);
    glDrawArrays(GL_QUADS, 0, walls_verts_count);
  }
  if(0){
    int i;
    glBindTexture(GL_TEXTURE_2D, obj_tex);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glColor3f(0, 0, 1);
    glScalef(0.2f, 0.2f, 0.2f);
    for(i = 0; i < 25; i++){
      if(i != 0 && i % 5 == 0)
        glTranslatef(4 * 5, 0, 8);
      glTranslatef(-4, 0, 0);
      glTexCoordPointer(2, GL_FLOAT, 0, obj_tex_coord);
      glVertexPointer(3, GL_FLOAT, 0, obj_verts);
      glDrawArrays(GL_TRIANGLES, 0, obj_verts_count);
    }
    glLineWidth(1.0);
    glPopMatrix();
  }
  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  {
    glLineWidth(2.0);
    glColor3f(0.6f, 0.2f, 0.2f);
    glVertexPointer(3, GL_FLOAT, 0, path_verts);
    glDrawArrays(GL_LINES, 0, path_verts_count);
    glLineWidth(1.0);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
  {
    glLineWidth(2.0);
    draw_active_block(active_block_pos);
    glLineWidth(1.0);
  }
  if(0){
    glEnable(GL_TEXTURE_2D);
    obj_draw(obj_tex, &obj_m);
    glDisable(GL_TEXTURE_2D);
  }
  glPopMatrix();
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
#if 0
      if(p.z == 0){
        map[p.z][p.y][p.x] = ALLOCATE(1, Block3);
        map[p.z][p.y][p.x]->t = B_FLOOR;
      }
#endif
    }else{
      Block3 *b = map[p.z][p.y][p.x];
      int h[4], w[4], t, n;
      if(!b)
        b = ALLOCATE(1, Block3);
      sscanf(s, "%d:: type:%d h:%d walls:(%d %d %d %d) h:(%d %d %d %d)",
          &n, &t, &b->h,
          &w[0], &w[1], &w[2], &w[3],
          &h[0], &h[1], &h[2], &h[3]);
      b->t = (Block_type_id)t;
      b->walls[0] = (bool)w[0];
      b->walls[1] = (bool)w[1];
      b->walls[2] = (bool)w[2];
      b->walls[3] = (bool)w[3];
      b->heights[0] = h[0];
      b->heights[1] = h[1];
      b->heights[2] = h[2];
      b->heights[3] = h[3];
      b->parent.x = -1;
      map[p.z][p.y][p.x] = b;
    }
    inc_v3i(&p);
  }
  fclose(f);
}

void keys_callback(SDL_KeyboardEvent e) {
  SDL_Keycode key = e.keysym.sym;
  Uint8 *state = SDL_GetKeyboardState(NULL);
  if(e.type != SDL_KEYDOWN)
    return;
  if(mode == M_NORMAL){
    if(key >= '1' && key <= '9'){
      int n = key - '1';
      enabled_levels[n] = !enabled_levels[n];
      build_walls_array();
      build_map_array();
      build_path_array();
    }
  }
  if(state[SDL_SCANCODE_H] && active_block_pos.x > 0)
    active_block_pos.x--;
  else if(state[SDL_SCANCODE_J] && active_block_pos.y < MAP_Y - 1)
    active_block_pos.y++;
  else if(state[SDL_SCANCODE_K] && active_block_pos.y > 0)
    active_block_pos.y--;
  else if(state[SDL_SCANCODE_L] && active_block_pos.x < MAP_X - 1)
    active_block_pos.x++;
  else if(key == SDLK_d && active_block_pos.z > 0){
    active_block_pos.z--;
    build_picking_blocks_array();
  }else if(key == SDLK_u && active_block_pos.z < MAP_Z - 1){
    active_block_pos.z++;
    build_picking_blocks_array();
  }else if(key == SDLK_m){
    mode++;
    if(mode == M_COUNT)
      mode = M_NORMAL;
  }else if(key == SDLK_x){
    fill_map(active_block_pos);
    build_path_array();
  }else if(key == SDLK_s)
    map_to_file("out.map");
  else if(key == 'Z'){
    map_from_file("out.map");
    build_map_array();
  }
  {
    Block3 *b = block(active_block_pos);
    if(key == SDLK_EQUALS && b){
      b->h++;
      build_map_array();
    }else if(key == SDLK_MINUS && b){
      b->h--;
      build_map_array();
    }else if(key == SDLK_t){
      if(b){
        free(b);
        map[active_block_pos.z][active_block_pos.y][active_block_pos.x] = NULL;
      }else{
        b = ALLOCATE(1, Block3);
        b->t = B_FLOOR;
        b->h = 0;
        b->parent = mk_v3i(-1, -1, -1);
        map[active_block_pos.z][active_block_pos.y][active_block_pos.x] = b;
        calc_map_clearence(3);
      }
      build_map_array();
    }
  }
  if(mode == M_SET_WALLS){
    Block3 *b = block(active_block_pos);
    if(b){
      bool *w = b->walls;
      if(key == '1') w[0] = (w[0]) ? 0 : 1;
      if(key == '2') w[1] = (w[1]) ? 0 : 1;
      if(key == '3') w[2] = (w[2]) ? 0 : 1;
      if(key == '4') w[3] = (w[3]) ? 0 : 1;
      build_walls_array();
    }
  } else if(mode == M_SET_HEIGHTS){
    Block3 *b = block(active_block_pos);
    if(b){
      int *h = b->heights;
      if(key == '1') h[0] = (h[0]) ? 0 : 1;
      if(key == '2') h[1] = (h[1]) ? 0 : 1;
      if(key == '3') h[2] = (h[2]) ? 0 : 1;
      if(key == '4') h[3] = (h[3]) ? 0 : 1;
      build_map_array();
    }
  }
}

void keys(void){
  Uint8 *state = SDL_GetKeyboardState(NULL);
  if(state[SDL_SCANCODE_ESCAPE] || state[SDL_SCANCODE_Q]){
    done = true;
  }else if(state[SDL_SCANCODE_LCTRL]){
    if(state[SDL_SCANCODE_UP]){
      map_pos.x -= (float)sin(deg2rad(rotate_z)) * (zoom / 40.0f);
      map_pos.y -= (float)cos(deg2rad(rotate_z)) * (zoom / 40.0f);
    }else if(state[SDL_SCANCODE_DOWN]){
      map_pos.x += (float)sin(deg2rad(rotate_z)) * (zoom / 40.0f);
      map_pos.y += (float)cos(deg2rad(rotate_z)) * (zoom / 40.0f);
    }
    if(state[SDL_SCANCODE_LEFT]){
      map_pos.x -= (float)sin(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
      map_pos.y -= (float)cos(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
    }else if(state[SDL_SCANCODE_RIGHT]){
      map_pos.x += (float)sin(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
      map_pos.y += (float)cos(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
    }
  }else if(state[SDL_SCANCODE_LALT]){
    if(state[SDL_SCANCODE_UP] && zoom > 5)
      zoom -= 1;
    else if(state[SDL_SCANCODE_DOWN] && zoom < 100)
      zoom += 1;
  }else{
    if(state[SDL_SCANCODE_LEFT]){
      rotate_z -= rotations_per_tick;
      if(rotate_z < 0)
        rotate_z += 360;
    }else if(state[SDL_SCANCODE_RIGHT]){
      rotate_z += rotations_per_tick;
      if(rotate_z > 360)
        rotate_z -= 360;
    }
    if(state[SDL_SCANCODE_UP] && rotate_x > 0)
      rotate_x -= rotations_per_tick;
    else if(state[SDL_SCANCODE_DOWN] && rotate_x < 100)
      rotate_x += rotations_per_tick;
  }
}

void print_world(void){
  V3f v;
  win2world(mouse_pos.x, mouse_pos.y, &v);
  printf("%3d:%3d [% 3.3f % 3.3f % 3.3f]\n",
      mouse_pos.x, mouse_pos.y, v.x, v.y, v.z);
}

static void
mousemove(SDL_MouseMotionEvent e){
  mouse_pos.x = (int)e.x;
  mouse_pos.y = (int)e.y;
}

void events(void){
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_WINDOWEVENT:
        switch (e.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            if (win)
              SDL_DestroyWindow(win);
            break;
          default:
            /* TODO */
            /*die("events(): default case for"
                " window.event %d\n", e.window.type);*/
            break;
        }
        break;
      case SDL_KEYDOWN:
        keys_callback(e.key);
        break;
      case SDL_MOUSEMOTION:
        mousemove(e.motion);
        break;
      default:
	/* TODO */
        /*die("events(): default case for type %d\n", e.type);*/
        break;
    }
  }
}

void draw_for_picking(void); /* TODO remove */

void main_loop(void){
  while(!done) {
    events();
    keys();
    {
      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      draw_for_picking();
      pick_block(&active_block_pos);
    }
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw();
    SDL_GL_SwapWindow(win);
    /* print_world(); */
    SDL_Delay(10);
  }
}

void init(void) {
  const int window_width = 700;
  const int window_height = 500;
  float aspect_ratio = (float)window_height / (float)window_width;
  SDL_VideoInit(NULL);
  win = SDL_CreateWindow("SDL TEST",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      window_width, window_height, SDL_WINDOW_OPENGL);
  context = SDL_GL_CreateContext(win);
  IMG_Init(0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(0.5, -0.5, -0.5 * aspect_ratio, 0.5 * aspect_ratio, 1, 500);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  /*glEnable(GL_LIGHTING);*/
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  /*glEnable(GL_CULL_FACE);*/
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
  glEnable(GL_COLOR_MATERIAL);
  /*glColorMaterial(GL_FRONT,GL_DIFFUSE);*/
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
#if 0
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth(1.0);
#endif
  /* glfwSetKeyCallback(keys_callback); */
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
  glEnable(GL_LIGHT1);
}

void md5_init(void){
  md5_load_model(&m, "data/guard/bob.md5mesh");
  md5_load_anim("data/guard/bob.md5anim", &anim);
  md5_model_compute(&m, anim.joints);
#if 0
  md5_model_debug_print(&m);
  md5_anim_debug_print(&anim);
#endif
  md5_set_frame(&m, &anim, 0);
}

void obj_init(void){
  obj_read(&obj_m, "data/obj_test/model.obj");
  if(!load_texture("data/obj_test/test.tga", &obj_tex))
    die("blah blah blah");
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
    if(b && b->parent.x != -1)
      n++;
    inc_v3i(&p);
  }
  return n;
}

void set_xy(float *coords, int n, int i, int vi, float x, float y){
  float *coord;
  assert(n == 2 || n == 3 || n == 4);
  assert(vi >= 0 && vi <= n);
  assert(i >= 0);
  coord = coords + (i * n * 2) + (2 * vi);
  *(coord + 0) = x;
  *(coord + 1) = y;
}

/*if(quads) n=4; if(tris) n=3*/
/* i - polygon number, vi - vertex number*/
void set_xyz(float *verts, int n, int i, int vi, float x, float y, float z){
  float *vertex;
  assert(n == 2 || n == 3 || n == 4);
  assert(vi >= 0 && vi <= n);
  assert(i >= 0);
  vertex = verts + (i * n * 3) + (3 * vi);
  *(vertex + 0) = x;
  *(vertex + 1) = y;
  *(vertex + 2) = z;
}

/* TODO rename arguments */
void set_rgb(GLubyte *colors, int n, int i, int vi, GLubyte r, GLubyte g, GLubyte b){
  GLubyte *color;
  assert(n == 2 || n == 3 || n == 4);
  assert(vi >= 0 && vi <= n);
  assert(i >= 0);
  color = colors + (i * n * 3) + (3 * vi);
  color[0] = r;
  color[1] = g;
  color[2] = b;
}

void build_obj(Obj_model *model){
  int i, j;
  obj_verts_count = model->f_count * 3;
  obj_verts = ALLOCATE(obj_verts_count * 3, float);
  obj_tex_coord = ALLOCATE(obj_verts_count * 2, float);
  for(i = 0; i < model->f_count; i++){
    Obj_triangle *tri = model->faces + i;
    for(j = 0; j < 3; j++){
      int vert_id = tri->v[j] - 1;
      int tex_id = tri->t[j] - 1;
      V3f *vert = model->vertexes + vert_id;
      V2f *tex = model->text_coords + tex_id;
      set_xyz(obj_verts, 3, i, j, vert->x, vert->y, vert->z);
      set_xy(obj_tex_coord, 2, i, j, tex->x, tex->y);
    }
  }
}

V3f v3i_to_v3f(V3i i){
  V3f f;
  Block3 *b;
  assert(inboard(i));
  b = block(i);
  f.x = (float)i.x * BLOCK_SIZE;
  f.y = (float)i.y * BLOCK_SIZE;
  f.z = (float)i.z * (BLOCK_SIZE*2) + ((BLOCK_SIZE*2) / BLOCK_HEIGHT) * (float)b->h;
  return f;
}

void build_map_array(void){
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  map_verts_count = get_blocks_count() * 4;
  if(map_verts)
    free(map_verts);
  if(map_tex_coord)
    free(map_tex_coord);
  map_verts = ALLOCATE(map_verts_count * 3, float);
  map_tex_coord = ALLOCATE(map_verts_count * 2, float);
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b && enabled_levels[p.z]){
      int *h = b->heights;
      float n = BLOCK_SIZE / 2.0;
      float n2 = (BLOCK_HEIGHT / 32.0f) * 4;
      V3f pos = v3i_to_v3f(p);
      set_xyz(map_verts, 4, i, 0, pos.x - n, pos.y - n, pos.z + (h[0] ? n2 : 0));
      set_xyz(map_verts, 4, i, 1, pos.x + n, pos.y - n, pos.z + (h[1] ? n2 : 0));
      set_xyz(map_verts, 4, i, 2, pos.x + n, pos.y + n, pos.z + (h[2] ? n2 : 0));
      set_xyz(map_verts, 4, i, 3, pos.x - n, pos.y + n, pos.z + (h[3] ? n2 : 0));
      set_xy(map_tex_coord, 4, i, 0, 0.0f, 0.0f);
      set_xy(map_tex_coord, 4, i, 1, 1.0f, 0.0f);
      set_xy(map_tex_coord, 4, i, 2, 1.0f, 1.0f);
      set_xy(map_tex_coord, 4, i, 3, 0.0f, 1.0f);
      i++;
    }
    inc_v3i(&p);
  }
}

void build_path_array(void){
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  path_verts_count = get_path_lines_count() * 2;
  if(path_verts_count == 0)
    return;
  assert(path_verts_count > 0);
  if(path_verts)
    free(path_verts);
  path_verts = ALLOCATE(path_verts_count * 3, float);
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b && enabled_levels[p.z] && b->parent.x != -1){
      V3f pos1 = v3i_to_v3f(p);
      V3f pos2 = v3i_to_v3f(b->parent);
      set_xyz(path_verts, 2, i, 0, pos1.x, pos1.y, pos1.z + 0.1f);
      set_xyz(path_verts, 2, i, 1, pos2.x, pos2.y, pos2.z + 0.1f);
      i++;
    }
    inc_v3i(&p);
  }

}

void build_walls_array(void){
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  walls_verts_count = get_walls_count() * 4;
  if(walls_verts_count == 0)
    return;
  assert(walls_verts_count > 0);
  if(walls_verts)
    free(walls_verts);
  if(wall_tex_coord)
    free(wall_tex_coord);
  walls_verts = ALLOCATE(walls_verts_count * 3, float);
  wall_tex_coord = ALLOCATE(walls_verts_count * 2, float);
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b && enabled_levels[p.z]){
      float n = BLOCK_SIZE / 2.0;
      V3f pos = v3i_to_v3f(p);
      if(b->walls[0]){
        set_xyz(walls_verts, 4, i, 0, pos.x + n, pos.y - n, pos.z);
        set_xyz(walls_verts, 4, i, 1, pos.x + n, pos.y + n, pos.z);
        set_xyz(walls_verts, 4, i, 2, pos.x + n, pos.y + n, pos.z + 4*n);
        set_xyz(walls_verts, 4, i, 3, pos.x + n, pos.y - n, pos.z + 4*n);
        set_xy(wall_tex_coord, 4, i, 0, 0.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 1, 1.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 2, 1.0f, 1.0f);
        set_xy(wall_tex_coord, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
      if(b->walls[1]){
        set_xyz(walls_verts, 4, i, 0, pos.x - n, pos.y + n, pos.z);
        set_xyz(walls_verts, 4, i, 1, pos.x + n, pos.y + n, pos.z);
        set_xyz(walls_verts, 4, i, 2, pos.x + n, pos.y + n, pos.z + 4*n);
        set_xyz(walls_verts, 4, i, 3, pos.x - n, pos.y + n, pos.z + 4*n);
        set_xy(wall_tex_coord, 4, i, 0, 0.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 1, 1.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 2, 1.0f, 1.0f);
        set_xy(wall_tex_coord, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
      if(b->walls[2]){
        set_xyz(walls_verts, 4, i, 0, pos.x - n, pos.y - n, pos.z);
        set_xyz(walls_verts, 4, i, 1, pos.x - n, pos.y + n, pos.z);
        set_xyz(walls_verts, 4, i, 2, pos.x - n, pos.y + n, pos.z + 4*n);
        set_xyz(walls_verts, 4, i, 3, pos.x - n, pos.y - n, pos.z + 4*n);
        set_xy(wall_tex_coord, 4, i, 0, 0.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 1, 1.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 2, 1.0f, 1.0f);
        set_xy(wall_tex_coord, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
      if(b->walls[3]){
        set_xyz(walls_verts, 4, i, 0, pos.x - n, pos.y - n, pos.z);
        set_xyz(walls_verts, 4, i, 1, pos.x + n, pos.y - n, pos.z);
        set_xyz(walls_verts, 4, i, 2, pos.x + n, pos.y - n, pos.z + 4*n);
        set_xyz(walls_verts, 4, i, 3, pos.x - n, pos.y - n, pos.z + 4*n);
        set_xy(wall_tex_coord, 4, i, 0, 0.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 1, 1.0f, 0.0f);
        set_xy(wall_tex_coord, 4, i, 2, 1.0f, 1.0f);
        set_xy(wall_tex_coord, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
    }
    inc_v3i(&p);
  }
}

void build_picking_blocks_array(void){
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  pick_verts_count = MAP_X * MAP_Y * 4;
  if(pick_verts)
    free(pick_verts);
  if(pick_colors)
    free(pick_colors);
  pick_verts = ALLOCATE(pick_verts_count * 3, float);
  pick_colors = ALLOCATE(pick_verts_count * 3, GLubyte);
  for(p.y = 0; p.y < MAP_Y; p.y++){
    for(p.x = 0; p.x < MAP_X; p.x++){
      float n = BLOCK_SIZE / 2.0;
      V3f pos;
      pos.x = BLOCK_SIZE * (float)p.x;
      pos.y = BLOCK_SIZE * (float)p.y;
      pos.z = BLOCK_SIZE * 2 * (float)active_block_pos.z;
      set_xyz(pick_verts, 4, i, 0, pos.x - n, pos.y - n, pos.z);
      set_xyz(pick_verts, 4, i, 1, pos.x + n, pos.y - n, pos.z);
      set_xyz(pick_verts, 4, i, 2, pos.x + n, pos.y + n, pos.z);
      set_xyz(pick_verts, 4, i, 3, pos.x - n, pos.y + n, pos.z);
      set_rgb(pick_colors, 4, i, 0, (GLubyte)p.x, (GLubyte)p.y, 1);
      set_rgb(pick_colors, 4, i, 1, (GLubyte)p.x, (GLubyte)p.y, 1);
      set_rgb(pick_colors, 4, i, 2, (GLubyte)p.x, (GLubyte)p.y, 1);
      set_rgb(pick_colors, 4, i, 3, (GLubyte)p.x, (GLubyte)p.y, 1);
      i++;
    }
  }
}

void draw_for_picking(void){
  glLoadIdentity();
  glPushMatrix();
  set_camera();
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  {
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, pick_colors);
    glVertexPointer(3, GL_FLOAT, 0, pick_verts);
    glDrawArrays(GL_QUADS, 0, pick_verts_count);
  }
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();
}

bool pick_block(V3i *p){
  GLint viewport[4];
  GLubyte pixel[3];
  assert(p);
  glGetIntegerv(GL_VIEWPORT, viewport);
  glReadPixels(mouse_pos.x, viewport[3] - mouse_pos.y,
      1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*)pixel);
  if(pixel[2] == 0)
    return false;
  p->x = pixel[0];
  p->y = pixel[1];
  return true;
}

int main(void){
  init();
  map_init();
#if 1
  map_from_file("out.map");
#else
  {
    int x, y;
    for(y = 0; y < MAP_Y; y++){
      for(x = 0; x < MAP_X; x++){
        map[0][y][x] = ALLOCATE(1, Block3);
        map[0][y][x]->t = B_FLOOR;
      }
    }
  }
#endif
  calc_map_clearence(3);
  obj_init();
  load_texture("data/floor.tga", &floor_texture);
  load_texture("data/wall.tga", &wall_texture);
  build_map_array();
  build_obj(&obj_m);
  build_walls_array();
#if 0
  md5_init();
#endif
  build_picking_blocks_array();
  main_loop();
  shut_down(0);
  return 0;
}
