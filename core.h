/*See LICENSE file for copyright and license details.*/

typedef unsigned short int ushort;

typedef enum {
  B_EMPTY,
  B_FLOOR
} Block_type_id;

/*forward right backward left up down*/
typedef enum {
  D_F, D_FR, D_R, D_BR, D_B, D_BL, D_L, D_FL,
  D_UF, D_UFR, D_UR, D_UBR, D_UB, D_UBL, D_UL, D_UFL,
  D_DF, D_DFR, D_DR, D_DBR, D_DB, D_DBL, D_DL, D_DFL,
  D_NONE, D_ERROR, D_COUNT
} Dir;

typedef struct {
  ushort t; /*Block_type_id*/
  ushort h; /*height*/
  ushort clearence;
  ushort walls[4];
  ushort heights[4];
  ushort parent; /*Dir*/
  int cost;
} Block3;

typedef enum {
  M_NORMAL,
  M_SET_WALLS,
  M_SET_HEIGHTS,
  M_COUNT
} Mode;

typedef enum {
  UM_NORMAL,
  UM_MOVING
} Unit_mode;

typedef struct {
  V3i p;
} Unit;

#define BLOCK_HEIGHT 4
#define MAP_X 100
#define MAP_Y 100
#define MAP_Z 6

#define MAX_HEIGHT_DIFF 1

#define MIN_CLEARENCE 1
#define ACTION_POINTS 30

extern Block3 *map[MAP_Z][MAP_Y][MAP_X];
extern V3i active_block_pos;
extern Unit_mode unit_mode;
extern List units;
extern Unit *selected_unit;
extern Mode mode;

Block3 *block(V3i p);
Block3 *block_2(int x, int y, int z);
bool inboard(V3i p);
bool check_height_diff(V3i p1, V3i p2, int max_diff);
Unit *unit_at(V3i p);
void add_unit(V3i p);
bool inboard(V3i p);
void map_to_file(const char *filename);
void calc_map_clearence(ushort max_size);
void calc_block_height(Block3 *b);
void map_from_file(const char *filename);
void select_unit(Unit *u);
void map_init(void);
int get_blocks_count(void);
int get_walls_count(void);
int get_path_lines_count(void);
