#include <stdio.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"
#include "scheduler.h"

extern long long int CYCLE_VAL;

void init_scheduler_vars()
{
    // Initialize all scheduler variables here

    return;
}

// Write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 40

// End write queue drain once write queue has this many writes in it
#define LO_WM 20

// 1 means we are in write-drain mode for that channel
int drain_writes[MAX_NUM_CHANNELS];

// Age threshold to differentiate between young and old requests
#define AGE_THRESHOLD 50

void schedule(int channel)
{
    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;

    // If in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // Initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if(drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                break;
            }
        }
        return;
    }

    // Staged memory scheduling
    request_t * oldest_young_request = NULL;
    request_t * oldest_request = NULL;

    LL_FOREACH(read_queue_head[channel], rd_ptr)
    {
        if(rd_ptr->command_issuable)
        {
            long long int age = CYCLE_VAL - rd_ptr->arrival_time;

            // Prioritize older requests
            if(age >= AGE_THRESHOLD)
            {
                if(!oldest_request || rd_ptr->arrival_time < oldest_request->arrival_time)
                {
                    oldest_request = rd_ptr;
                }
            }
            // Keep track of the oldest young request
            else
            {
                if(!oldest_young_request || rd_ptr->arrival_time < oldest_young_request->arrival_time)
                {
                    oldest_young_request = rd_ptr;
                }
            }
        }
    }

    // Issue the oldest request if there is one
    if (oldest_request)
    {
        issue_request_command(oldest_request);
    }
    // If there is no old request, issue the oldest young request if there is one
    else if (oldest_young_request)
    {
        issue_request_command(oldest_young_request);
    }

    return;
}

void scheduler_stats()
{
    /* Nothing to print for now. */
}
