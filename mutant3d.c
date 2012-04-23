/*See LICENSE file for copyright and license details.*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <SDL.h>
#include "GL/gl.h"
#include <SDL_opengl.h>
#include <SDL_scancode.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "bool.h"
#include "list.h"
#include "math.h"
#include "core.h"
#include "mutant3d.h"
#include "misc.h"
#include "md5.h"
#include "obj.h"
#include "gl.h"
#include "path.h"
#include "widgets.h"

float rotate_x = 45.0f;
float rotate_z = 180.0f;
const float rotations_per_tick = 1.5;
V3f map_pos = {0, 0, 0};
float zoom = 100.0f;
bool done = false;

int last_move_index;
int current_move_index;
List move_path = {NULL, NULL, 0};

TTF_Font *font = NULL;

#define BLOCK_SIZE 1.0f
#define BLOCK_SIZE_2 (BLOCK_SIZE * 2.0f)

#define MOVE_SPEED 10 /*frames count*/

GLfloat LightAmbient[4] = {0.1f, 0.1f, 0.1f, 0.2f};
GLfloat LightDiffuse[4] = {0.5, 0.5, 0.5, 0.5};
GLfloat LightPosition[4] = {
  BLOCK_SIZE *(MAP_X / 2.0),
  BLOCK_SIZE *(MAP_Y / 2.0),
  BLOCK_SIZE * MAP_Z,
  1.0
};

V2i window_size = {640, 480};

Va va_map;
Va va_pick;
Va va_walls;
Va va_obj;
Va va_path;

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

V3f v3i_to_v3f(V3i i)
{
  V3f f;
  Block3 *b;
  assert(inboard(i));
  b = block(i);
  f.x = (float)i.x * BLOCK_SIZE;
  f.y = (float)i.y * BLOCK_SIZE;
  f.z = (float)i.z * BLOCK_SIZE_2;
  if (b) {
    f.z += (BLOCK_SIZE_2 / BLOCK_HEIGHT) * (float)b->h;
  }
  return f;
}

void shut_down(int return_code)
{
  /*glfwTerminate();*/
  exit(return_code);
}

void set_camera(void)
{
  glTranslatef(0, 0, -zoom);
  glRotatef(rotate_x, -1, 0, 0);
  glRotatef(rotate_z, 0, 0, 1);
  glTranslatef(
      -BLOCK_SIZE * (MAP_X / 2),
      -BLOCK_SIZE * (MAP_Y / 2),
      -BLOCK_SIZE * (MAP_Z / 2));
  glTranslatef(map_pos.x, map_pos.y, 0);
}

void draw_active_block(V3i p)
{
  GLfloat va[28 * 3] = {0};
  float n = BLOCK_SIZE / 2.0f;
  float x = (float)p.x * BLOCK_SIZE;
  float y = (float)p.y * BLOCK_SIZE;
  float z = (float)p.z * BLOCK_SIZE_2;
  set_xyz(va, 2, 0, 0, x, y, 0);
  set_xyz(va, 2, 0, 1, x, y, z);
  set_xyz(va, 2, 1, 0, x, y, (float)(p.z + 1) * BLOCK_SIZE_2);
  set_xyz(va, 2, 1, 1, x, y, (float)MAP_Z * BLOCK_SIZE_2);
  /*vertical lines*/
  set_xyz(va, 2, 2, 0, x - n, y - n, z);
  set_xyz(va, 2, 2, 1, x - n, y - n, z + n * 4);
  set_xyz(va, 2, 3, 0, x + n, y - n, z);
  set_xyz(va, 2, 3, 1, x + n, y - n, z + n * 4);
  set_xyz(va, 2, 4, 0, x + n, y + n, z);
  set_xyz(va, 2, 4, 1, x + n, y + n, z + n * 4);
  set_xyz(va, 2, 5, 0, x - n, y + n, z);
  set_xyz(va, 2, 5, 1, x - n, y + n, z + n * 4);
  /*bottom*/
  set_xyz(va, 2, 6, 0, x - n, y - n, z);
  set_xyz(va, 2, 6, 1, x + n, y - n, z);
  set_xyz(va, 2, 7, 0, x + n, y - n, z);
  set_xyz(va, 2, 7, 1, x + n, y + n, z);
  set_xyz(va, 2, 8, 0, x + n, y + n, z);
  set_xyz(va, 2, 8, 1, x - n, y + n, z);
  set_xyz(va, 2, 9, 0, x - n, y + n, z);
  set_xyz(va, 2, 9, 1, x - n, y - n, z);
  /*top*/
  set_xyz(va, 2, 10, 0, x - n, y - n, z + n * 4);
  set_xyz(va, 2, 10, 1, x + n, y - n, z + n * 4);
  set_xyz(va, 2, 11, 0, x + n, y - n, z + n * 4);
  set_xyz(va, 2, 11, 1, x + n, y + n, z + n * 4);
  set_xyz(va, 2, 12, 0, x + n, y + n, z + n * 4);
  set_xyz(va, 2, 12, 1, x - n, y + n, z + n * 4);
  set_xyz(va, 2, 13, 0, x - n, y + n, z + n * 4);
  set_xyz(va, 2, 13, 1, x - n, y - n, z + n * 4);
  glColor3f(0, 0, 1);
  glVertexPointer(3, GL_FLOAT, 0, va);
  glDrawArrays(GL_LINES, 0, 28);
}

