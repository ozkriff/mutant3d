typedef struct { int index[3]; } Md5_triangle;

typedef struct{
  Vec2 tex;
  int weight_index;
  int weight_count;
} Md5_vertex;

typedef struct{
  int joint_index;
  float weight;
  Vec3 pos;
} Md5_weight;

typedef struct{
  char *shader;
  int num_vertices;
  int num_tris;
  int num_weights;
  Md5_vertex *vertices;
  Md5_triangle *tris;
  Md5_weight *weights;
  Vec3 *points;
  int max_joints_per_vert;
  GLuint texture;
} Md5_mesh;

typedef struct Md5_joint Md5_joint;
struct Md5_joint{
  char *name;
  int parent_index;
  Md5_joint *parent;
  Vec3 pos;
  Quat orient;
};

typedef struct{
  int num_joints;
  int num_meshes;
  Md5_joint *joints;
  Md5_mesh  *meshes;
} Md5_model;

typedef struct{
  char *name;
  int parent;
  int flags;
  int start_index;
} Md5_hierarchy_item;

typedef struct{
  Vec3 pos;
  Quat orient;
} Md5_base_frame_joint;

typedef struct{
  Md5_mesh *mesh;
  Md5_hierarchy_item *hierarchy;
  Md5_base_frame_joint *base_frame;
  float **frames;
  Md5_joint *joints;
  int num_joints;
  int num_frames;
  int num_animated_components;
  int frame;
} Md5_anim;
 
void md5_anim_debug_print (Md5_anim *a);
void md5_mesh_debug_print (Md5_mesh *m);
Vec3 md5_joint_transform (Md5_joint *j, Vec3 v);
void md5_set_frame (Md5_model *m, Md5_anim *a, int n);
void md5_model_draw (Md5_model *m);
void md5_load_anim (char *filename, Md5_anim *a);
void md5_load_model (Md5_model *m, char *filename);
void md5_model_compute (Md5_model *m, Md5_joint *joints);
void md5_model_debug_print (Md5_model *m);
