#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h> // for kmalloc
#include <linux/workqueue.h> // workqueue_struct
#include<linux/timer.h>
#include<linux/jiffies.h>
#include<linux/mutex.h>
#include "linklist.h"
#define BUF_SIZE 1024

#define TIMER_PERIOD 5 // in millisecs
#define SUCCESS 0
#define FAIL -1
#define TRUE 1
#define FALSE 0
/* list of kernel error codes can be found here 
http://linuxdeveloper.blogspot.com/2012/02/kernel-error-codes.html
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group_2");
MODULE_DESCRIPTION("CS-423 MP1");
struct mutex my_mutex; // mutex to synchronize
// structure that stores info regarding each process

struct process_info {
	int pid;
	unsigned long cpu_time;
	struct list_head list; 
	/* list_head is added to the struct which need to be a list. This apporach is
	*  different from conventional approach of add the struct to a list. Here it 
	*  is the reverse.
	*/
};

// declare and intilaize the list
static struct process_info proc_list;
static int list_size = 0;

int ll_initialize_list(void)
{
	INIT_LIST_HEAD(&proc_list.list);
	return SUCCESS;
}


int ll_is_pid_in_list(int pid)
{
	struct process_info *proc_iter = NULL;
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		if(proc_iter->pid == pid) 
			return TRUE;
	}
	return FALSE;
}
/**
int ll_generate_cpu_info_string(char **buf, int *count_)
{
	*buf = (char *)kmalloc(BUF_SIZE, GFP_KERNEL);
	int count = 0;
	struct process_info *proc_iter = NULL;
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		count += sprintf(*buf+count,"%d %lu\n",proc_iter->pid,proc_iter->cpu_time);
	}
    *count_ = count;
	return SUCCESS;

}


int ll_update_time(int pid,unsigned long cpu_use)
{
	printk(KERN_INFO "update_time starts for pid=%d\n",pid);
	struct process_info *proc_iter = NULL;
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		 if( proc_iter->pid == pid )
		 {
			 proc_iter->cpu_time = cpu_use;
			 printk(KERN_INFO "update time ends for pid=%d\n",pid);
		 	 return SUCCESS;
		 }
	}
	printk(KERN_INFO "update_time() pid=%d not found in the list\n",pid);
	return FAIL;
}
*/
int ll_add_to_list(int pid)
{
	printk(KERN_INFO "adding pid=%d to list\n",pid);
	if( ll_is_pid_in_list(pid) == FALSE )
	{
		struct process_info *new_proc = NULL;
		new_proc = (struct process_info *)kmalloc(sizeof(struct process_info),GFP_KERNEL);
        new_proc->pid = pid;
		//new_proc->pid = 0;
		INIT_LIST_HEAD(&new_proc->list);
		list_add_tail(&(new_proc->list),&(proc_list.list));
		printk(KERN_INFO "added pid=%d to list\n",pid);
		list_size++;
		return SUCCESS;
	}
	else
	{
		printk(KERN_INFO "pid=%d found to be duplicate. not added to the list\n",pid);
		return DUPLICATE;
	}
}

int ll_cleanup(void)
{
struct process_info *proc_iter = NULL;
	printk(KERN_INFO "linklist cleanup starts\n");
	
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		list_del(&proc_iter->list);
		kfree(proc_iter);
	}
	printk(KERN_INFO "linklist cleanup ends\n");
    return SUCCESS;
}
/**
int ll_get_pids(int **pids, int *count)
{
	//create memory for list_size of pids
	*count = list_size;
	if ( list_size > 0 )
	{
		*pids = (int *)kmalloc(sizeof(int)*list_size,GFP_KERNEL);
		struct process_info *proc_iter = NULL;
		int index = 0;
		list_for_each_entry(proc_iter,&proc_list.list,list) {
			(*pids)[index++] = proc_iter->pid;
		}
	}
	return SUCCESS;
}*/

int ll_delete_pid(int pid)
{
	struct process_info *proc_iter = NULL;
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		if (proc_iter->pid == pid )
		{
			list_del(&proc_iter->list);
			kfree(proc_iter);
			return SUCCESS;
		}
	}
	return NOT_FOUND;
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
struct process_info *proc_iter = NULL;
/*Currently adding one value into the Linked List*/
mutex_lock(&my_mutex); //locking 
ll_add_to_list(wk->number);
list_for_each_entry(proc_iter,&proc_list.list,list) {
			printk( "wk->number %d\n", proc_iter->pid );
		}
mutex_unlock(&my_mutex); //unlocking
kfree( (void*)work );
}
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
wk1->number = 1;

}

}

static struct timer_list intr_timer;

static void work_moron(unsigned long data){
	queue_work( my_wq, &wk1->work );
	mod_timer(&intr_timer,jiffies+5*HZ);
}

/* Function to Initialize Timer*/
static void initialize_timer(void){
	int wait_time = 5;
	init_timer(&intr_timer);
	intr_timer.expires = jiffies+wait_time*HZ;
	intr_timer.data = intr_timer.expires;
	intr_timer.function = work_moron;
	add_timer(&intr_timer); 
}

int init_module(void)
{
ll_initialize_list(); //initializing List
initialize_timer();//Initializing timer
create_work_queue(); //calling workqueue
//queue_work( my_wq, &wk1->work );
return 0;
}
void cleanup_module()
{
ll_cleanup();
del_timer(&intr_timer);
flush_workqueue( my_wq );
destroy_workqueue( my_wq );
}