void draw_unit(Unit *u)
{
  V3f p = v3i_to_v3f(u->p);
  float unit_radious = (float)(u->size - 1) / 2.0f;
  glPushMatrix();
  glTranslatef(p.x + unit_radious, p.y + unit_radious, p.z);
  if (u->size == 1) {
    glScalef(0.2f, 0.2f, 0.2f);
  } else if (u->size == 2) {
    glScalef(0.3f, 0.3f, 0.3f);
  } else if (u->size == 3) {
    glScalef(0.5f, 0.5f, 0.4f);
  }
  glRotatef(90, 1, 0, 0);
  glTexCoordPointer(2, GL_FLOAT, 0, va_obj.t);
  glVertexPointer(3, GL_FLOAT, 0, va_obj.v);
  glDrawArrays(GL_TRIANGLES, 0, va_obj.count);
  glPopMatrix();
}

void draw_units(void)
{
  Node *node;
  FOR_EACH_NODE(units, node) {
    Unit *u = node->data;
    if (unit_mode == UM_MOVING && u == selected_unit) {
      continue;
    }
    draw_unit(u);
  }
}

void get_current_moving_nodes(V3i *from, V3i *to)
{
  Node *node;
  int i = current_move_index;
  FOR_EACH_NODE(move_path, node) {
    i -= MOVE_SPEED;
    if (i < 0) {
      break;
    }
  }
  *from = *(V3i *)node->data;
  *to = *(V3i *)node->next->data;
}

void end_movement(V3i pos)
{
  while (move_path.count > 0) {
    delete_node(&move_path, move_path.head);
  }
  unit_mode = UM_NORMAL;
  selected_unit->p = pos;
  fill_map(selected_unit->p, selected_unit->size);
  build_path_array();
}

void draw_moving_unit(void)
{
  float unit_radious = (float)(selected_unit->size - 1) / 2.0f;
  V3i from_i, to_i;
  V3f from_f, to_f;
  int node_index;
  V3f diff;
  V3f p;
  get_current_moving_nodes(&from_i, &to_i);
  from_f = v3i_to_v3f(from_i);
  to_f = v3i_to_v3f(to_i);
  diff = v3f_divide_float(v3f_subt(to_f, from_f),
      MOVE_SPEED);
  node_index = current_move_index % MOVE_SPEED;
  p = v3f_plus(from_f,
      v3f_mul_float(diff, (float)node_index));
  glPushMatrix();
  glTranslatef(p.x + unit_radious, p.y + unit_radious, p.z);
  if (selected_unit->size == 1) {
    glScalef(0.2f, 0.2f, 0.2f);
  } else if (selected_unit->size == 2) {
    glScalef(0.3f, 0.3f, 0.3f);
  } else if (selected_unit->size == 3) {
    glScalef(0.5f, 0.5f, 0.4f);
  }
  glRotatef(90, 1, 0, 0);
  glRotatef(get_rot_angle(from_f, to_f), 0, 1, 0);
  glTexCoordPointer(2, GL_FLOAT, 0, va_obj.t);
  glVertexPointer(3, GL_FLOAT, 0, va_obj.v);
  glDrawArrays(GL_TRIANGLES, 0, va_obj.count);
  glPopMatrix();
  current_move_index++;
  if (current_move_index == last_move_index) {
    end_movement(to_i);
  }
}

