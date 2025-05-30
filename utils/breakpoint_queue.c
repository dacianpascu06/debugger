#include "breakpoint_queue.h"
#include <stdlib.h>

void bp_queue_init(BreakpointQueue *q) {
  q->head = q->tail = NULL;
  q->size = 0;
}

int bp_queue_is_empty(BreakpointQueue *q) { return q->size == 0; }

int bp_queue_enqueue(BreakpointQueue *q, long addr, long orig_instr) {
  Breakpoint *node = malloc(sizeof(Breakpoint));
  if (!node)
    return -1;
  node->addr = addr;
  node->orig_instr = orig_instr;
  node->next = NULL;
  if (q->tail)
    q->tail->next = node;
  else
    q->head = node;
  q->tail = node;
  q->size++;
  return 0;
}

int bp_queue_dequeue(BreakpointQueue *q, Breakpoint *bp) {
  if (bp_queue_is_empty(q))
    return -1;
  Breakpoint *node = q->head;
  *bp = *node;
  q->head = node->next;
  if (!q->head)
    q->tail = NULL;
  free(node);
  q->size--;
  return 0;
}

void bp_queue_free(BreakpointQueue *q) {
  Breakpoint tmp;
  while (!bp_queue_is_empty(q)) {
    bp_queue_dequeue(q, &tmp);
  }
}
