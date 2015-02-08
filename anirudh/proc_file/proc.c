/* This file contains code to create a proc entry (directories and files)
*  and clean them up when the module unloads (delete the proc entries)
*/

#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/proc_fs.h>   // included for the use of proc fs
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DIR_NAME "mp1"
#define FILE_NAME "status"
#define SUCCESS 0
#define PID_SIZE 10


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anirudh Jayakumar");
MODULE_DESCRIPTION("Proc read/write entries");

struct proc_dir_entry *proc_dir_mp; //directory mp
struct proc_dir_entry *proc_file_mp_status; // file mp/status

//LL API: make sure the returned value is deleted
// dummy implementation
char *generate_string_with_cpu_info(void)
{
	char *ptr = kmalloc(10, GFP_KERNEL);
	strcpy(ptr,"Anirudh\0");
	return ptr;
}

// LL API
int  add_to_list(char *buf); // function responsible for freeing buf

char *cpu_info_string = NULL;
int cpu_info_string_size = 0;
char pid_receive[PID_SIZE];

int read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{

	cpu_info_string = generate_string_with_cpu_info();
	cpu_info_string_size = strlen(cpu_info_string);
	// we assume this case for our requirements. If the size of the output
	// is huge, then we may have to consider sequence files technique
	printk(KERN_INFO "%s\n", cpu_info_string);
	printk(KERN_INFO "count size = %d, string size = %d \n", count, cpu_info_string_size);
	if( count > cpu_info_string_size)   count = cpu_info_string_size;
	copy_to_user(buf,cpu_info_string, count);
	if(count == 0 ) 
	{
		kfree(cpu_info_string);
		cpu_info_string = NULL;
		cpu_info_string_size = 0;
	}
    return count; // the file will be continuously read till count is 0
}

int write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	if ( count > PID_SIZE )
	{
		printk(KERN_INFO "Invalid pid entered\n");
		return -ENOSPC;
	}

	copy_from_user(pid_receive,buf,count);
	return count;
} 

// module init function
static int __init proc_init(void)
{
       int retval = 0;
       printk(KERN_INFO "Creating proc entries\n");
       proc_dir_mp = proc_mkdir(DIR_NAME, NULL);
       if (proc_dir_mp == NULL) {   // directory creation failed
       	   retval = -ENOMEM;
           goto out;
       }
       
	   struct file_operations status_fops = {
          read: read_proc,
		  write: write_proc
	   };

       proc_file_mp_status = proc_create(FILE_NAME, 0, proc_dir_mp, &status_fops);
     
       if (proc_file_mp_status == NULL) {   // file creation failed
           retval = -ENOMEM;
           goto badfile;
       }

       printk(KERN_INFO "Proc entries created\n");
       return retval;    // Non-zero return means that the module couldn't be loaded.

badfile:
       remove_proc_entry(DIR_NAME, NULL);
out:
       return retval;
}

//module destructoe
static void __exit proc_cleanup(void)
{
       printk(KERN_INFO "Cleaning up proc entries that were created my the module.\n");
       remove_proc_entry(FILE_NAME, proc_dir_mp);
       remove_proc_entry(DIR_NAME, NULL);
       printk(KERN_INFO "Finished cleaning proc entries\n");
}

module_init(proc_init);
module_exit(proc_cleanup);