void draw_map(void)
{
  glBindTexture(GL_TEXTURE_2D, floor_texture);
  glColor3f(1.0f, 1.0f, 1.0f);
  glTexCoordPointer(2, GL_FLOAT, 0, va_map.t);
  glVertexPointer(3, GL_FLOAT, 0, va_map.v);
  glDrawArrays(GL_QUADS, 0, va_map.count);
}

void draw_walls(void)
{
  glBindTexture(GL_TEXTURE_2D, wall_texture);
  glColor3f(0.6f, 0.8f, 0.4f);
  glTexCoordPointer(2, GL_FLOAT, 0, va_walls.t);
  glVertexPointer(3, GL_FLOAT, 0, va_walls.v);
  glDrawArrays(GL_QUADS, 0, va_walls.count);
}

void draw_path(void)
{
  glLineWidth(2.0);
  glColor3f(0.6f, 0.2f, 0.2f);
  glVertexPointer(3, GL_FLOAT, 0, va_path.v);
  glDrawArrays(GL_LINES, 0, va_path.count);
  glLineWidth(1.0);
}

void draw(void)
{
  glLoadIdentity();
  glPushMatrix();
  set_camera();
  glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  draw_map();
  draw_walls();
  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  draw_path();
  draw_active_block(active_block_pos);
  draw_units();
  if (unit_mode == UM_MOVING) {
    draw_moving_unit();
  }
  glDisableClientState(GL_VERTEX_ARRAY);
  if (0) {
    glEnable(GL_TEXTURE_2D);
    obj_draw(obj_tex, &obj_m);
    glDisable(GL_TEXTURE_2D);
  }
  glPopMatrix();
}

void keys_callback(SDL_KeyboardEvent e)
{
  SDL_Keycode key = e.keysym.sym;
  Uint8 *state = SDL_GetKeyboardState(NULL);
  if (e.type != SDL_KEYDOWN) {
    return;
  }
  if (mode == M_NORMAL) {
    if (key >= '1' && key <= '9') {
      int n = key - '1';
      enabled_levels[n] = !enabled_levels[n];
      build_walls_array();
      build_map_array();
      build_path_array();
    }
  }
  /*Create unit*/
  if (key == SDLK_z) {
    Unit *u = unit_at(active_block_pos);
    if (!u) {
      add_unit(active_block_pos, 3);
    }
  }
  if (state[SDL_SCANCODE_H] && active_block_pos.x > 0) {
    active_block_pos.x--;
  } else if (state[SDL_SCANCODE_J] && active_block_pos.y < MAP_Y - 1) {
    active_block_pos.y++;
  } else if (state[SDL_SCANCODE_K] && active_block_pos.y > 0) {
    active_block_pos.y--;
  } else if (state[SDL_SCANCODE_L] && active_block_pos.x < MAP_X - 1) {
    active_block_pos.x++;
  } else if (key == SDLK_d && active_block_pos.z > 0) {
    active_block_pos.z--;
    build_picking_blocks_array();
  } else if (key == SDLK_u && active_block_pos.z < MAP_Z - 1) {
    active_block_pos.z++;
    build_picking_blocks_array();
  } else if (key == SDLK_m) {
    mode++;
    if (mode == M_COUNT) {
      mode = M_NORMAL;
    }
  } else if (key == SDLK_x) {
    fill_map(active_block_pos, 1);
    build_path_array();
  } else if (key == SDLK_s) {
    map_to_file("out.map");
  } else if (key == 'Z') {
    map_from_file("out.map");
    build_map_array();
  }
  {
    V3i p = active_block_pos; /*shortcut*/
    Block3 *b = block(p);
    if (key == SDLK_EQUALS && b) {
      b->h++;
      build_map_array();
    } else if (key == SDLK_MINUS && b) {
      b->h--;
      build_map_array();
    } else if (key == SDLK_t) {
      if (b) {
        free(b);
        map[p.z][p.y][p.x] = NULL;
      } else {
        b = ALLOCATE(1, Block3);
        b->t = B_FLOOR;
        b->h = 0;
        b->parent = D_NONE;
        map[p.z][p.y][p.x] = b;
        calc_map_clearence(3);
      }
      build_map_array();
    }
  }
  if (mode == M_SET_WALLS) {
    Block3 *b = block(active_block_pos);
    if (b) {
      ushort *w = b->walls;
      if (key == '1') {
        w[0] = (w[0]) ? 0 : 1;
      }
      if (key == '2') {
        w[1] = (w[1]) ? 0 : 1;
      }
      if (key == '3') {
        w[2] = (w[2]) ? 0 : 1;
      }
      if (key == '4') {
        w[3] = (w[3]) ? 0 : 1;
      }
      build_walls_array();
    }
  } else if (mode == M_SET_HEIGHTS) {
    Block3 *b = block(active_block_pos);
    if (b) {
      ushort *h = b->heights;
      if (key == '1') {
        h[1]++;
        if (h[1] > BLOCK_HEIGHT) {
          h[1] = 0;
        }
      }
      if (key == '2') {
        h[2]++;
        if (h[2] > BLOCK_HEIGHT) {
          h[2] = 0;
        }
      }
      if (key == '3') {
        h[3]++;
        if (h[3] > BLOCK_HEIGHT) {
          h[3] = 0;
        }
      }
      if (key == '4') {
        h[0]++;
        if (h[0] > BLOCK_HEIGHT) {
          h[0] = 0;
        }
      }
      build_map_array();
      calc_block_height(b);
    }
  }
}

