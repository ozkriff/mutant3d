/*See LICENSE file for copyright and license details.*/

typedef enum {
  B_EMPTY,
  B_FLOOR
} Block_type_id;

#define BLOCK_HEIGHT 4

/*forward right backward left up down*/
typedef enum {
  D_F, D_FR, D_R, D_BR, D_B, D_BL, D_L, D_FL,
  D_UF, D_UFR, D_UR, D_UBR, D_UB, D_UBL, D_UL, D_UFL,
  D_DF, D_DFR, D_DR, D_DBR, D_DB, D_DBL, D_DL, D_DFL,
  D_NONE, D_ERROR, D_COUNT
} Dir;

typedef struct {
  Block_type_id t;
  int h; /*height*/
  int clearence;
  bool path;
  bool walls[4];
  int heights[4];
  Dir parent;
  int cost;
} Block3;

#define MAP_X 100
#define MAP_Y 100
#define MAP_Z 6

#define MAX_HEIGHT_DIFF 1

#define MIN_CLEARENCE 1
#define ACTION_POINTS 30

extern Block3 *map[MAP_Z][MAP_Y][MAP_X];

Block3 *block(V3i p);
Block3 *block_2(int x, int y, int z);
bool inboard(V3i p);
bool check_height_diff(V3i p1, V3i p2, int max_diff);
