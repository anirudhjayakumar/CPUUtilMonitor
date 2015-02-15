//////////////////////////////////////////////////
//	Author: Debjit Pal			//
//	Email: dpal2@illinois.edu		//
//////////////////////////////////////////////////


#define LINUX

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We are doing kernel work after all, no big deal */
#include <linux/proc_fs.h>	/* Necessary because we use proc fs system */
#include <linux/init.h>		/* For using the __init and __exit */
#include <asm/uaccess.h>	/* for copy_from_user and copy_to_user */
#include <linux/types.h>	/* to get ssize_t, size_t, loff_t */
#include <linux/slab.h>
#include "mp1_given.h"
#include "linklist.h"
#include "workqueue.h"

#define DEBUG 1
#define PROCFS_MAX_SIZE	1024

static unsigned long procfs_buffer_size = 0;
static unsigned long temp = 0;

typedef struct proc_dir_entry procfs_entry;
procfs_entry* newproc = NULL;
procfs_entry* newdir = NULL;
procfs_entry* newentry = NULL;

/* This is the process write function. When a process registers, this function will
 * add the PID of the registering process in the /proc/mp1/status file
 * If SUCCESSFUL returns the size of the procfs_buffer
 * If any error returns -EFAULT
 */

static ssize_t procfile_write(struct file *file, const char __user *buffer, size_t count, loff_t *data) {
	char *proc_buffer;
	long lpid;
	int pid;
	proc_buffer = (char *)kmalloc(count, GFP_KERNEL);
	if(copy_from_user(proc_buffer, buffer, count)) {
		kfree(proc_buffer);
		return -EFAULT;
	}
	proc_buffer[count] = '\0';
	strict_strtol (proc_buffer, 10,&lpid);
	pid = (int)lpid;
	ll_add_to_list(pid);
	kfree(proc_buffer);
	return count;

}

/* This is the read function. This will return a buffer of all the processes PID.
 * INPUT: buffer to store and size
 * OUTPUT: Number of bytes copied
 */

static ssize_t procfile_read (struct file *file, char __user *buffer, size_t count, loff_t *data) {
	
	char *read_buf = NULL;
	int buf_size;
	printk(KERN_INFO "PROCFILE_READ /proc/mp1/staus CALLED\n");
	//TODO: Look into temp later
	// TODO: Understand the count
    ll_generate_cpu_info_string(&read_buf,&buf_size);
	printk(KERN_INFO "read buffer: %s",read_buf);
	if(copy_to_user(buffer, read_buf,buf_size )) {
		return -EFAULT;
	}
	kfree(read_buf);
	if(count == 0) {
		temp = procfs_buffer_size;
	}
	return 0;
}
/* Associating appropriate proc file system read write function
 * to read and write into /proc/mp1/status
 * Step 4
 */

static struct file_operations proc_file_op = {
	.owner 	= THIS_MODULE,
	.read	= procfile_read,
	.write	= procfile_write,
};

/* This function will create the file directory in the proc filesystem.
 * It expects a string input which will be the name and will return the created 
 * directory and the status file
 * INPUT: procname and parent name
 * OUTPUT: none
 */
procfs_entry* proc_filesys_entries(char *procname, char *parent) {

	newdir = proc_mkdir(parent, NULL);
	if(newdir == NULL) {
		printk(KERN_ALERT "ERROR IN DIRECTORY CREATION\n");
	}
	else
		printk(KERN_ALERT "DIRECTORY CREATION SUCCESSFUL");
	
	newproc = proc_create(procname, 0666, newdir, &proc_file_op);
	if(newproc == NULL)
		printk(KERN_ALERT "ERROR: COULD NOT INITIALIZE /proc/%s/%s\n", parent, procname);

	printk(KERN_INFO "INFO: SUCCESSFULLY INITIALIZED /proc/%s/%s\n", parent, procname);
	
	return newproc;

}

/* REMOVE previously loaded entry
 * INPUT: procname and parent name
 * OUTPUT: none. 
 * Should destroy the proc entries created
 */
static void remove_entry(char *procname, char *parent) {
	remove_proc_entry(procname, newdir);
	remove_proc_entry(parent, NULL);
}


// mp1_init - Called when module is loaded
static int __init mp1_init(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE LOADING\n");
   #endif
   
   /* Step 1: Implementing a Hello World Kernel Module */
   printk("Module init called");
   /* Step 2: Implementing Proc filesystem Entries */
   
   newentry = proc_filesys_entries("status", "mp1");
   
   ll_initialize_list();
   init_workqueue();   
   printk(KERN_ALERT "MP1 MODULE LOADED\n");
   return 0;   
}

// mp1_exit - Called when module is unloaded
static void __exit mp1_exit(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
   #endif
   /* Step 2: Removing proc file system on module unload */
   remove_entry("status", "mp1");
   ll_cleanup();
   cleanup_workqueue();
   printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(mp1_init);
module_exit(mp1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group_2");
MODULE_DESCRIPTION("CS-423 MP1");