void keys(void)
{
  Uint8 *state = SDL_GetKeyboardState(NULL);
  if (state[SDL_SCANCODE_ESCAPE] || state[SDL_SCANCODE_Q]) {
    done = true;
  } else if (state[SDL_SCANCODE_LCTRL]) {
    if (state[SDL_SCANCODE_UP]) {
      map_pos.x -= (float)sin(deg2rad(rotate_z)) * (zoom / 40.0f);
      map_pos.y -= (float)cos(deg2rad(rotate_z)) * (zoom / 40.0f);
    } else if (state[SDL_SCANCODE_DOWN]) {
      map_pos.x += (float)sin(deg2rad(rotate_z)) * (zoom / 40.0f);
      map_pos.y += (float)cos(deg2rad(rotate_z)) * (zoom / 40.0f);
    }
    if (state[SDL_SCANCODE_LEFT]) {
      map_pos.x -= (float)sin(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
      map_pos.y -= (float)cos(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
    } else if (state[SDL_SCANCODE_RIGHT]) {
      map_pos.x += (float)sin(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
      map_pos.y += (float)cos(deg2rad(rotate_z + 90)) * (zoom / 40.0f);
    }
  } else if (state[SDL_SCANCODE_LALT]) {
    if (state[SDL_SCANCODE_UP] && zoom > 5) {
      zoom -= 1;
    } else if (state[SDL_SCANCODE_DOWN] && zoom < 100) {
      zoom += 1;
    }
  } else {
    if (state[SDL_SCANCODE_LEFT]) {
      rotate_z -= rotations_per_tick;
      if (rotate_z < 0) {
        rotate_z += 360;
      }
    } else if (state[SDL_SCANCODE_RIGHT]) {
      rotate_z += rotations_per_tick;
      if (rotate_z > 360) {
        rotate_z -= 360;
      }
    }
    if (state[SDL_SCANCODE_UP] && rotate_x > 0) {
      rotate_x -= rotations_per_tick;
    } else if (state[SDL_SCANCODE_DOWN] && rotate_x < 100) {
      rotate_x += rotations_per_tick;
    }
  }
}

void print_world(void)
{
  V3f v;
  win2world(mouse_pos.x, mouse_pos.y, &v);
  printf("%3d:%3d [% 3.3f % 3.3f % 3.3f]\n",
      mouse_pos.x, mouse_pos.y, v.x, v.y, v.z);
}

static void mousemove(SDL_MouseMotionEvent e)
{
  if (SDL_GetMouseState(NULL, NULL) == SDL_BUTTON_RMASK) {
    rotate_z -= (float)e.xrel / 2.0f;
    rotate_x += (float)e.yrel / 2.0f;
  }
  mouse_pos.x = (int)e.x;
  mouse_pos.y = (int)e.y;
}

/*This is unreadble. TODO rewrite.*/
void events(void)
{
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_WINDOWEVENT:
      switch (e.window.event) {
      case SDL_WINDOWEVENT_CLOSE:
        if (win) {
          SDL_DestroyWindow(win);
        }
        break;
      case SDL_WINDOWEVENT_SHOWN:
        printf("WARNING: unhandled SDL_WINDOWEVENT_SHOWN\n");
        break;
      case SDL_WINDOWEVENT_HIDDEN:
        printf("WARNING: unhandled SDL_WINDOWEVENT_HIDDEN\n");
        break;
      case SDL_WINDOWEVENT_EXPOSED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_EXPOSED\n");
        break;
      case SDL_WINDOWEVENT_MOVED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_MOVED\n");
        break;
      case SDL_WINDOWEVENT_RESIZED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_RESIZED\n");
        break;
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_SIZE_CHANGED\n");
        break;
      case SDL_WINDOWEVENT_MINIMIZED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_MINIMIZED\n");
        break;
      case SDL_WINDOWEVENT_MAXIMIZED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_MAXIMIZED\n");
        break;
      case SDL_WINDOWEVENT_RESTORED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_RESTORED\n");
        break;
      case SDL_WINDOWEVENT_ENTER:
        printf("WARNING: unhandled SDL_WINDOWEVENT_ENTER\n");
        break;
      case SDL_WINDOWEVENT_LEAVE:
        printf("WARNING: unhandled SDL_WINDOWEVENT_LEAVE\n");
        break;
      case SDL_WINDOWEVENT_FOCUS_GAINED:
        printf("WARNING: unhandled SDL_WINDOWEVENT_FOCUS_GAINED\n");
        break;
      case SDL_WINDOWEVENT_FOCUS_LOST:
        printf("WARNING: unhandled SDL_WINDOWEVENT_FOCUS_LOST\n");
        break;
      default:
        printf("WARNING: events(): default case for "
            "window.event %d\n", e.window.type);
        break;
      }
      break;
    case SDL_KEYDOWN:
      keys_callback(e.key);
      break;
    case SDL_KEYUP:
      printf("WARNING: Unhandled SDL_KEYUP\n"); /*TODO*/
      break;
    case SDL_MOUSEMOTION:
      mousemove(e.motion);
      break;
    case SDL_MOUSEBUTTONDOWN: {
      /*TODO*/
      V2i pos = mk_v2i((int)e.button.x, (int)e.button.y);
      Button *button = v2i_to_button(pos);
      if (button) {
        printf("BUTTON ID:%d\n", button->id);
        if (button->callback) {
          button->callback();
        }
      } else if (unit_mode == UM_NORMAL
          && SDL_GetMouseState(NULL, NULL) == SDL_BUTTON_LMASK) {
        Unit *u = unit_at(active_block_pos);
        Block3 *b = block(active_block_pos);
        if (!u && selected_unit && b && b->parent != D_NONE) {
          fill_map(selected_unit->p, selected_unit->size);
          move_path = get_path(active_block_pos);
          unit_mode = UM_MOVING;
          current_move_index = 0;
          last_move_index = (move_path.count - 1) * MOVE_SPEED;
          free(va_path.v);
          va_path.v = NULL;
          va_path.count = 0;
        } else if (u) {
          select_unit(u);
          build_path_array();
        }
      }
      break;
    }
    case SDL_MOUSEBUTTONUP:
      /*TODO*/
      printf("WARNING: Unhandled SDL_MOUSEBUTTONUP\n");
      break;
    case SDL_MOUSEWHEEL:
      zoom -= (float)e.wheel.y;
      break;
    case SDL_SYSWMEVENT:
      printf("WARNING: Unhandled SDL_SYSWMEVENT \n");
      break;
    case SDL_TEXTEDITING:
      printf("WARNING: Unhandled SDL_TEXTEDITING\n");
      break;
    case SDL_TEXTINPUT:
      printf("WARNING: Unhandled SDL_TEXTINPUT\n");
      break;
    case SDL_FINGERDOWN:
      printf("WARNING: Unhandled SDL_FINGERDOWN\n");
      break;
    case SDL_FINGERUP:
      printf("WARNING: Unhandled SDL_FINGERUP\n");
      break;
    case SDL_FINGERMOTION:
      printf("WARNING: Unhandled SDL_FINGERMOTION\n");
      break;
    case SDL_TOUCHBUTTONDOWN:
      printf("WARNING: Unhandled SDL_TOUCHBUTTONDOWN\n");
      break;
    case SDL_TOUCHBUTTONUP:
      printf("WARNING: Unhandled SDL_TOUCHBUTTONUP\n");
      break;
    case SDL_DOLLARGESTURE:
      printf("WARNING: Unhandled SDL_DOLLARGESTURE\n");
      break;
    case SDL_DOLLARRECORD:
      printf("WARNING: Unhandled SDL_DOLLARRECORD\n");
      break;
    case SDL_MULTIGESTURE:
      printf("WARNING: Unhandled SDL_MULTIGESTURE\n");
      break;
    case SDL_CLIPBOARDUPDATE:
      printf("WARNING: Unhandled SDL_CLIPBOARDUPDATE\n");
      break;
    case SDL_DROPFILE:
      printf("WARNING: Unhandled SDL_DROPFILE\n");
      break;
    default:
      printf("WARNING: events(): "
          "default case for type %d\n", e.type);
      break;
    }
  }
}

void draw_for_picking(void)
{
  glLoadIdentity();
  glPushMatrix();
  set_camera();
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  {
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, va_pick.ub_c);
    glVertexPointer(3, GL_FLOAT, 0, va_pick.v);
    glDrawArrays(GL_QUADS, 0, va_pick.count);
  }
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();
}

