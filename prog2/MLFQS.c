#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prioque.h"
#include <stdbool.h>

typedef struct Process
{
    int arrival_time;
    int pid;
    int ioCounter;
    unsigned long count;
    unsigned long total_cpu;
    unsigned long total_IO;
    int currentcpuburst;
    int queueLevel;
    Queue behaviors;
    int demotionCounter;
    int promotionCounter;

} Process;

typedef struct ProcessBehavior
{
    unsigned long cpuburst;
    unsigned long ioburst;
    int repeat;
} ProcessBehavior;

typedef struct RR
{
    int quantum;
    int demotionCounter;
    int promotionCounter;
    int queuelevelCounter;
    Queue priority;
} RR;

// initialize global variable
int endprinted = 0;
int currentqueuelevel = 0;
int priorityQueueLevel;
int Clock = 0;
int nullProcessTime = 0;
int counter = 0;

// Queues
Queue IdleProcess;
Queue ArrivalQ;
Queue endQueue;
Queue io;
Queue highestPriorityQueue; // highest priority queue
Queue mediumPriorityQueue;  // medium priority queue
Queue lowestPriorityQueue;  // lowest priority queue
RR priorityQueues[2];       // round robin scheduling

// initialize all queues
void init_all_queues()
{
    init_queue(&ArrivalQ, sizeof(Process), 1, 0, 0);
    init_queue(&endQueue, sizeof(Process), 1, 0, 0);
    init_queue(&io, sizeof(Process), 1, 0, 0);
    init_queue(&priorityQueues[0].priority, sizeof(Process), 1, 0, 0);
    init_queue(&priorityQueues[1].priority, sizeof(Process), 1, 0, 0);
    init_queue(&priorityQueues[2].priority, sizeof(Process), 1, 0, 0);
}

// discord screenshot from 10/11/18
// initialize process descriptor for a new process
void init_process(Process *p)
{
    p->arrival_time = 0;
    p->queueLevel = 0;
    p->ioCounter = 0;
    p->count = 0;
    p->currentcpuburst = 0;
    p->pid = 0;
    p->total_cpu = 0;
    p->total_IO = 0;
    p->demotionCounter = 0;
    p->promotionCounter = 0;
    init_queue(&(p->behaviors), sizeof(ProcessBehavior), 1, 0, 0);
}

// priorityQueues[0] = highestPriorityQueue
// priorityQueues[1] = mediumPriorityQueue
// priorityQueues[2] = lowestPriorityQueue
// init_priority sets the values of quantum, demotion and promotion counter
// for each priority queue
void init_RR()
{
    priorityQueues[0].priority = highestPriorityQueue;
    priorityQueues[1].priority = mediumPriorityQueue;
    priorityQueues[2].priority = lowestPriorityQueue;
    priorityQueues[0].quantum = 10;
    priorityQueues[1].quantum = 30;
    priorityQueues[2].quantum = 100;
    priorityQueues[0].demotionCounter = 1;
    priorityQueues[1].demotionCounter = 2;
    priorityQueues[2].demotionCounter = 25000;
    priorityQueues[0].promotionCounter = 25000;
    priorityQueues[1].promotionCounter = 2;
    priorityQueues[2].promotionCounter = 1;
}

// Check if processes are empty, if not empty return 1
int processes_exist()
{
    int processesExist = 0;
    int i = 0;
    // if ArrivalQ or io is not empty set processesExist to 1
    if (!empty_queue(&ArrivalQ) || !empty_queue(&io) || endprinted == 0)
    {
        processesExist = 1; // true
    }
    // if arrivalQ or io is empty
    else
    {
        // run while i is less then or equal to 2
        while (i <= 2)
        {
            // Check to see if any of the priorityQueues are empty
            // if not empty set processesExist to 1, else it would be 0.
            if (!empty_queue(&priorityQueues[i].priority))
            {
                processesExist = 1; // true
            }
            else
            {
                processesExist = 0; // false
            }
            i++;
        }
    }
    return processesExist; // returns true or false
}

