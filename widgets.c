#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include "bool.h"
#include "list.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"
#include "widgets.h"

TTF_Font *open_font(char *font_name, int size){
  TTF_Font *f = TTF_OpenFont(font_name, size);
  if(!f)
    die("Unable to open font file: '%s'\n", TTF_GetError());
  return f;
}