void main_loop(void)
{
  while (!done) {
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
    draw_buttons();
    SDL_GL_SwapWindow(win);
    /* print_world(); */
    SDL_Delay(10);
  }
}

void init(void)
{
  float aspect_ratio = (float)window_size.y / (float)window_size.x;
  SDL_VideoInit(NULL);
  win = SDL_CreateWindow("SDL TEST",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      window_size.x, window_size.y, SDL_WINDOW_OPENGL);
  context = SDL_GL_CreateContext(win);
  IMG_Init(0);
  TTF_Init();
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
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
  glEnable(GL_LIGHT1);
}

void md5_init(void)
{
  md5_load_model(&m, "data/guard/bob.md5mesh");
  md5_load_anim("data/guard/bob.md5anim", &anim);
  md5_model_compute(&m, anim.joints);
#if 0
  md5_model_debug_print(&m);
  md5_anim_debug_print(&anim);
#endif
  md5_set_frame(&m, &anim, 0);
}

void obj_init(void)
{
  obj_read(&obj_m, "data/obj_test/model.obj");
  if (!load_texture("data/obj_test/test.tga", &obj_tex)) {
    die("blah blah blah");
  }
}

void build_obj(Obj_model *model)
{
  int i, j;
  va_obj.count = model->f_count * 3;
  va_obj.v = ALLOCATE(va_obj.count * 3, float);
  va_obj.t = ALLOCATE(va_obj.count * 2, float);
  for (i = 0; i < model->f_count; i++) {
    Obj_triangle *tri = model->faces + i;
    for (j = 0; j < 3; j++) {
      int vert_id = tri->v[j] - 1;
      int tex_id = tri->t[j] - 1;
      V3f *vert = model->vertexes + vert_id;
      V2f *tex = model->text_coords + tex_id;
      set_xyz(va_obj.v, 3, i, j, vert->x, vert->y, vert->z);
      set_xy(va_obj.t, 2, i, j, tex->x, tex->y);
    }
  }
}