// destry queues
void destroy_queues()
{
    destroy_queue(&IdleProcess);
    destroy_queue(&ArrivalQ);
    destroy_queue(&endQueue);
    destroy_queue(&IdleProcess);
    destroy_queue(&io);
    destroy_queue(&highestPriorityQueue);
    destroy_queue(&mediumPriorityQueue);
    destroy_queue(&lowestPriorityQueue);
}

// do io and prints out queued
void do_IO()
{
    Process *data;
    ProcessBehavior *temp;
    // run when io is not empty
    while (!empty_queue(&io))
    {
        // point Process to current of io
        data = pointer_to_current(&io);
        // point ProcessBehavior to current of behaviors
        temp = pointer_to_current(&(*data).behaviors);
        // decrement io_counter
        (*data).ioCounter = (*data).ioCounter - 1;
        // print queued and delete current if iocounter is zero
        if ((*data).ioCounter == 0)
        {
            // set currentcpuburst to cpuburst
            (*data).currentcpuburst += (*temp).cpuburst;
            // set total_io Io to total_io and ioburst
            (*data).total_IO += (*temp).ioburst;
            // combine total_cpu and total_io
            (*data).total_cpu = (*data).total_cpu + (*data).total_IO;
            add_to_queue(&priorityQueues[(*data).queueLevel].priority, data, 0);
            currentqueuelevel = (*data).queueLevel + 1;
            printf("QUEUED:Process %d queued at level %d at time %u.\n", (*data).pid, currentqueuelevel, Clock);
            remove_from_front(&io, data);
            (*data).ioCounter = (*data).ioCounter + 1;
        }
        else if ((*data).ioCounter == 0)
        {
            rewind_queue(&io);
            // point Process to current of io
            data = pointer_to_current(&io);
            (*data).total_cpu = (*data).total_cpu + (*data).total_IO;
            add_to_queue(&priorityQueues[(*data).queueLevel].priority, data, 0);
            // set currentcpuburst to cpuburst
            (*data).currentcpuburst += (*temp).cpuburst;
            currentqueuelevel = (*data).queueLevel + 1;
            printf("QUEUED:Process %d queued at level %d at time %u.\n", (*data).pid, currentqueuelevel, Clock);
            // set total_io Io to total_io and ioburst
            (*data).total_IO += (*temp).ioburst;
            (*data).ioCounter = (*data).ioCounter + 1;
        }
    }
}

// read all processes descriptions from standard input and populate the ArrivalQueue
void read_process_descriptions()
{
    Process p;
    ProcessBehavior b;
    int pid = 0, first = 1;
    unsigned long arrival;

    init_process(&p);
    arrival = 0;
    while (scanf("%lu", &arrival) != EOF)
    {
        scanf("%d %lu %lu %d", &pid, &b.cpuburst, &b.ioburst, &b.repeat);
        if (!first && p.pid != pid)
        {
            add_to_queue(&ArrivalQ, &p, p.arrival_time);
            init_process(&p);
        }

        p.pid = pid;
        p.arrival_time = arrival;
        first = 0;
        add_to_queue(&p.behaviors, &b, 1);
    }
    add_to_queue(&ArrivalQ, &p, p.arrival_time);
}

// set priority level of queues
int set_priority_level()
{
    int priorityQueueLevel = 9001;
    for (int i = 0; i < 3; i++)
    {
        if (!empty_queue(&priorityQueues[i].priority))
        {
            priorityQueueLevel = i;
        }
    }
    return priorityQueueLevel;
}

// check queues and adds new queue to priority queues and delete
void queue_new_arrivals()
{
    Process *tempData;
    ProcessBehavior *tempProcessBehavior;
    tempData = pointer_to_current(&ArrivalQ);
    while (!(empty_queue(&ArrivalQ)) && (Clock == (*tempData).arrival_time))
    {
        endprinted = false;
        counter = 1;
        // pointer ProcessBehavior to current of behavior
        tempProcessBehavior = pointer_to_current(&(*tempData).behaviors);
        (*tempData).currentcpuburst += (*tempProcessBehavior).cpuburst;
        // print Created
        printf("CREATE: Process %d entered the ready queue at time %d.\n", (*tempData).pid, Clock);
        // adds new queue to priority queues
        add_to_queue(&priorityQueues[0].priority, tempData, 1);
        // delete current element of arrivalQ
        remove_from_front(&ArrivalQ, tempData);
    }
}

