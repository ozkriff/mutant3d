/*See LICENSE file for copyright and license details.*/

bool strcmp_sp(const char *s1, const char *s2);
char *my_strdup(const char *s);
void die(const char *errstr, ...);
void *copy2heap(void *data, int size);

Dir m2dir(V3i a, V3i b);
V3i neib(V3i pos, Dir i);
void fixnum(int min, int max, int *n);
int rnd(int min, int max);

#define COPY_TO_HEAP(data, type) \
  mk_node(copy2heap(data, sizeof(type)))

bool is_able_to_inc_v3i(V3i *pos);
void inc_v3i(V3i *pos);

void *my_alloc(int count, int size);
#define ALLOCATE(count, type) \
  my_alloc(count, sizeof(type))
