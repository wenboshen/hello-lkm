#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>	/* for copy_*_user */
#include <linux/sched/signal.h>

#define MAX_SIZE	1024

static char msg[MAX_SIZE];

/*static int hello_proc_show(struct seq_file *m, void *v) {*/
/*seq_printf(m, "Hello proc!\n");*/
/*return 0;*/
/*}*/

/*static int hello_proc_open(struct inode *inode, struct  file *file) {*/
/*return single_open(file, hello_proc_show, NULL);*/
/*}*/


/*ssize_t proc_read(struct file *filp,char __user *buf,size_t count,loff_t *offp ) */
/*{*/
/*sprintf(msg, "%s", "hello proc is read");*/
/*printk("proc_read:%s\n", msg);*/
/*return copy_to_user(buf, msg, strlen(msg));*/
/*}*/


static void traversal_thread_group(struct task_struct * tsk){
	struct task_struct * curr_thread = NULL;
	unsigned long tg_offset = offsetof(struct task_struct, thread_group);

	curr_thread = (struct task_struct *) (((unsigned long)tsk->thread_group.next) - tg_offset);
	/*if (curr_thread == tsk){*/
	/*printk("Thread Group is empty!");*/
	/*return;*/
	/*}*/
	while (curr_thread != tsk){
		printk("\t\tTHREAD TSK=%llx\tPID=%d\tSTACK=%llx \tCOMM=%s\tMM=%llx\tACTIVE_MM=%llx\n", 
				(u64)curr_thread, curr_thread->pid, (u64)curr_thread->stack,
				curr_thread->comm, (u64)curr_thread->mm, (u64)curr_thread->active_mm);
		curr_thread = (struct task_struct *) (((unsigned long)curr_thread->thread_group.next) - tg_offset);
	}
}

static void traversal_process(void) {
	struct task_struct * tsk = NULL;

	
	traversal_thread_group(&init_task);
	for_each_process(tsk){
		printk("PROCESS\tTHREAD TSK=%llx\tPID=%d\tSTACK=%llx \tCOMM=%s\tMM=%llx\tACTIVE_MM=%llx\n", 
				(u64)tsk, tsk->pid, (u64)tsk->stack, tsk->comm,
				(u64)tsk->mm, (u64)tsk->active_mm);
		traversal_thread_group(tsk);
	}
}

ssize_t proc_write(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	int ret = 0;
	if (count > MAX_SIZE){
		count =  MAX_SIZE;
	}

	/*ret = copy_from_user(msg,buf,count);*/
	traversal_process();
	printk("proc_write: copy_from_user return =%d\n", ret);
	return count;
}

static const struct file_operations hello_proc_fops = {
	/*.owner = THIS_MODULE,*/
	/*.open = hello_proc_open,*/
	/*.read = proc_read,*/
	.write = proc_write,
	/*.llseek = seq_lseek,*/
	/*.release = single_release,*/
};

static int __init hello_proc_init(void) {
	proc_create("hello_proc", 0666, NULL, &hello_proc_fops);
	return 0;
}

static void __exit hello_proc_exit(void) {
	remove_proc_entry("hello_proc", NULL);
}

MODULE_LICENSE("GPL");
module_init(hello_proc_init);
module_exit(hello_proc_exit);
