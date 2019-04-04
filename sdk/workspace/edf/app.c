/**
 * Anupam Godse     angodse@ncsu.edu
 * Brayden McDonald bwmcdon2@ncsu.edu
 */

/**
 * This program demonstrates 3 cyclic tasks which log data to 
 * indicate edf scheduling for modified kernel
 */

#include "app.h"
#include <stdlib.h>

#include <string.h>

/*struct to log preemption data*/
typedef struct loggingStruct{
    uint32_t num_invoc;

    uint32_t num_interrupts;

    SYSTIM times[MAX_INTERRUPTS];

} LOG;

/*log preemption data of NUM_TASKS*/
LOG logs[NUM_TASKS];

uint8_t now_running = 0;  //stores currently running task
uint8_t prev_running = 0; //stores previously running task

uint16_t running_log_index = 0; //stores index of running_log/invocation_table
char running_log[600];  //log currently running task (every 100ms)
uint8_t invocation_table[600]; //log the number of invocations of currently running task


//not used
static FILE *bt = NULL;

// a cyclic handler to activate a task
void task_activator(intptr_t tskid) {
    ER ercd = act_tsk(tskid);
    assert(ercd == E_OK);
}

// a periodic task 1 with period 300ms and execution time 100ms
void periodic_task_1(intptr_t unused) {
    SYSTIM start, now, prev;
    get_tim(&start);
    int count=0;
    prev = start;
    now = start;

    //if the task preempts some other task then log it
    if(now_running != IDLE){
        int n = (logs[now_running-TSK_1].num_interrupts)++;
        get_tim(&(logs[now_running-TSK_1].times[n]));
    }
    //set you are running
    now_running = TSK_1;

    //increment the number of invocations
    int t = logs[TSK_1-TSK_1].num_invoc++;

    //log you are running now
    running_log[running_log_index] = TSK_1;

    //log the number of invocations at this time
    invocation_table[running_log_index++] = t+1;
    
    //burn 100ms cpu time
    while(count < EXEC_TSK_1) {
        while(prev == now){
            get_tim(&now);
            now_running = TSK_1;
        }
        count++;
        prev=now;
    }
    //set previously running task as you
    prev_running = TSK_1;

    //set now running to IDLE
    now_running = IDLE; 
}

// a periodic task 2 with period 500ms and execution time 300ms
void periodic_task_2(intptr_t unused) {
    SYSTIM start, now, prev;
    get_tim(&start);
    int count = 0;
    now = start;
    prev = start;

    //if the task preempts some other task then log it
    if(now_running != IDLE){
        int n = (logs[now_running-TSK_1].num_interrupts)++;
        get_tim(&(logs[now_running-TSK_1].times[n]));
    }
    //set you are running
    now_running = TSK_2;

    //increment the number of invocations
    int t = logs[TSK_2-TSK_1].num_invoc++;

    int i;
    //burn 300ms cpu time and log stuff
    for(i = 0; i < 3; i++) {
        count=0;
        //log you are running now
        running_log[running_log_index] = TSK_2;

        //log the number of invocations at this time
        invocation_table[running_log_index++] = t+1;

        //burn 100ms cpu time
        while(count < EXEC_TSK_1) {
            while(prev == now){
                get_tim(&now);
                now_running = TSK_2;
            }
            count++;
            prev=now;
        }
    }

    //set previously running task as you
    prev_running = TSK_2;

    //set now running to IDLE
    now_running = IDLE; 
}

// a periodic task with period 1500ms and execution time 100ms
void periodic_task_3(intptr_t unused) {
    SYSTIM start, now, prev;
    int count = 0;
    get_tim(&start);
    prev = start;
    now = start;

    //if the task preempts some other task then log it
    if(now_running != IDLE){
        int n = (logs[now_running-TSK_1].num_interrupts)++;
        get_tim(&(logs[now_running-TSK_1].times[n]));
    }
    //set you are running
    now_running = TSK_3;

    //increment the number of invocations
    int t = logs[TSK_3-TSK_1].num_invoc++;

    //log you are running now
    running_log[running_log_index] = TSK_3;
    
    //log the number of invocations at this time
    invocation_table[running_log_index++] = t+1;

    //burn 100ms cpu time
    while(count < EXEC_TSK_1) {
        while(prev == now){
            get_tim(&now);
            now_running = TSK_3;
        }
        count++;
        prev=now;
    }
    
    //set previously running task as you
    prev_running = TSK_3;
    
    //set now running to IDLE
    now_running = IDLE; 
}

//main task having higher static priority than any other tasks
void main_task(intptr_t unused) {

    int i,j, temp;
    
    //initialize logs array
    for(i=0; i<NUM_TASKS; i++) {
        logs[i].num_invoc = 0;
        logs[i].num_interrupts = 0;
    }

    //sleep for 2 hyperperiods, let other tasks run and log data
    tslp_tsk(2*HYPERPERIOD);

    //stop cycles of all tasks after two hyperperiods
    ev3_stp_cyc(CYC_PRD_TSK_1);
    ev3_stp_cyc(CYC_PRD_TSK_2);
    ev3_stp_cyc(CYC_PRD_TSK_3);

    // Open Bluetooth file
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);
    setvbuf(bt, NULL, _IONBF, 0);

    //append '/0' to running_log
    running_log[running_log_index] = '\0';

    //print running log
    //fprintf(bt, "%s\n", running_log);
    printf("%s\n", running_log);

    //print invocation log 
    for(i=0; i<running_log_index; i++) {
        //fprintf(bt, "%d", invocation_table[i]);
        printf("%d", invocation_table[i]);
    }
    //fprintf(bt, "\n");
    printf("\n");

    //print which task was preempted and how many times it was preempted
    for(i=0; i<NUM_TASKS; i++) {
        temp = logs[i].num_interrupts;
        printf("task %d preempted %d times\n", i+1, temp);
        for(j=0; j < temp; j++) { //print times at which task was preempted
           //fprintf(bt, "@  %lu\n", logs[i].times[j]);
           printf("@  %lu\n", logs[i].times[j]);
        } 
    }

}