// execute highest priority processv by set priority level for queues
void execute_highest_priority_process()
{
    Process *IO;
    ProcessBehavior *process;
    // runs if queue priority level is set
    if (priorityQueueLevel != 9001)
    {
        // point queues to first element
        if (!empty_queue(&priorityQueues[priorityQueueLevel].priority))
        {
            // point IO to current element of priorityQueues[].priority
            IO = pointer_to_current(&priorityQueues[priorityQueueLevel].priority);
            // point process to current element of (*IO).behaviors)
            process = pointer_to_current(&(*IO).behaviors);
            int i = 0;
            // only run if counter equal 0
            while (counter == 0)
            {
                if (queue_length(&priorityQueues[i].priority) != 0)
                {
                    currentqueuelevel = (*IO).queueLevel + 1;
                    printf("RUN: Process %d started execution from level %d at time %d; wants to execute for %d ticks.\n", (*IO).pid, currentqueuelevel, Clock, (*IO).currentcpuburst);
                    //
                    if (!empty_queue(&priorityQueues[i].priority))
                    {
                        rewind_queue(&priorityQueues[i].priority);
                        // decrement currentcpuburst and increase count, totalcpu by one and set counter to one
                        (*IO).currentcpuburst--;
                        (*IO).count++;
                        (*IO).total_cpu++;
                        counter++;
                        rewind_queue(&priorityQueues[i].priority);
                    }
                    else if (empty_queue(&priorityQueues[i].priority))
                    {
                        queue_new_arrivals();
                        rewind_queue(&priorityQueues[i].priority);
                        // decrement currentcpuburst and increase count, totalcpu by one and set counter to one
                        (*IO).currentcpuburst--;
                        (*IO).count++;
                        (*IO).total_cpu++;
                        counter++;
                    }
                }
                i++;
            }
            // decrement currentcpuburst and increase count and totalcpu by one
            (*IO).currentcpuburst--;
            (*IO).count++;
            (*IO).total_cpu++;
        }
        // if currentcpuburst is 0 and repeat is 0 print out finished
        // and add endqueue to priority
        if ((*IO).currentcpuburst == 0)
        {
            if ((*process).repeat == 0 && (*IO).currentcpuburst == 0)
            {
                printf("FINISHED: Process %d finished at time %u.\n", (*IO).pid, Clock);
                (*IO).currentcpuburst--;
                (*IO).count++;
                (*IO).total_cpu++;
                add_to_queue(&endQueue, IO, 1);
                endprinted = true;
            }
            // check if priorityQueues should be promoted and increase
            // and decrease processes and print IO
            else
            {
                /* For promotion calculations, g can't be reset on each I/O, because the idea is that the
                process gets its full CPU need, does I/O, returns to ready queue, gets its full CPU
                need, etc., without exhausting the quantum, g times in a row, to be promoted. So
                the g can't be reset when the process does an I/O, or you can't track this */
                // If a process doesn't use its entire quantum at a particular priority g times, its priority is increased and it moves up one level
                if ((*IO).count < priorityQueues[(*IO).queueLevel].quantum)
                {
                    if ((*IO).currentcpuburst != 0)
                    {
                        // add_to_queue(&io, IO, 1);
                        // increase promotionCounter
                        (*IO).queueLevel++;
                        (*IO).promotionCounter++;
                    }
                }
                else if (priorityQueues[(*IO).queueLevel].quantum > (*IO).count)
                {
                    (*IO).demotionCounter = 0;
                    (*IO).promotionCounter++;
                }
                // reset counts
                counter = 0;
                (*IO).count = 0;
                (*IO).ioCounter += (*process).ioburst;
                (*process).repeat--;
                (*IO).total_cpu++;
                // print process
                printf("I/O: Process %d blocked for I/O at time %u.\n", (*IO).pid, Clock);
                // add io to queue
                (*IO).currentcpuburst++;
                (*IO).total_cpu += (*IO).count;
                add_to_queue(&io, IO, 0);
            }
            remove_from_front(&priorityQueues[priorityQueueLevel].priority, IO);
        }
        // start demotion
        else if (((*IO).count == priorityQueues[priorityQueueLevel].quantum))
        {
            /* For demotion calculations, doing an I/O resets b. The idea here is that burning
            through the entire quantum b times in a row before you do I/O means the process
            should be demoted. If the process does I/O before using the entire quantum, b is
            reset (i.e., it has “behaved” during this execution). */
            // If a process uses its entire quantum at a particular priority b times, its priority is reduced and it moves down one level
            if ((*IO).count == priorityQueues[(*IO).queueLevel].quantum)
            {
                if (((*IO).count == priorityQueues[priorityQueueLevel].quantum))
                {
                    // reset promotionCounter
                    (*IO).promotionCounter = 0;
                    // increase demotionCounter
                    (*IO).demotionCounter++;
                }
                if ((*IO).demotionCounter >= priorityQueues[(*IO).queueLevel].demotionCounter)
                {
                    // increase queue_lvl only if queue_lvl is less then 2
                    if (((*IO).queueLevel < (*IO).count) || ((*IO).count == priorityQueues[priorityQueueLevel].quantum))
                    {
                        (*IO).queueLevel++;
                        // f the process does I/O before using the entire quantum, b is reset
                        if (((*IO).count > priorityQueues[priorityQueueLevel].quantum))
                        {
                            // reset demotionCounter or b
                            (*IO).demotionCounter = 0;
                        }
                        if (((*IO).count < priorityQueues[priorityQueueLevel].quantum))
                        {
                            // reset promotionCounter
                            (*IO).promotionCounter = 0;
                            // increase demotionCounter
                            (*IO).demotionCounter++;
                        }
                    }
                }
            }
            else if ((*IO).count != priorityQueues[(*IO).queueLevel].quantum)
            {
                // add element priority to queue
                add_to_queue(&priorityQueues[(*IO).queueLevel].priority, IO, 1);
                ////increase currentcpuburst by one, reset count and totalcpu by one
                (*IO).count = 0;
                (*IO).currentcpuburst++;
                (*IO).total_cpu++;
                currentqueuelevel = (*IO).queueLevel + 1;
                // print queued
                printf("QUEUED: Process %d queued at level %d at time %u. \n", (*IO).pid, currentqueuelevel, Clock);
                rewind_queue(&priorityQueues[priorityQueueLevel].priority);
                // delete priority at front of queue
                remove_from_front(&priorityQueues[priorityQueueLevel].priority, IO);
            }
        }
        else
        {
            nullProcessTime++; // increase idle
        }
    }
}

// report the final clock time and total
//  CPU usage of all (including the <<null>> process) scheduled when it exits.
void final_report()
{
    Process *temp;
    // print clock
    printf("Scheduler shutdown at time %d.\n", Clock);
    printf("Total CPU usage for all processes scheduled: \n");
    // print process null time
    printf("Process <<null>>: %8d time units.\n", nullProcessTime);
    // run until end of queue
    while (end_of_queue(&endQueue) != 1)
    {
        temp = pointer_to_current(&endQueue);
        printf("Process %d: %12ld time units. \n", (*temp).pid, (*temp).total_cpu);
        next_element(&endQueue);
    }
    // destroy all queues
    destroy_queues();
}

int main(int argc, char *argv[])
{
    init_RR();
    init_all_queues();
    read_process_descriptions();
    while (processes_exist())
    {
        Clock++;
        queue_new_arrivals();
        priorityQueueLevel = set_priority_level();
        execute_highest_priority_process();
        do_IO();
    }
    Clock++;
    final_report();
    return 0;
}