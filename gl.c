/*See LICENSE file for copyright and license details.*/

#include <assert.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "bool.h"
#include "math.h"
#include "list.h"
#include "core.h"
#include "mutant3d.h"
#include "misc.h"
#include "gl.h"

bool load_texture(char *filename, GLuint *id)
{
  GLenum texture_format;
  GLint n_of_colors;
  SDL_Surface *surface;
  assert(id);
  surface = IMG_Load(filename);
  if (surface) {
    if ((surface->w & (surface->w - 1)) != 0) {
      die("image's width is not a power of 2\n");
    }
    if ((surface->h & (surface->h - 1)) != 0) {
      die("image's height is not a power of 2\n");
    }
    n_of_colors = surface->format->BytesPerPixel;
    if (n_of_colors == 4) {
      /* contains an alpha channel */
      if (surface->format->Rmask == 0x000000ff) {
        texture_format = GL_RGBA;
      } else {
        texture_format = GL_BGRA;
      }
    } else if (n_of_colors == 3) {
      /* no alpha channel */
      if (surface->format->Rmask == 0x000000ff) {
        texture_format = GL_RGB;
      } else {
        texture_format = GL_BGR;
      }
    } else {
      die("warning: the image is not truecolor..  this will probably break\n");
    }
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, n_of_colors, surface->w, surface->h, 0,
        texture_format, GL_UNSIGNED_BYTE, surface->pixels);
  } else {
    die("gl.c: load_texture(): Can't load file '%s'\n",
        filename);
    return false;
  }
  if (surface) {
    SDL_FreeSurface(surface);
  }
  return true;
}

/*Translate window coords to 3d world coords.*/
void win2world(int x, int y, V3f *p)
{
  GLint viewport[4];
  GLdouble projection[16];
  GLdouble modelview[16];
  GLfloat vx, vy, vz;
  GLdouble wx, wy, wz;
  assert(p);
  assert(x >= 0);
  assert(y >= 0);
  assert(x < window_size.x);
  assert(y < window_size.y);
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  vx = (float)x;
  vy = (float)viewport[3] - (float)y;
  glReadPixels((int)vx, (int)vy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &vz);
  gluUnProject(vx, vy, vz, modelview, projection, viewport,
      &wx, &wy, &wz);
  p->x = (float)wx;
  p->y = (float)wy;
  p->z = (float)wz;
}

void va_rotate(Va *va, Quat q)
{
  int i;
  assert(va);
  for (i = 0; i < va->count; i++) {
    V3f *original = (V3f *)(va->v + i * 3);
    V3f rotated = quat_rot(q, *original);
    *original = rotated;
  }
}

void set_xy(float *coords, int n, int i, int vi, float x, float y)
{
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
void set_xyz(float *verts, int n, int i, int vi, float x, float y, float z)
{
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
void set_rgb(GLubyte *colors, int n, int i, int vi, GLubyte r, GLubyte g, GLubyte b)
{
  GLubyte *color;
  assert(n == 2 || n == 3 || n == 4);
  assert(vi >= 0 && vi <= n);
  assert(i >= 0);
  color = colors + (i * n * 3) + (3 * vi);
  color[0] = r;
  color[1] = g;
  color[2] = b;
}
