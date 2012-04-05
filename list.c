/*See LICENSE file for copyright and license details.*/
/*Double-linked list, stack, queue.*/

#include <malloc.h>
#include "bool.h"
#include "list.h"

/*Create in heap node that points to 'data' and return
  pointer to rhis node.*/
Node *mk_node(void *data){
  Node *n = malloc(sizeof(Node));
  n->data = data;
  n->next = NULL;
  n->prev = NULL;
  return(n); 
}

/*If 'after' is NULL, then 'node' will be added at the head
  of the list, else it will be added following 'after'.*/
void insert_node(List *list, Node *node, Node *after){
  if(after){
    node->next = after->next;
    node->prev = after;
    after->next = node;
  }else{
    node->next = list->head;
    node->prev = NULL;
    list->head = node;
  }
  if(node->next)
    node->next->prev = node;
  else
    list->tail = node;
  list->count++;
}

/*Extructs node from list, returns pointer to this node.*/
Node *extruct_node(List *list, Node *node){
  if(!node)
    return(NULL);
  if(node->next)
    node->next->prev = node->prev;
  else
    list->tail = node->prev;
  if(node->prev)
    node->prev->next = node->next;
  else
    list->head = node->next;
  list->count--;
  return(node);
}

/*Delete data and node.*/
void delete_node(List *list, Node *node){
  Node *tmp = extruct_node(list, node);
  free(tmp->data);
  free(tmp);
}

/*Extruct node from list, delete node,
  return pointer to data.*/
void *extruct_data(List *list, Node *node){
  Node *tmp = extruct_node(list, node);
  void *data = node->data;
  free(tmp);
  return(data);
}

void clear_list(List *l){
  while(l->count)
    delete_node(l, l->head);
}

Node *data2node(List l, void *data){
  Node *node;
  FOR_EACH_NODE(l, node)
    if(node->data == data)
      return(node);
  return(NULL);
}