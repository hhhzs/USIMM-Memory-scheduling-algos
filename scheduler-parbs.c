#include <stdio.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"

extern long long int CYCLE_VAL;

void init_scheduler_vars()
{
    // initialize all scheduler variables here

    return;
}

// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 40

// end write queue drain once write queue has this many writes in it
#define LO_WM 20

// 1 means we are in write-drain mode for that channel
int drain_writes[MAX_NUM_CHANNELS];

// Add a macro to define the BATCH_INTERVAL
#define BATCH_INTERVAL 20

// Add a function to calculate the batch number for a request
int get_request_batch(request_t *req) {
    return req->arrival_time / BATCH_INTERVAL;
}

void schedule(int channel)
{
    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;
    request_t * oldest_request = NULL;

    int current_read_batch = CYCLE_VAL / BATCH_INTERVAL;
    int current_write_batch = (write_queue_length[channel] > HI_WM || read_queue_length[channel] == 0) ? current_read_batch : current_read_batch - 1;

    int max_batch_difference = -1;

    // Check both read and write queues for requests to process.
    LL_FOREACH(read_queue_head[channel], rd_ptr) {
        int batch_difference = current_read_batch - get_request_batch(rd_ptr);
        if (rd_ptr->command_issuable && batch_difference >= max_batch_difference) {
            oldest_request = rd_ptr;
            max_batch_difference = batch_difference;
        }
    }

    LL_FOREACH(write_queue_head[channel], wr_ptr) {
        int batch_difference = current_write_batch - get_request_batch(wr_ptr);
        if (wr_ptr->command_issuable && batch_difference >= max_batch_difference) {
            oldest_request = wr_ptr;
            max_batch_difference = batch_difference;
        }
    }

    // If an eligible request was found, issue its command.
    if (oldest_request != NULL) {
        issue_request_command(oldest_request);
    }
}

void scheduler_stats()
{
    /* Nothing to print for now. */
}
