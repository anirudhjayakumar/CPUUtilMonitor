/* this file contains the interfaces to the linklist module */
#define DUPLICATE 1000 // indicate duplicate entry in the list
                       // starts from 1000 to avoid other return
					   // codes in the kernel.
					   

/* initilalize_list() to be used before using this module. Ideally,
this should be called from the module init function. */
int ll_initilaize_list(void);



/*
add_to_list(int pid) takes in the pid of the new process.

return SUCCESS if the pid is added successfully to the list
return DUPLICATE if pid is already present in the list
return KERNEL return codes in case of other errors

*/
int ll_add_to_list(int pid);

/*
Gives a list of pids with the cpu times. This is called when the 
user-space tries to read a the status entry

buf - char array
count - totol size of data

delete buffer by the caller
*/
int ll_generate_cpu_info_string(char **buf,int *count);

/*
Update list: updates the pid with the new cpu usage(
*/
int ll_update_time(int pid, unsigned long cpu_use);

/*
cleanup(): frees all allocated memory and empties the list
*/

int ll_cleanup(void);

/* is_pid_in_list(int pid) see if pid is present in the list.
*/
int ll_is_pid_in_list(int pid);
