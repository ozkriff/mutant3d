/*See LICENSE file for copyright and license details.*/

#include <stdlib.h>
#include "bool.h"
#include "list.h"
#include "math.h"
#include "mutant3d.h"
#include "misc.h"
#include "path.h"

/*stack for filling map*/
static Stack stack = {NULL, NULL, 0};

/*Push this coordinates to stack,
  update cost and parent of this block*/
static void push(V3i m, V3i parent, int newcost) {
  Block3 *b = block(m);
  b->cost = newcost;
  b->parent = parent;
  push_node(&stack, COPY2HEAP(&m, V3i));
}

static V3i pop(void){
  Node *tmp = pop_node(&stack);
  V3i m = *(V3i*)(tmp->data);
  free(tmp->data);
  free(tmp);
  return(m);
}

static int get_tile_cost(V3i t, V3i nb){
  int dx = t.x - nb.x;
  int dy = t.y - nb.y;
  int dz = t.z - nb.z;
  int cost = 1;
  if(dx) cost++;
  if(dy) cost++;
  if(dz) cost++;
#if 0
  /*diagonals*/
  {
    if(dx && dy) cost += 500;
    /*if(dx == -1 && dy == +1) cost -= 500;*/
  }
#endif
  return cost;
}

/*проверить стенку этого блока.
  проверить что есть сосед в нужном направлении.
  если есть сосед, то
  проверить противоположную стенку соседа*/
static bool check_wall(V3i pos, int dir){
  V3i pos2;
  int rev_dir;
  Block3 *b1;
  Block3 *b2;
  rev_dir = dir + 2;
  if(rev_dir >= 4)
    rev_dir -= 4;
  if(dir == 0) pos2 = neib(pos, D_F);
  if(dir == 1) pos2 = neib(pos, D_R);
  if(dir == 2) pos2 = neib(pos, D_B);
  if(dir == 3) pos2 = neib(pos, D_L);
  b1 = block(pos);
  if(!inboard(pos2))
    return b1->walls[dir];
  b2 = block(pos2);
  if(!b2)
    return b1->walls[dir];
  return b1->walls[dir] || b2->walls[rev_dir];
}

/*TODO rename*/
/*p1 - orig_pos, p2 - neib pos*/
static void process_neibor(V3i p1, V3i p2){
  int newcost;
  Block3 *b1 = block(p1);
  Block3 *b2 = block(p2);
  if(!b2 || b2->t != B_FLOOR)
    return;
  {
    Dir d = m2dir(p1, p2);
    if(d == D_F  && (check_wall(p1, 0) || check_wall(p2, 2))) return;
    if(d == D_R  && (check_wall(p1, 1) || check_wall(p2, 3))) return;
    if(d == D_B  && (check_wall(p1, 2) || check_wall(p2, 0))) return;
    if(d == D_L  && (check_wall(p1, 3) || check_wall(p2, 1))) return;
    if(d == D_FR && (check_wall(p1, 0) || check_wall(p1, 1)
        || check_wall(p2, 2) || check_wall(p2, 3))) return;
    if(d == D_BR && (check_wall(p1, 1) || check_wall(p1, 2)
        || check_wall(p2, 0) || check_wall(p2, 3))) return;
    if(d == D_BL && (check_wall(p1, 2) || check_wall(p1, 3)
        || check_wall(p2, 0) || check_wall(p2, 1))) return;
    if(d == D_FL && (check_wall(p1, 3) || check_wall(p1, 0)
        || check_wall(p2, 2) || check_wall(p2, 1))) return;
  }
  if(!check_height_diff(p1, p2, MAX_HEIGHT_DIFF))
    return;
  if(b2->clearence < MIN_CLEARENCE)
    return;
  newcost = b1->cost + get_tile_cost(p1, p2);
  if(b2->cost > newcost && newcost <= ACTION_POINTS)
    push(p2, p1, newcost);
}

static void clean_map(void){
  V3i p = {0, 0, 0};
  while(is_able_to_inc_v3i(&p)){
    Block3 *b = block(p);
    if(b){
      b->cost = 30000;
      b->parent = mk_v3i(-1, -1, -1);
    }
    inc_v3i(&p);
  }
}

static void try_to_push_neibors(V3i m){
  unsigned int i;
  for(i = 0; i < 8 * 3; i++){
    V3i neib_m = neib(m, i);
    if(inboard(neib_m))
      process_neibor(m, neib_m);
  }
}

void fill_map(V3i pos){
  clean_map();
  push(pos, pos, 0); /*push start position*/
  while(stack.count > 0)
    try_to_push_neibors(pop());
  clear_list(&stack);
}

List get_path(V3i pos){
  List path = {NULL, NULL, 0};
  while(block(pos)->cost != 0){
    push_node(&path, COPY2HEAP(&pos, V3i));
    pos = block(pos)->parent;
  }
  /*Add start position.*/
  push_node(&path, COPY2HEAP(&pos, V3i));
  return(path);
}
