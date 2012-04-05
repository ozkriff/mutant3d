/*See LICENSE file for copyright and license details.*/

#include "GL/glfw.h"
#include <assert.h>
#include "bool.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"

bool load_texture(char *filename, GLuint *id){
  glGenTextures(1, id);
  glBindTexture(GL_TEXTURE_2D, *id);
  if(glfwLoadTexture2D(filename, GLFW_BUILD_MIPMAPS_BIT)){
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);
    return true;
  }else{
    die("gl.c: load_texture(): Can't load file '%s'\n",
        filename);
    return false;
  }
}

/*Translate window coords to 3d world coords.*/
void win2world(int x, int y, V3f *p){
  GLint viewport[4];
  GLdouble projection[16];
  GLdouble modelview[16];
  GLfloat vx, vy, vz;
  GLdouble wx, wy, wz;
  assert(p);
  assert(x >= 0 && y >= 0); /*TODO <= SCR_WIDTH*/
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
