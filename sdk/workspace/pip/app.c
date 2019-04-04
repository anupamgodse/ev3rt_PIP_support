/**
 * Anupam Godse     angodse@ncsu.edu
 * Brayden McDonald bwmcdon2@ncsu.edu
 */

/**
 * This program demonstrates tasks using resources via mutexes
 * to verify the PIP implementation of modified kernel
 */

#include "app.h"
#include <stdlib.h>

#include <string.h>
#include "../../kernel/kernel_impl.h"

/* Distinguish between proper and improper nesting
 * If proper nesting of locks then the kernel takes 
 * O(1) time to recompute priority on unlock.
 * If improper nesting of locks, then the kernel
 * takes O(tasks+locks) time to recompute priority
 * on unlock
 * Programmer does not need to specify to kernel
 * if nesting is proper or improper.
 * This is just to ensure the kernel implementation 
 * for proper and improper works properly
 * */
#define PROPER_NESTING 1

/*struct to log preemption data*/
typedef struct loggingStruct{
    int waiting_on;
    int is_runnable;
    int times_direct_blocked;
    int times_trans_blocked;
    int times_inherit_blocked;
    int flag;
    int locks[NUM_MUTEX];

    uint8_t d_blocked[NUM_TASKS];
    uint8_t t_blocked[NUM_TASKS];

    uint32_t num_invoc;

    uint32_t num_interrupts;

    SYSTIM times[MAX_INTERRUPTS];

} LOG;


/*log preemption data of NUM_TASKS*/
LOG logs[NUM_TASKS];

uint8_t now_running = 0;  //stores cur:rently running task
uint8_t prev_running = 0; //stores previously running task

uint16_t running_log_index = 0; //stores index of running_log/invocation_table
char running_log[RUN_UNTIL+1];  //log currently running task (every 100ms)
//uint8_t invocation_table[600]; //log the number of invocations of currently running task


//not used
static FILE *bt = NULL;

// a cyclic handler to activate a task
void task_activator(intptr_t tskid) {
    ER ercd = act_tsk(tskid);
    assert(ercd == E_OK);
}

/*check if the task is tranitively blocked
 * check calling funciton for explanation*/
void increase_trans_count(int task) {
    int my_id = task, i, j, k;
    for(j=0;j<NUM_MUTEX;j++) {
        if(logs[task].locks[j] == 1) {
            for(k=0; k<my_id; k++) {
                if(logs[k].flag == 0 && logs[k].waiting_on == j+MTX_1) {
                    logs[k].times_trans_blocked++;
                    logs[k].flag = 1;
                    increase_trans_count(k);
                }
            }
        }
    } 
}

/*when unlocking a mutex log all types of blocking by mutex holding task*/
void log_blocking(int task, int mutex_id) {
    int my_id = task-TSK_0, i;
    for(i=0; i<my_id; i++) { //only higher prirority tasks can be blocked
        //direct blocking
        if(logs[i].flag == 0 && logs[i].waiting_on == mutex_id) {
            logs[i].times_direct_blocked++;            
            logs[i].flag = 1;
            /*if task with id i is directly blocked then 
             * all tasks which are waiting on the resources held 
             * by this task are transitively blocked by task 
             * which directly blocks task i*/
            increase_trans_count(i);
        }
    }
    /*if the higher priority task is not direct or transitively 
     * blocked then it must be inheritance blocked*/
    for(i=0; i < my_id; i++) {
        if(logs[i].flag == 0 && logs[i].is_runnable == 1) {
            logs[i].times_inherit_blocked++;
        }
    }
    for(i=0; i < NUM_TASKS; i++) {
        logs[i].flag = 0;
    }
}

/*check if this task has preempted any other task*/
void log_preemption(uint8_t now_running) {
    int n = (logs[now_running-TSK_0].num_interrupts)++;
    get_tim(&(logs[now_running-TSK_0].times[n]));
}

/* Burn CPU cycles for time ms.
 * Used to determine execution times of tasks
 */
void burn(int time, uint8_t task_id) {
	now_running = task_id;
    SYSTIM prev, now;
    get_tim(&now);
    prev = now;
    int count=0;
    while(count < time) {
        while(prev == now){
            get_tim(&now);
            now_running = task_id;
        }
        if(count == time/2) {
            running_log[running_log_index++] = task_id;
        }
        count++;
        prev=now;
    }
}

/*wrapper function to log when locking a mutex*/
void wrap_loc(uint8_t mutex, int task) {
    printf("task %d acquiring %d\n", task-TSK_0, mutex-MTX_1);
    logs[task-TSK_0].waiting_on = mutex;
    logs[task-TSK_0].is_runnable = 0;
    loc_mtx(mutex);
    logs[task-TSK_0].is_runnable = 1;
    logs[task-TSK_0].locks[mutex-MTX_1] = 1;
    logs[task-TSK_0].waiting_on = -1;
    printf("task %d acquired %d\n", task-TSK_0, mutex-MTX_1);
}

