/*See LICENSE file for copyright and license details.*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "bool.h"
#include "math.h"
#include "list.h"
#include "core.h"
#include "mutant3d.h"
#include "misc.h"

void *my_alloc(int count, int size)
{
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
bool strcmp_sp(const char *s1, const char *s2)
{
  while (s2 && s1) {
    if (*s1 != *s2) {
      return false;
    }
    if (*s1 == ' ' || *s1 == '\t') {
      return true;
    }
    s1++, s2++;
  }
  return true;
}

char *my_strdup(const char *s)
{
  char *d;
  assert(strlen(s) + 1 < 10000);
  d = ALLOCATE((int)strlen(s) + 1, char);
  if (d) {
    strcpy(d, s);
  }
  return d;
}

void die(const char *errstr, ...)
{
  va_list l;
  va_start(l, errstr);
  vfprintf(stderr, errstr, l);
  va_end(l);
  exit(EXIT_FAILURE);
}

void *copy_to_heap(void *data, int size)
{
  void *tmp = my_alloc(1, size);
  memcpy(tmp, data, (unsigned int)size);
  return tmp;
}

Dir m2dir(V3i a, V3i b)
{
  int dx = b.x - a.x;
  int dy = b.y - a.y;
  int dz = b.z - a.z;
  if (dz == 0) {
    if (dx == +1 && dy == +0) {
      return D_F;
    } else if (dx == +1 && dy == +1) {
      return D_FR;
    } else if (dx == +0 && dy == +1) {
      return D_R;
    } else if (dx == -1 && dy == +1) {
      return D_BR;
    } else if (dx == -1 && dy == +0) {
      return D_B;
    } else if (dx == -1 && dy == -1) {
      return D_BL;
    } else if (dx == +0 && dy == -1) {
      return D_L;
    } else if (dx == +1 && dy == -1) {
      return D_FL;
    }
  } else if (dz == +1) {
    if (dx == +1 && dy == +0) {
      return D_UF;
    } else if (dx == +1 && dy == +1) {
      return D_UFR;
    } else if (dx == +0 && dy == +1) {
      return D_UR;
    } else if (dx == -1 && dy == +1) {
      return D_UBR;
    } else if (dx == -1 && dy == +0) {
      return D_UB;
    } else if (dx == -1 && dy == -1) {
      return D_UBL;
    } else if (dx == +0 && dy == -1) {
      return D_UL;
    } else if (dx == +1 && dy == -1) {
      return D_UFL;
    }
  } else if (dz == -1) {
    if (dx == +1 && dy == +0) {
      return D_DF;
    } else if (dx == +1 && dy == +1) {
      return D_DFR;
    } else if (dx == +0 && dy == +1) {
      return D_DR;
    } else if (dx == -1 && dy == +1) {
      return D_DBR;
    } else if (dx == -1 && dy == +0) {
      return D_DB;
    } else if (dx == -1 && dy == -1) {
      return D_DBL;
    } else if (dx == +0 && dy == -1) {
      return D_DL;
    } else if (dx == +1 && dy == -1) {
      return D_DFL;
    }
  }
  return D_ERROR;
}

/*Get tile's neiborhood by it's index.*/
V3i neib(V3i pos, Dir i)
{
  int dx, dy, dz;
  int directions[8][2] = {
    {1, 0},
    {1, 1},
    {0, 1},
    { -1, 1},
    { -1, 0},
    { -1, -1},
    {0, -1},
    {1, -1}
  };
  assert(i < D_COUNT);
  if (i == D_NONE || i == D_ERROR)
    die("misc: neib(): "
        "Wrong direction: pos:[%d %d %d] dir:%d\n",
        pos.x, pos.y, pos.z, i);
  dx = directions[i % 8][0];
  dy = directions[i % 8][1];
  if ((int)i / 8 == 0) {
    dz = 0;
  }
  if ((int)i / 8 == 1) {
    dz = 1;
  }
  if ((int)i / 8 == 2) {
    dz = -1;
  }
  return mk_v3i(pos.x + dx, pos.y + dy, pos.z + dz);
}

void fixnum(int min, int max, int *n)
{
  assert(n);
  if (*n < min) {
    *n = min;
  }
  if (*n > max) {
    *n = max;
  }
}

int rnd(int min, int max)
{
  assert(min < max);
  if (max != min) {
    return rand() % (max - min) + min;
  } else {
    return max;
  }
}

/*TODO rename*/
/*Check if this was last block in map.*/
bool is_able_to_inc_v3i(V3i *pos)
{
  bool is_x_correct, is_y_correct, is_z_correct;
  assert(pos);
  assert(pos->x >= 0 && pos->y >= 0 && pos->z >= 0);
  is_x_correct = pos->x != (MAP_X - 1);
  is_y_correct = pos->y != (MAP_Y - 1);
  is_z_correct = pos->z != (MAP_Z - 1);
  return is_x_correct || is_y_correct || is_z_correct;
}

/*TODO rename*/
void inc_v3i(V3i *pos)
{
  assert(is_able_to_inc_v3i(pos));
  pos->x++;
  if (pos->x == MAP_X) {
    pos->x = 0;
    pos->y++;
    if (pos->y == MAP_Y) {
      pos->y = 0;
      pos->z++;
    }
  }
}
