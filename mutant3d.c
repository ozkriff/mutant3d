#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glfw.h>
#include "bool.h"
#include "misc.h"
#include "math.h"
#include "md5.h"
#include "obj.h"
#include "gl.h"

float rotate_x = 0;
float rotate_y = 0;
float rotate_z = 0;
const float rotations_per_tick = 0.2f;

Md5_model m;
Md5_anim anim;

Obj_model obj_m;
GLuint obj_tex;

void shut_down (int return_code){
  glfwTerminate();
  exit(return_code);
}

void draw(void){
  glLoadIdentity();
  glTranslatef(0, 0, -100);
  glPushMatrix();
  glRotatef(-90, 1, 0, 0);
  glRotatef(rotate_y, 1, 0, 0);
  glRotatef(rotate_z, 0, 0, 1);
  glTranslatef(0, 0, -30);
  md5_set_frame(&m, &anim, anim.frame + 1);
  md5_model_draw(&m);
#if 0
  int i;
  for(i = 0; i < 0; i++){
    glRotatef(-11, 0, 0, 1);
    glTranslatef(40, 0, 0);
    model_draw(&m);
  }
#endif
  glPopMatrix();
  glRotatef(rotate_y, 1, 0, 0);
  glRotatef(rotate_z, 0, 1, 0);
  obj_draw(obj_tex, &obj_m);
}

void main_loop (void){
  double old_time = glfwGetTime();
  while(1) {
    double current_time = glfwGetTime();
    double delta_rotate = (current_time - old_time) * rotations_per_tick * 360;
    old_time = current_time;
    if(glfwGetKey(GLFW_KEY_ESC) || glfwGetKey('Q'))
      break;
    if(glfwGetKey(GLFW_KEY_LEFT )) rotate_z += delta_rotate;
    if(glfwGetKey(GLFW_KEY_RIGHT)) rotate_z -= delta_rotate;
    if(glfwGetKey(GLFW_KEY_UP   )) rotate_y += delta_rotate;
    if(glfwGetKey(GLFW_KEY_DOWN )) rotate_y -= delta_rotate;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw();
    glfwSwapBuffers();
    if(0){
      int winx, winy;
      Vec3 v;
      glfwGetMousePos(&winx, &winy);
      win2world(winx, winy, &v);
      printf("%d:%d [%f %f %f]\n", winx, winy, v.x, v.y, v.z);
    }
  }
}
 
void
init (void) {
  const int window_width = 500;
  const int window_height = 500;
  float aspect_ratio = ((float)window_height) / window_width;
  if(glfwInit() != GL_TRUE)
    shut_down(1);
  /*800 x 600, 16 bit color, no depth, alpha or stencil buffers, windowed*/
  if(glfwOpenWindow(window_width, window_height,
      5, 6, 5,
      0, 8, 0, GLFW_WINDOW) != GL_TRUE){
    shut_down(1);
  }
  glfwSetWindowTitle("The GLFW Window");
  /*Set the projection matrix to a normal frustum with a max depth of 50*/
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(.5, -.5, -.5 * aspect_ratio, .5 * aspect_ratio, 1, 500);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  /*glEnable(GL_CULL_FACE);*/
  glEnable(GL_AUTO_NORMAL);
  /*glShadeModel(GL_SMOOTH);*/
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void md5_init (void){
  md5_load_model(&m, "data/guard/bob.md5mesh");
  md5_load_anim("data/guard/bob.md5anim", &anim);
  md5_model_compute(&m, anim.joints);
#if 0
  md5_model_debug_print(&m);
  md5_anim_debug_print(&anim);
#endif
  md5_set_frame(&m, &anim, 0);
}

void obj_init (void){
  obj_read (&obj_m, "data/obj_test/model.obj");
  obj_tex = load_texture("data/obj_test/test.tga");
}

int main (void){
  init();
  md5_init();
  obj_init();
  main_loop();
  shut_down(0);
  return(0);
}
