#include "math.h"
#include "GL/glfw.h"

GLuint load_texture (char *filename){
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  if(glfwLoadTexture2D(filename, GLFW_BUILD_MIPMAPS_BIT)){ 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return(id);
  }else{
    /*TODO: log error. die.*/
    return(-1);
  }
}

/*Translate window coords to 3d world coords.*/
void win2world (int x, int y, Vec3 *p){
  GLint viewport[4];
  GLdouble projection[16];
  GLdouble modelview[16];
  GLfloat vx, vy, vz;
  GLdouble wx, wy, wz;
  glGetIntegerv(GL_VIEWPORT,viewport);
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
  vx = x;
  vy = viewport[3] - (float)y;
  glReadPixels(vx, (int)vy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &vz);
  gluUnProject(vx, vy, vz, modelview, projection, viewport,
      &wx, &wy, &wz);
  p->x = wx;
  p->y = wy;
  p->z = wz;
}