void build_map_array(void)
{
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  va_map.count = get_blocks_count() * 4;
  if (va_map.v) {
    free(va_map.v);
  }
  if (va_map.t) {
    free(va_map.t);
  }
  va_map.v = ALLOCATE(va_map.count * 3, float);
  va_map.t = ALLOCATE(va_map.count * 2, float);
  while (is_able_to_inc_v3i(&p)) {
    Block3 *b = block(p);
    if (b && enabled_levels[p.z]) {
      ushort *h = b->heights;
      float n = BLOCK_SIZE / 2.0;
      float n2 = BLOCK_SIZE_2 / BLOCK_HEIGHT;
      float z = BLOCK_SIZE_2 * (float)p.z;
      V3f pos = v3i_to_v3f(p);
      set_xyz(va_map.v, 4, i, 0, pos.x - n, pos.y - n, z + (float)h[0] * n2);
      set_xyz(va_map.v, 4, i, 1, pos.x + n, pos.y - n, z + (float)h[1] * n2);
      set_xyz(va_map.v, 4, i, 2, pos.x + n, pos.y + n, z + (float)h[2] * n2);
      set_xyz(va_map.v, 4, i, 3, pos.x - n, pos.y + n, z + (float)h[3] * n2);
      set_xy(va_map.t, 4, i, 0, 0.0f, 0.0f);
      set_xy(va_map.t, 4, i, 1, 1.0f, 0.0f);
      set_xy(va_map.t, 4, i, 2, 1.0f, 1.0f);
      set_xy(va_map.t, 4, i, 3, 0.0f, 1.0f);
      i++;
    }
    inc_v3i(&p);
  }
}

