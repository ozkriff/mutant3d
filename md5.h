typedef struct { int index[3]; } Triangle;

typedef struct Vertex{
  Vec2 tex;
  int weight_index;
  int weight_count;
} Vertex;

typedef struct {
  int joint_index;
  float weight;
  Vec3 pos;
} Weight;

typedef struct{
  char *shader;
  int num_vertices;
  int num_tris;
  int num_weights;
  Vertex *vertices;
  Triangle *tris;
  Weight *weights;
  Vec3 *points;
  int max_joints_per_vert;
  GLuint texture;
} Mesh;

typedef struct Joint Joint;
struct Joint {
  char *name;
  int parent_index;
  Joint *parent;
  Vec3 pos;
  Quat orient;
};

typedef struct {
  int num_joints;
  int num_meshes;
  Joint *joints;
  Mesh  *meshes;
} Model;

typedef struct{
  char *name;
  int parent;
  int flags;
  int start_index;
} Hierarchy_item;

typedef struct {
  Vec3 pos;
  Quat orient;
} Base_frame_joint;

typedef struct {
  Mesh *mesh;
  Hierarchy_item *hierarchy;
  Base_frame_joint *base_frame;
  float **frames;
  Joint *joints;
  int num_joints;
  int num_frames;
  int num_animated_components;
  int frame;
} Anim;
 
void anim_debug_print (Anim *a);
void mesh_debug_print (Mesh *m);
Vec3 joint_transform (Joint *j, Vec3 v);
void set_frame (Model *m, Anim *a, int n);
void model_draw (Model *m);
void load_anim (char *filename, Anim *a);
void load_model (Model *m, char *filename);
void model_compute (Model *m, Joint *joints);
void model_debug_print (Model *m);
