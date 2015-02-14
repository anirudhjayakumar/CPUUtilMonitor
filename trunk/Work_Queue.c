
#include <linux/slab.h> // for kmalloc
#include <linux/workqueue.h> // workqueue_struct
#include<linux/timer.h>
#include<linux/jiffies.h>
#include<linux/mutex.h>
#include <linux/pid.h>
#include "common.h"
#define find_task_by_pid(nr) pid_task(find_vpid(nr), PIDTYPE_PID)

//THIS FUNCTION RETURNS 0 IF THE PID IS VALID AND THE CPU TIME IS SUCCESFULLY RETURNED BY THE PARAMETER CPU_USE. OTHERWISE IT RETURNS -1

int get_cpu_use(int pid, unsigned long *cpu_use)
{
   struct task_struct* task;
   rcu_read_lock();
   task=find_task_by_pid(pid);
   if (task!=NULL)
   {  
	*cpu_use=task->utime;
        rcu_read_unlock();
        return SUCCESS;
   }
   else 
   {
     rcu_read_unlock();
     return FAIL;
   }
}

/*Work Queue:- my_wq*/
static struct workqueue_struct *my_wq;

//creating work element:- work
typedef struct {
	struct work_struct work;
	int number;
} Works;
Works *wk1; //an instance of work

//Work handler to update CPU time in Linked List

static void work_handler( struct work_struct *work )
{
	Works *wk = (Works*)work;
	/*Insert code here for updation of CPU Time*/
    // get the pids
    int *pids = NULL;
	int count = 0;
    int index,pid;
	unsigned long cpu_time;
	ll_get_pids(&pids,&count);
    for (index = 0; index <count; ++index)
	{
		pid = pids[index];

		if( get_cpu_use(pid,&cpu_time) == SUCCESS )
		{
			ll_update_time(pid,cpu_time);
		}
		else
		{
			ll_delete_pid(pid);
		}
	}
	// call the function
	// update the cpu time
	// if possible, if pid not there delete from list 
	kfree( (void*)work );


/** function to create workqueue*/
void create_work_queue(void)
{
	my_wq= create_workqueue( "mp1_workqueue" );
	if ( !my_wq ) {
		printk( "Error!Workqueue could not be created\n" );
		return ;
	}
	wk1 = (Works*)kmalloc( sizeof(Works), GFP_KERNEL );
	if ( wk1 ) {
		INIT_WORK( &wk1->work, work_handler );
		//wk1->number = 1;
	}
}

static struct timer_list intr_timer;

static void timer_callback(unsigned long data){
	queue_work( my_wq, &wk1->work );
	mod_timer(&intr_timer,jiffies+5*HZ);
}

/* Function to Initialize Timer*/
static void initialize_timer(void){
	int wait_time = 5;
	init_timer(&intr_timer);
	intr_timer.expires = jiffies+wait_time*HZ;
	intr_timer.data = intr_timer.expires;
	intr_timer.function = timer_callback;
	add_timer(&intr_timer); 
}
/*
int init_module(void)
{
	initialize_timer();//Initializing timer
	create_work_queue(); //calling workqueue
	//queue_work( my_wq, &wk1->work );
	return 0;
}

void cleanup_module()
{
	del_timer(&intr_timer);
	flush_workqueue( my_wq );
	destroy_workqueue( my_wq );
}
*/
