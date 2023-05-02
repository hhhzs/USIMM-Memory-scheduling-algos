#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

void init_scheduler_vars(); //called from main
void scheduler_stats(); //called from main
void schedule(int); // scheduler function called every cycle

// Add the following function declaration to the header file:
int get_request_batch(request_t *req);

#endif //__SCHEDULER_H__