void build_path_array(void)
{
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  va_path.count = get_path_lines_count() * 2;
  if (va_path.count == 0) {
    return;
  }
  assert(va_path.count > 0);
  if (va_path.v) {
    free(va_path.v);
  }
  va_path.v = ALLOCATE(va_path.count * 3, float);
  while (is_able_to_inc_v3i(&p)) {
    Block3 *b = block(p);
    if (b && enabled_levels[p.z] && b->parent != D_NONE) {
      V3f pos1 = v3i_to_v3f(p);
      V3f pos2 = v3i_to_v3f(neib(p, b->parent));
      set_xyz(va_path.v, 2, i, 0, pos1.x, pos1.y, pos1.z + 0.1f);
      set_xyz(va_path.v, 2, i, 1, pos2.x, pos2.y, pos2.z + 0.1f);
      i++;
    }
    inc_v3i(&p);
  }
}

void build_walls_array(void)
{
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  va_walls.count = get_walls_count() * 4;
  if (va_walls.count == 0) {
    return;
  }
  assert(va_walls.count > 0);
  if (va_walls.v) {
    free(va_walls.v);
  }
  if (va_walls.t) {
    free(va_walls.t);
  }
  va_walls.v = ALLOCATE(va_walls.count * 3, float);
  va_walls.t = ALLOCATE(va_walls.count * 2, float);
  while (is_able_to_inc_v3i(&p)) {
    Block3 *b = block(p);
    if (b && enabled_levels[p.z]) {
      float n = BLOCK_SIZE / 2.0;
      V3f pos = v3i_to_v3f(p);
      if (b->walls[0]) {
        set_xyz(va_walls.v, 4, i, 0, pos.x + n, pos.y - n, pos.z);
        set_xyz(va_walls.v, 4, i, 1, pos.x + n, pos.y + n, pos.z);
        set_xyz(va_walls.v, 4, i, 2, pos.x + n, pos.y + n, pos.z + 4 * n);
        set_xyz(va_walls.v, 4, i, 3, pos.x + n, pos.y - n, pos.z + 4 * n);
        set_xy(va_walls.t, 4, i, 0, 0.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 1, 1.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 2, 1.0f, 1.0f);
        set_xy(va_walls.t, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
      if (b->walls[1]) {
        set_xyz(va_walls.v, 4, i, 0, pos.x - n, pos.y + n, pos.z);
        set_xyz(va_walls.v, 4, i, 1, pos.x + n, pos.y + n, pos.z);
        set_xyz(va_walls.v, 4, i, 2, pos.x + n, pos.y + n, pos.z + 4 * n);
        set_xyz(va_walls.v, 4, i, 3, pos.x - n, pos.y + n, pos.z + 4 * n);
        set_xy(va_walls.t, 4, i, 0, 0.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 1, 1.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 2, 1.0f, 1.0f);
        set_xy(va_walls.t, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
      if (b->walls[2]) {
        set_xyz(va_walls.v, 4, i, 0, pos.x - n, pos.y - n, pos.z);
        set_xyz(va_walls.v, 4, i, 1, pos.x - n, pos.y + n, pos.z);
        set_xyz(va_walls.v, 4, i, 2, pos.x - n, pos.y + n, pos.z + 4 * n);
        set_xyz(va_walls.v, 4, i, 3, pos.x - n, pos.y - n, pos.z + 4 * n);
        set_xy(va_walls.t, 4, i, 0, 0.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 1, 1.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 2, 1.0f, 1.0f);
        set_xy(va_walls.t, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
      if (b->walls[3]) {
        set_xyz(va_walls.v, 4, i, 0, pos.x - n, pos.y - n, pos.z);
        set_xyz(va_walls.v, 4, i, 1, pos.x + n, pos.y - n, pos.z);
        set_xyz(va_walls.v, 4, i, 2, pos.x + n, pos.y - n, pos.z + 4 * n);
        set_xyz(va_walls.v, 4, i, 3, pos.x - n, pos.y - n, pos.z + 4 * n);
        set_xy(va_walls.t, 4, i, 0, 0.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 1, 1.0f, 0.0f);
        set_xy(va_walls.t, 4, i, 2, 1.0f, 1.0f);
        set_xy(va_walls.t, 4, i, 3, 0.0f, 1.0f);
        i++;
      }
    }
    inc_v3i(&p);
  }
}

void build_picking_blocks_array(void)
{
  V3i p = {0, 0, 0};
  int i = 0; /*block index*/
  va_pick.count = MAP_X * MAP_Y * 4;
  if (va_pick.v) {
    free(va_pick.v);
  }
  if (va_pick.ub_c) {
    free(va_pick.ub_c);
  }
  va_pick.v = ALLOCATE(va_pick.count * 3, float);
  va_pick.ub_c = ALLOCATE(va_pick.count * 3, GLubyte);
  for (p.y = 0; p.y < MAP_Y; p.y++) {
    for (p.x = 0; p.x < MAP_X; p.x++) {
      float n = BLOCK_SIZE / 2.0;
      V3f pos;
      pos.x = BLOCK_SIZE * (float)p.x;
      pos.y = BLOCK_SIZE * (float)p.y;
      pos.z = BLOCK_SIZE_2 * (float)active_block_pos.z;
      set_xyz(va_pick.v, 4, i, 0, pos.x - n, pos.y - n, pos.z);
      set_xyz(va_pick.v, 4, i, 1, pos.x + n, pos.y - n, pos.z);
      set_xyz(va_pick.v, 4, i, 2, pos.x + n, pos.y + n, pos.z);
      set_xyz(va_pick.v, 4, i, 3, pos.x - n, pos.y + n, pos.z);
      set_rgb(va_pick.ub_c, 4, i, 0, (GLubyte)p.x, (GLubyte)p.y, 1);
      set_rgb(va_pick.ub_c, 4, i, 1, (GLubyte)p.x, (GLubyte)p.y, 1);
      set_rgb(va_pick.ub_c, 4, i, 2, (GLubyte)p.x, (GLubyte)p.y, 1);
      set_rgb(va_pick.ub_c, 4, i, 3, (GLubyte)p.x, (GLubyte)p.y, 1);
      i++;
    }
  }
}

bool pick_block(V3i *p)
{
  GLint viewport[4];
  GLubyte pixel[3];
  assert(p);
  glGetIntegerv(GL_VIEWPORT, viewport);
  viewport[3] -= 1;
  glReadPixels(mouse_pos.x, viewport[3] - mouse_pos.y,
      1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void *)pixel);
  if (pixel[2] == 0) {
    return false;
  }
  p->x = pixel[0];
  p->y = pixel[1];
  return true;
}

void test_callback_1(void)
{
  puts("TEST CALLBACK 1");
}

void test_callback_2(void)
{
  puts("TEST CALLBACK 2");
}

int main(void)
{
  init();
  map_init();
  map_from_file("out.map");
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
  font = open_font("data/fonts/DejaVuSansMono.ttf", 12);
  add_button(font, 0, mk_v2i(0, 0), "TEST_BUTTON_1",
      test_callback_1);
  add_button(font, 1, mk_v2i(0, 30), "test_button_2",
      test_callback_2);
  main_loop();
  shut_down(0);
  return 0;
}
