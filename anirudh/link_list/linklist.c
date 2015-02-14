#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/rwsem.h>

#include "common.h"
#include "linklist.h"
#define BUF_SIZE 1024

// structure that stores info regarding each process

struct process_info {
	int pid;
	unsigned long cpu_time;
	struct list_head list; 
	/* list_head is added to the struct which need to be a list. This apporcah is
	*  different from conventional approach of add the struct to a list. Here it 
	*  is the reverse.
	*/
};

// read-write lock
struct rw_semaphore *sem = NULL;


// declare and intilaize the list
static struct process_info proc_list;
static int list_size = 0;

int ll_initialize_list(void)
{
	// intialize the linklist
	INIT_LIST_HEAD(&proc_list.list);
	
	// initialize the rwsem
	init_rwsem(sem);
	return SUCCESS;
}


int ll_is_pid_in_list(int pid)
{
	down_read(sem); //read lock acquire
	struct process_info *proc_iter = NULL;
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		if(proc_iter->pid == pid) 
			return TRUE;
	}
	up_read(sem); //read lock release
	return FALSE;
}

int ll_generate_cpu_info_string(char **buf, int *count_)
{
	*buf = (char *)kmalloc(BUF_SIZE, GFP_KERNEL);
	int count = 0;
	struct process_info *proc_iter = NULL;
	down_read(sem); // acquire read lock
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		count += sprintf(*buf+count,"%d %lu\n",proc_iter->pid,proc_iter->cpu_time);
	}
	up_read(sem); // release read lock
    *count_ = count;
	return SUCCESS;

}


int ll_update_time(int pid,unsigned long cpu_use)
{
	printk(KERN_INFO "update_time starts for pid=%d\n",pid);
	struct process_info *proc_iter = NULL;
	down_write(sem); // acquire write lock
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		 if( proc_iter->pid == pid )
		 {
			 proc_iter->cpu_time = cpu_use;
			 printk(KERN_INFO "update time ends for pid=%d\n",pid);
		 	 return SUCCESS;
		 }
	}
	up_write(sem); // release write lock
	printk(KERN_INFO "update_time() pid=%d not found in the list\n",pid);
	return FAIL;
}

int ll_add_to_list(int pid)
{
	printk(KERN_INFO "adding pid=%d to list\n",pid);
	if( ll_is_pid_in_list(pid) == FALSE )
	{
		struct process_info *new_proc = NULL;
		new_proc = (struct process_info *)kmalloc(sizeof(struct process_info),GFP_KERNEL);
        new_proc->pid = pid;
		new_proc->pid = 0;
		INIT_LIST_HEAD(&new_proc->list);
		down_write(sem);
		list_add_tail(&(new_proc->list),&(proc_list.list));
		up_write(sem);
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
	printk(KERN_INFO "linklist cleanup starts\n");
	struct process_info *proc_iter = NULL;
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		list_del(&proc_iter->list);
		kfree(proc_iter);
	}
	printk(KERN_INFO "linklist cleanup ends\n");
    return SUCCESS;
}

int ll_get_pids(int **pids, int *count)
{
	//create memory for list_size of pids
	*count = list_size;
	if ( list_size > 0 )
	{
		*pids = (int *)kmalloc(sizeof(int)*list_size,GFP_KERNEL);
		struct process_info *proc_iter = NULL;
		int index = 0;
		down_read(sem); // acquire read lock
		list_for_each_entry(proc_iter,&proc_list.list,list) {
			(*pids)[index++] = proc_iter->pid;
		}
		up_read(sem);
	}
	return SUCCESS;
}

int ll_delete_pid(int pid)
{
	struct process_info *proc_iter = NULL;
    down_write(sem);	
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		if (proc_iter->pid == pid )
		{
			list_del(&proc_iter->list);
			kfree(proc_iter);
			return SUCCESS;
		}
	}
	up_write(sem);
	return NOT_FOUND;
}
