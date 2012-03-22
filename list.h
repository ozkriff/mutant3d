/*See LICENSE file for copyright and license details.*/

/*Double-linked list, stack, queue.*/

typedef struct Node Node;
struct Node {
  Node *next;
  Node *prev;
  void *data;
};

typedef struct List List;
struct List {
  Node *head; /*pointer to first node*/
  Node *tail; /*pointer to last node*/
  int count; /*number of nodes in list*/
};

void  insert_node(List *list, Node *node, Node *after);
Node *extruct_node(List *list, Node *node);
void  delete_node(List *list, Node *node);
void *extruct_data(List *list, Node *node);

Node *mk_node(void *data);
Node *data2node(List l, void *data);
void  clear_list(List *l);

#define add_node_to_head(list, node) \
  insert_node(list, node, NULL)
#define add_node_after(list, node, after) \
  insert_node(list, node, after)
#define add_node_to_tail(list, node) \
  insert_node(list, node, (list)->tail)

#define Stack          List
#define push_node      add_node_to_head
#define pop_node(list) extruct_node(list, (list)->head)

#define Queue          List
#define enq_node       add_node_to_tail
#define deq_node(list) extruct_node(list, (list)->head)

#define FOR_EACH_NODE(list, node) \
  for(node=(list).head; node; node=node->next)
