/*
 * TOPPERS/EV3RT
 * 
 * Copyright (C) 2016 by Yixiao Li <liyixiao7@gmail.com>
 * 
 *  The above copyright holders grant permission gratis to use,
 *  duplicate, modify, or redistribute (hereafter called use) this
 *  software (including the one made by modifying this software),
 *  provided that the following four conditions (1) through (4) are
 *  satisfied.
 * 
 *  (1) When this software is used in the form of source code, the above
 *      copyright notice, this use conditions, and the disclaimer shown
 *      below must be retained in the source code without modification.
 * 
 *  (2) When this software is redistributed in the forms usable for the
 *      development of other software, such as in library form, the above
 *      copyright notice, this use conditions, and the disclaimer shown
 *      below must be shown without modification in the document provided
 *      with the redistributed software, such as the user manual.
 * 
 *  (3) When this software is redistributed in the forms unusable for the
 *      development of other software, such as the case when the software
 *      is embedded in a piece of equipment, either of the following two
 *      conditions must be satisfied:
 * 
 *    (a) The above copyright notice, this use conditions, and the
 *        disclaimer shown below must be shown without modification in
 *        the document provided with the redistributed software, such as
 *        the user manual.
 * 
 *    (b) How the software is to be redistributed must be reported to the
 *        TOPPERS Project according to the procedure described
 *        separately.
 * 
 *  (4) The above copyright holders and the TOPPERS Project are exempt
 *      from responsibility for any type of damage directly or indirectly
 *      caused from the use of this software and are indemnified by any
 *      users or end users of this software from any and all causes of
 *      action whatsoever.
 * 
 *  THIS SOFTWARE IS PROVIDED "AS IS." THE ABOVE COPYRIGHT HOLDERS AND
 *  THE TOPPERS PROJECT DISCLAIM ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, ITS APPLICABILITY TO A PARTICULAR
 *  PURPOSE. IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS AND THE
 *  TOPPERS PROJECT BE LIABLE FOR ANY TYPE OF DAMAGE DIRECTLY OR
 *  INDIRECTLY CAUSED FROM THE USE OF THIS SOFTWARE.
 */

/**
 * Common header files
 */
#include "ev3api.h"
#include "target_test.h"

#define NUM_TASKS 5
#define NUM_MUTEX 3
#define MAX_INTERRUPTS 300
#define RUN_UNTIL 2100

#define IDLE 0
#define TSK_0 'A'
#define TSK_1 'B'
#define TSK_2 'C'
#define TSK_3 'D'
#define TSK_4 'E'

/**
 * Task priorities (smaller number has higher priority)
 */

#define PRIORITY_PRD_TSK_0 TMIN_APP_TPRI + 1
#define PRIORITY_PRD_TSK_1 TMIN_APP_TPRI + 2
#define PRIORITY_PRD_TSK_2 TMIN_APP_TPRI + 3
#define PRIORITY_PRD_TSK_3 TMIN_APP_TPRI + 4
#define PRIORITY_PRD_TSK_4 TMIN_APP_TPRI + 5

//#define PRIORITY_LOG_TSK TMIN_APP_TPRI
//#define PRIORITY_AT_TSK TMIN_APP_TPRI + 2

#define PRIORITY_MAIN_TASK (TMIN_APP_TPRI)

/**
 * Task periods in ms
 */

#define PERIOD_PRD_TSK_0  (1800)
#define PERIOD_PRD_TSK_1  (2000)
#define PERIOD_PRD_TSK_2  (2500)
#define PERIOD_PRD_TSK_3 (3000)
#define PERIOD_PRD_TSK_4 (3500)
//#define PERIOD_LOG_TSK  (100)
//#define PERIOD_AT_TSK  (1500)

//#define HYPERPERIOD (1500)

//execution times of tasks

/*
#define EXEC_TSK_0  (100)
#define EXEC_TSK_1  (500)
#define EXEC_TSK_2  (700)
#define EXEC_TSK_3 (500)
#define EXEC_TSK_4 (100)
*/
//#define EXEC_LOG_TSK  (3)
//#define EXEC_AT_TSK  (100)

//Task phases

/*#define PHASE_TSK_1  (400)
#define PHASE_TSK_2  (300)
#define PHASE_TSK_3 (100)
#define PHASE_TSK_4 (0)
*/
/**
 * Default task stack size in bytes
 */
#ifndef STACK_SIZE
#define	STACK_SIZE 4096
#endif

/**
 * Prototypes for configuration
 */
#ifndef TOPPERS_MACRO_ONLY

extern void	main_task(intptr_t);
extern void periodic_task_0(intptr_t);
extern void periodic_task_1(intptr_t);
extern void periodic_task_2(intptr_t);
extern void periodic_task_3(intptr_t);
extern void periodic_task_4(intptr_t);
//extern void at_task(intptr_t);
//extern void log_task(intptr_t);

extern void task_activator(intptr_t tskid);

#endif /* TOPPERS_MACRO_ONLY */

