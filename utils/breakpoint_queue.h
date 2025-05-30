#ifndef BREAKPOINT_QUEUE_H
#define BREAKPOINT_QUEUE_H

typedef struct Breakpoint {
  long addr;
  long orig_instr;
  struct Breakpoint *next;
} Breakpoint;

typedef struct {
  Breakpoint *head;
  Breakpoint *tail;
  int size;
} BreakpointQueue;

extern BreakpointQueue queue;

void bp_queue_init(BreakpointQueue *q);
int bp_queue_enqueue(BreakpointQueue *q, long addr, long orig_data);
int bp_queue_dequeue(BreakpointQueue *q, Breakpoint *bp);
int bp_queue_is_empty(BreakpointQueue *q);
void bp_queue_free(BreakpointQueue *q);

#endif
