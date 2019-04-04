Single Author info:

bwmcdon2 Brayden W McDonald

Group info:

angodse Anupam N Godse

#Modified Toppers Kernel with support for Priority Inheritance Protocol in Mutexes
EV3 Mindstorm robot programmed with EV3RT

Description: We modified the Toppers kernel to implement the Priority Inheritance Protocol for mutexes. This protocol is in addtion to the Priority Ceiling Protocol that is already implemented. When declaring a mutex in an application's .cfg file, the programmer specifies that the mutex should follow the priority inheritance protocol by passing it the TA_INHERIT flag in the declaration. 

To implement priority inheritance, we made several changes to files in the kernel, outlined below:
	-We added an ID field to the TCB called waiting_mutex, which holds the mtxid of the mutex that the task is waiting on (it is -1 if the task is not waiting on a mutex). This only needed to be a single field because a task can only be waiting on one mutex at a time. 
	-We created a global flag that tracks whether the system of tasks is obeying proper nesting
	-A global QUEUE name nestQueue was created. It is used to track what order mutexes have been locked or unlocked in. This was not explicitly necessary for PIP, but was needed to check if proper nesting had been followed.
	-We added a QUEUE field to the MTXCB struct called global_lock_order. It is used to attach mutexes to nestQueue.
	-We added a previous_priority field to the MTXCB that is used to restore a locking task's priority only if proper nesting has been obeyed
	
	The initialization functions for TCBs and MTXCBs were modified to initialize the new fields, and the kernel.h file was changed to contain the #define for the TA_INHERIT flag.
	
	The loc_mtx() function was modified, if the mutex being locked has the TA_INHERIT flag set, to perform the following actions:
		-set the locking task's waiting_mutex variable to the mtxid of the mutex it is attempting to lock	
		-call priority_increase() on the task currently holding the mutex.
		once the task acquires the mutex, it;
		-sets the TCB's waiting_mutex field to -1
		-inserts the mutex into the global nestQueue QUEUE (not needed except to track proper lock nesting.)
		
	The unl_mtx() function was modified to add the following behaviors if the TA_INHERIT flag is set:
		-it checks if the lock being unlocked obeys proper nesting order, and if the proper nesting flag is set to true
		-if both are true, then the unlocking task's priority will be set to the mutex's previous priority value
		-if the lock is not the correct lock under proper nesting, then the nesting flag will be set to false, and the unlocking task's priority will be recalculated via the dyn_priority() function
		
	The dyn_priority() function was created, and it follows the pseudocode given in the slides. It iterates over the list of mutexes the task still holds, and finds the maximum priority of all the tasks that are waiting on it.
	
	
	
	
	
Testing: To test the implementation of PIP, we designed a system of taskes that featured examples of all three kinds of blocking; direct blocking, transitive blocking, and inheritance blocking. The task set created is as follows:

	P = priority, E = execution time, p = phase, and locks go lock(from-to)

		P      E     p     locks
	T0: 2000   100   400   Z(0-1) 
	T1: 2500   400   300   X(2-3)
	T2: 3000   400   200   X(0-3) Y(2-3) 
	T3: 3500   300   100   Y(0-2)
	T4: 4000   300     0   Z(0-2)



A chart detailing the execution history of the system until all tasks are complete is shown below:

Legend: X2 indicates that the process is holding lock X and is running at priority 2. 
		N is used to indicate that a process is holding no locks 
		b indicates that a process is currently blocked by a process holding a required resource. 
		Smaller priority numbers indicate a higher priority. 



time:    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14
	T0: |___|___|___|___|b__|_Z0|___|___|___|___|___|___|___|___|___|
	T1: |___|___|___|_N1|b__|___|_N1|b__|b__|b__|_X1|_N1|___|___|___|
	T2: |___|___|_X2|___|b__|___|___|_X1|b__|XY1|___|___|_N2|___|___|
	T3: |___|_Y3|___|___|b__|___|___|___|_Y1|___|___|___|___|_N3|___|
	T4: |_Z4|___|___|___|_Z0|___|___|___|___|___|___|___|___|___|_N4|
	
	
	
	
	
To test our system's ability to take advantage of properly nested locks, we designed the following system, which exhibits proper nesting:

time:    0   1   2   3   4   5   6   7   8   9   10  11  12  
	T1: |___|___|___|_N1|_N1|b__|b__|_X1|_X1|___|___|___|___|___|
	T2: |___|___|_X2|___|___|_X1|_X1|___|___|_N2|___|___|___|___|
	T3: |___|_Y3|___|___|___|___|___|___|___|___|_Y3|_N3|___|___|
	T4: |_Z4|___|___|___|___|___|___|___|___|___|___|___|_Z4|_N4|
	
	
	Our test program prints its execution history in a gantt chart, the same way as the test program from homework 3. In this case, tasks lock, unlock, and are released at multiples of 100 miliseconds, while the executing task is only recorded multiples of 50 that are *not* also multiples of 100, ensuring that the gantt chart is the same every time. 
	
	Under the first set, we see history EDCBEABCDCBBCDE
	Under the second, we see EDCBBCCBBCDDEE
				  
	
	
	
	
	