/*wrapper function to log when unlocking a mutex*/
void wrap_loc(uint8_t mutex, int task) {
void wrap_unl(uint8_t mutex, int task) {
    log_blocking(task, mutex);
    printf("task %d unlocking %d\n", task-TSK_0, mutex-MTX_1);
    unl_mtx(mutex);
    logs[task-TSK_0].locks[mutex-MTX_1] = 0;
    printf("task %d unlocked %d\n", task-TSK_0, mutex-MTX_1);
}


void periodic_task_0(intptr_t unused) {
    logs[0].is_runnable = 1;
  	//if the task preempts some other task then log it
    if(now_running != IDLE){
        int n = (logs[now_running-TSK_0].num_interrupts)++;
        get_tim(&(logs[now_running-TSK_0].times[n]));
    } 

    now_running = 'A';
    wrap_loc(MTX_3, 'A');

    if(now_running != 'A' && now_running != IDLE) {
        log_preemption(now_running);
    }
    burn(100, 'A');

    wrap_unl(MTX_3, 'A');

	now_running = IDLE;
    logs[0].is_runnable = 0;
}

void periodic_task_1(intptr_t unused) {
    logs[1].is_runnable = 1;
  	//if the task preempts some other task then log it
    if(now_running != IDLE){
        log_preemption(now_running);
    } 

    now_running = 'B';

    burn(100, 'B');

    burn(100, 'B');

    wrap_loc(MTX_1, 'B');
    if(now_running != 'B' && now_running != IDLE) {
        log_preemption(now_running);
    }

    burn(100, 'B');

    wrap_unl(MTX_1, 'B');

    burn(100, 'B');

	now_running = IDLE;
    logs[1].is_runnable = 0;
}

void periodic_task_2(intptr_t unused) {
    logs[2].is_runnable = 1;
  	//if the task preempts some other task then log it
    if(now_running != IDLE){
        log_preemption(now_running);
    } 

    now_running = 'C';
    wrap_loc(MTX_1, 'C');

    if(now_running != 'C' && now_running != IDLE) {
        log_preemption(now_running);
    }

    burn(100, 'C');
    burn(100, 'C');

    now_running = 'C';

#if !PROPER_NESTING
    wrap_loc(MTX_2, 'C');

    if(now_running != 'C' && now_running != IDLE) {
        log_preemption(now_running);
    }
#endif    

    burn(100, 'C');

#if !PROPER_NESTING
    wrap_unl(MTX_2, 'C');
#endif    

    wrap_unl(MTX_1, 'C');

    burn(100, 'C');

	now_running = IDLE;
    logs[2].is_runnable = 0;
}
// a periodic task 1 with period 300ms and execution time 100ms
void periodic_task_3(intptr_t unused) {
    logs[3].is_runnable = 1;
  	//if the task preempts some other task then log it
    if(now_running != IDLE){
        log_preemption(now_running);
    } 


    now_running = 'D';
    wrap_loc(MTX_2, 'D');

    if(now_running != 'D' && now_running != IDLE) {
        log_preemption(now_running);
    }

    burn(100, 'D');
    burn(100, 'D');

    wrap_unl(MTX_2, 'D');

    burn(100, 'D');

	now_running = IDLE;
    logs[3].is_runnable = 0;
}

void periodic_task_4(intptr_t unused) {
    logs[4].is_runnable = 1;
  	//if the task preempts some other task then log it
    if(now_running != IDLE){
        log_preemption(now_running);
    } 

    now_running = 'E';
    wrap_loc(MTX_3, 'E');

    if(now_running != 'E' && now_running != IDLE) {
        log_preemption(now_running);
    }

    burn(100, 'E');
    burn(100, 'E');

    wrap_unl(MTX_3, 'E');

    burn(100, 'E');
	now_running = IDLE;
    logs[4].is_runnable = 0;
}

void main_task(intptr_t unused) {

    int i,j, temp;
    int start;
    
    //initialize logs array
    for(i=0; i<NUM_TASKS; i++) {
        logs[i].num_invoc = 0;
        logs[i].flag = 0;
        logs[i].num_interrupts = 0;
        logs[i].waiting_on = -1;
        logs[i].is_runnable = 0;
        logs[i].times_direct_blocked=0;
        logs[i].times_trans_blocked=0;
        logs[i].times_inherit_blocked=0;
        for(j=0;j<NUM_MUTEX;j++) {
            logs[i].locks[j] = 0;
        }
        for(j=0;j<NUM_TASKS;j++) {
            logs[i].d_blocked[j] = -1;
            logs[i].t_blocked[j] = -1;
        }
    }

    //start task cycles with appropriate phase timings
    ev3_sta_cyc(CYC_PRD_TSK_4);

    tslp_tsk(100);
    ev3_sta_cyc(CYC_PRD_TSK_3);
    
    tslp_tsk(100);
    ev3_sta_cyc(CYC_PRD_TSK_2);

    tslp_tsk(100);
    ev3_sta_cyc(CYC_PRD_TSK_1);

#if !PROPER_NESTING
    tslp_tsk(100);
    ev3_sta_cyc(CYC_PRD_TSK_0);
#endif

    //sleep for 1500 ms < HYPERPERIOD
    //we ensure that we encounter all types of blocking in this period
    tslp_tsk(1500);

    //stop cycles of all tasks
#if !PROPER_NESTING
    ev3_stp_cyc(CYC_PRD_TSK_0);
#endif
    ev3_stp_cyc(CYC_PRD_TSK_1);
    ev3_stp_cyc(CYC_PRD_TSK_2);
    ev3_stp_cyc(CYC_PRD_TSK_3);
    ev3_stp_cyc(CYC_PRD_TSK_4);

    //append '/0' to running_log
    running_log[running_log_index] = '\0';

    printf("running_log= %s\n", running_log);

#if PROPER_NESTING
    start=1;
#else
    start=0;
#endif
    for(i=start; i<NUM_TASKS; i++) {
        temp = logs[i].num_interrupts;
        printf("task %d preempted %d times\n", i, temp);
        for(j=0; j < temp; j++) { //print times at which task was preempted
           fprintf(bt, "@  %lu\n", logs[i].times[j]);
           printf("@  %lu\n", logs[i].times[j]);
        }

        printf("task %d was directly blocked %d times\n", i, logs[i].times_direct_blocked);
        printf("task %d was inheritance blocked %d times\n", i, logs[i].times_inherit_blocked);
        printf("task %d was transitive blocked %d times\n", i, logs[i].times_trans_blocked);
    }

}
