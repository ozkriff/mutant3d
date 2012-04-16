/*See LICENSE file for copyright and license details.*/

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include "bool.h"
#include "list.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"
#include "gl.h"
#include "widgets.h"

static List buttons = {NULL, NULL, 0}; /*Button*/

GLuint ttf_gl_print(TTF_Font *f, char* text, V2i *size){
  SDL_Color Color = {255, 255, 255, 0};
  SDL_Surface *s = TTF_RenderUTF8_Blended(f, text, Color);
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameterf(GL_TEXTURE_2D,
      GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,
      GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h,
      0, GL_BGRA, GL_UNSIGNED_BYTE, s->pixels);
  size->x = s->w;
  size->y = s->h;
  SDL_FreeSurface(s);
  return id;
}

void change_button_text(Button* b, char* text){
  glDeleteTextures(1, &(b->texture_id));
  b->texture_id = ttf_gl_print(b->f, text, &b->size);
}

void change_button_text_by_id (int id, char* text){
  Node *node;
  FOR_EACH_NODE(buttons, node){
    Button *b = node->data;
    if(b->id == id){
      change_button_text(b, text);
      return;
    }
  }
}

void add_button(TTF_Font *f, int id, V2i pos, char *text, void (*callback)(void)){
  Button *b = ALLOCATE(1, Button);
  b->f = f;
  b->id = id;
  b->pos = pos;
  b->text = text;
  b->texture_id = ttf_gl_print(f, text, &b->size);
  b->callback = callback;
  push_node(&buttons, mk_node(b));
}

Button *v2i_to_button(V2i pos){
  Node *node;
  FOR_EACH_NODE(buttons, node){
    Button *b = node->data;
    if(pos.x >= b->pos.x
    && pos.y >= b->pos.y
    && pos.x <= b->pos.x + b->size.x
    && pos.y <= b->pos.y + b->size.y){
      return b;
    }
  }
  return NULL;
}

void draw_button(Button *b){
  float rect[4 * 2];
  float texture_coord[4 * 2];
  set_xy(rect, 4, 0, 0, 0, (float)b->size.y);
  set_xy(rect, 4, 0, 1, (float)b->size.x, (float)b->size.y);
  set_xy(rect, 4, 0, 2, (float)b->size.x, 0);
  set_xy(rect, 4, 0, 3, 0, 0);
  set_xy(texture_coord, 4, 0, 0, 0, 1);
  set_xy(texture_coord, 4, 0, 1, 1, 1);
  set_xy(texture_coord, 4, 0, 2, 1, 0);
  set_xy(texture_coord, 4, 0, 3, 0, 0);

  glPushMatrix();

  glTranslatef((float)b->pos.x, (float)b->pos.y, 0.0f);

  glColor4f(0.3f, 0.3f, 0.3f, 0.6f);
  glVertexPointer(2, GL_FLOAT, 0, rect);
  glDrawArrays(GL_QUADS, 0, 4);

  glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, b->texture_id);
  glVertexPointer(2, GL_FLOAT, 0, rect);
  glTexCoordPointer(2, GL_FLOAT, 0, texture_coord);
  glDrawArrays(GL_QUADS, 0, 4);
  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
}

/*TODO split*/
void draw_buttons(void){
  Node *node;
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, window_size.x, window_size.y, 0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  FOR_EACH_NODE(buttons, node){
    Button *b = node->data;
    draw_button(b);
  }
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}

TTF_Font *open_font(char *font_name, int size){
  TTF_Font *f = TTF_OpenFont(font_name, size);
  if(!f)
    die("Unable to open font file: '%s'\n", TTF_GetError());
  return f;
}
