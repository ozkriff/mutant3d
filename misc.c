/*See LICENSE file for copyright and license details.*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "bool.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"

/*TODO maybu should add some checks later?*/
void *my_alloc(int count, int size){
  unsigned int u_count;
  unsigned int u_size;
  assert(count > 0 && size > 0);
  u_count = (unsigned int)count;
  u_size = (unsigned int)size;
  return calloc(u_count, u_size);
}

/*Compare strings till first space or tab.
  strcmp_sp("lo 2 3", "lo %d %d") -> true
  strcmp_sp("no 2 3", "lo %d %d") -> false */
bool strcmp_sp(const char *s1, const char *s2){
  while(s2 && s1){
    if(*s1 != *s2)
      return false;
    if(*s1==' ' || *s1=='\t')
      return true;
    s1++, s2++;
  }
  return true;
}

char *my_strdup(const char *s){
  char *d;
  assert(strlen(s) + 1 < 10000);
  d = ALLOCATE((int)strlen(s) + 1, char);
  if(d)
    strcpy(d, s);
  return d;
}

void die(const char *errstr, ...){
  va_list l;
  va_start(l, errstr);
  vfprintf(stderr, errstr, l);
  va_end(l);
  exit(EXIT_FAILURE);
}

/*TODO: rename me*/
void*
copy2heap(void *data, int size){
  void *tmp = my_alloc(1, size);
  memcpy(tmp, data, (unsigned int)size);
  return tmp;
}

Dir m2dir(V3i a, V3i b){
  int dx = b.x - a.x;
  int dy = b.y - a.y;
  int dz = b.z - a.z;
  if(dz == 0){
    if(dx == +1 && dy == +0) return D_F;
    else if(dx == +1 && dy == +1) return D_FR;
    else if(dx == +0 && dy == +1) return D_R;
    else if(dx == -1 && dy == +1) return D_BR;
    else if(dx == -1 && dy == +0) return D_B;
    else if(dx == -1 && dy == -1) return D_BL;
    else if(dx == +0 && dy == -1) return D_L;
    else if(dx == +1 && dy == -1) return D_FL;
  }else if(dz == +1){
    if(dx == +1 && dy == +0) return D_UF;
    else if(dx == +1 && dy == +1) return D_UFR;
    else if(dx == +0 && dy == +1) return D_UR;
    else if(dx == -1 && dy == +1) return D_UBR;
    else if(dx == -1 && dy == +0) return D_UB;
    else if(dx == -1 && dy == -1) return D_UBL;
    else if(dx == +0 && dy == -1) return D_UL;
    else if(dx == +1 && dy == -1) return D_UFL;
  }else if(dz == -1){
    if(dx == +1 && dy == +0) return D_DF;
    else if(dx == +1 && dy == +1) return D_DFR;
    else if(dx == +0 && dy == +1) return D_DR;
    else if(dx == -1 && dy == +1) return D_DBR;
    else if(dx == -1 && dy == +0) return D_DB;
    else if(dx == -1 && dy == -1) return D_DBL;
    else if(dx == +0 && dy == -1) return D_DL;
    else if(dx == +1 && dy == -1) return D_DFL;
  }
  return D_ERROR;
}

/*Get tile's neiborhood by it's index.*/
V3i neib(V3i pos, Dir i){
  int dx = 0;
  int dy = 0;
  int dz = 0;
  switch(i){
    case D_F:   dx = +1; dy = +0; dz = +0; break;
    case D_FR:  dx = +1; dy = +1; dz = +0; break;
    case D_R:   dx = +0; dy = +1; dz = +0; break;
    case D_BR:  dx = -1; dy = +1; dz = +0; break;
    case D_B:   dx = -1; dy = +0; dz = +0; break;
    case D_BL:  dx = -1; dy = -1; dz = +0; break;
    case D_L:   dx = +0; dy = -1; dz = +0; break;
    case D_FL:  dx = +1; dy = -1; dz = +0; break;
    /*UP*/
    case D_UF:  dx = +1; dy = +0; dz = +1; break;
    case D_UFR: dx = +1; dy = +1; dz = +1; break;
    case D_UR:  dx = +0; dy = +1; dz = +1; break;
    case D_UBR: dx = -1; dy = +1; dz = +1; break;
    case D_UB:  dx = -1; dy = +0; dz = +1; break;
    case D_UBL: dx = -1; dy = -1; dz = +1; break;
    case D_UL:  dx = +0; dy = -1; dz = +1; break;
    case D_UFL: dx = +1; dy = -1; dz = +1; break;
    /*DOWN*/
    case D_DF:  dx = +1; dy = +0; dz = -1; break;
    case D_DFR: dx = +1; dy = +1; dz = -1; break;
    case D_DR:  dx = +0; dy = +1; dz = -1; break;
    case D_DBR: dx = -1; dy = +1; dz = -1; break;
    case D_DB:  dx = -1; dy = +0; dz = -1; break;
    case D_DBL: dx = -1; dy = -1; dz = -1; break;
    case D_DL:  dx = +0; dy = -1; dz = -1; break;
    case D_DFL: dx = +1; dy = -1; dz = -1; break;
    case D_ERROR:
    default:
      die("bla-bla-bla"); break;
  }
  return mk_v3i(pos.x + dx, pos.y + dy, pos.z + dz);
}

void fixnum(int min, int max, int *n){
  assert(n);
  if(*n < min)
    *n = min;
  if(*n > max)
    *n = max;
}

int rnd(int min, int max){
  assert(min < max);
  if(max != min)
    return rand() % (max - min) + min;
  else
    return max;
}

/*TODO rename*/
/*проверить, что остались блоки*/
bool is_able_to_inc_v3i(V3i *pos){
  bool is_x_correct, is_y_correct, is_z_correct;
  assert(pos);
  assert(pos->x >= 0 && pos->y >= 0 && pos->z >= 0);
  is_x_correct = pos->x != (MAP_X - 1);
  is_y_correct = pos->y != (MAP_Y - 1);
  is_z_correct = pos->z != (MAP_Z - 1);
  return is_x_correct || is_y_correct || is_z_correct;
}

/*TODO rename*/
void inc_v3i(V3i *pos){
  assert(is_able_to_inc_v3i(pos));
  pos->x++;
  if(pos->x == MAP_X){
    pos->x = 0;
    pos->y++;
    if(pos->y == MAP_Y){
      pos->y = 0;
      pos->z++;
    }
  }
}
