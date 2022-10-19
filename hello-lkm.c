#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>	/* for copy_*_user */
#include <linux/sched/signal.h>
#include <linux/init_task.h>
#include <asm/perf_event.h>

#define MAX_SIZE	1024

/*static char msg[MAX_SIZE];*/

/*static int hello_lkm_show(struct seq_file *m, void *v) {*/
/*seq_printf(m, "Hello lkm!\n");*/
/*return 0;*/
/*}*/

/*static int hello_lkm_open(struct inode *inode, struct  file *file) {*/
/*return single_open(file, hello_lkm_show, NULL);*/
/*}*/
void read_cr4(void) {
#ifdef __x86_64__
	u64 cr0, cr2, cr3, cr4;
	__asm__ __volatile__ (
	    "mov %%cr0, %%rax\n\t"
	    "mov %%eax, %0\n\t"
	    "mov %%cr2, %%rax\n\t"
	    "mov %%eax, %1\n\t"
	    "mov %%cr3, %%rax\n\t"
	    "mov %%eax, %2\n\t"
	    "mov %%cr4, %%rax\n\t"
	    "mov %%eax, %3\n\t"
	: "=m" (cr0), "=m" (cr2), "=m" (cr3), "=m" (cr4)
	: /* no input */
	: "%rax"
	);
#elif defined(__i386__)
	u32 cr0, cr2, cr3;
	__asm__ __volatile__ (
	    "mov %%cr0, %%eax\n\t"
	    "mov %%eax, %0\n\t"
	    "mov %%cr2, %%eax\n\t"
	    "mov %%eax, %1\n\t"
	    "mov %%cr3, %%eax\n\t"
	    "mov %%eax, %2\n\t"
	: "=m" (cr0), "=m" (cr2), "=m" (cr3)
	: /* no input */
	: "%eax"
	);
#endif
	printk(KERN_INFO "cr0 = 0x%llx\n", cr0);
	printk(KERN_INFO "cr2 = 0x%llx\n", cr2);
	printk(KERN_INFO "cr3 = 0x%llx\n", cr3);
	printk(KERN_INFO "cr4 = 0x%llx\n", cr4);
	return;
}

u64 get_pmc(void){
	unsigned int a=0, d=0;
	int ecx=(1<<30)+1; //What counter it selects?
	__asm __volatile("rdpmc" : "=a"(a), "=d"(d) : "c"(ecx));
	return ((long long)a) | (((long long)d) << 32);
}




static void traversal_thread_group(struct task_struct * tsk){
	struct task_struct * curr_thread = NULL;
	struct pt_regs * curr_regs = NULL;
	unsigned long tg_offset = offsetof(struct task_struct, thread_group);

	curr_thread = (struct task_struct *) (((unsigned long)tsk->thread_group.next) - tg_offset);
	/*if (curr_thread == tsk){*/
	/*printk("Thread Group is empty!");*/
	/*return;*/
	/*}*/
	while (curr_thread != tsk){
		curr_regs = task_pt_regs(curr_thread);
		printk("\t\tTHREAD TSK=%llx\tPID=%d\tSTACK=%llx \tCOMM=%s\tMM=%llx\tACTIVE_MM=%llx\tUSER_SP=%lx\tUSER_PC=%lx\n", 
				(u64)curr_thread, curr_thread->pid, (u64)curr_thread->stack,
				curr_thread->comm, (u64)curr_thread->mm, (u64)curr_thread->active_mm,
				curr_regs->sp, curr_regs->ip);
		curr_thread = (struct task_struct *) (((unsigned long)curr_thread->thread_group.next) - tg_offset);
	}
}

static void traversal_process(void) {
	struct task_struct * tsk = NULL;
	struct pt_regs * curr_regs = NULL;

	
	traversal_thread_group(&init_task);
	for_each_process(tsk){
		curr_regs = task_pt_regs(tsk);
		printk("PROCESS\tTHREAD TSK=%llx\tPID=%d\tSTACK=%llx \tCOMM=%s\tMM=%llx\tACTIVE_MM=%llx\tUSER_SP=%lx\tUSER_PC=%lx\n", 
				(u64)tsk, tsk->pid, (u64)tsk->stack, tsk->comm,
				(u64)tsk->mm, (u64)tsk->active_mm, 
				curr_regs->sp, curr_regs->ip);
		traversal_thread_group(tsk);
	}
}

ssize_t proc_read(struct file *filp,char __user *buf,size_t count,loff_t *offp ) 
{

	traversal_process();
	/*sprintf(msg, "%s", "hello lkm is read");*/
	/*printk("lkm_read:%s\n", msg);*/
	/*read_cr4();*/
	/*printk("Cycle Count: %llx\n", get_pmc());*/
	return 0;
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

static const struct proc_ops hello_lkm_fops = {
	/*.owner = THIS_MODULE,*/
	/*.open = hello_lkm_open,*/
	.proc_read = proc_read,
	.proc_write = proc_write,
	/*.llseek = seq_lseek,*/
	/*.release = single_release,*/
};

static int __init hello_lkm_init(void) {
	proc_create("hello_lkm", 0666, NULL, &hello_lkm_fops);
	return 0;
}

static void __exit hello_lkm_exit(void) {
	remove_proc_entry("hello_lkm", NULL);
}

MODULE_LICENSE("GPL");
module_init(hello_lkm_init);
module_exit(hello_lkm_exit);
