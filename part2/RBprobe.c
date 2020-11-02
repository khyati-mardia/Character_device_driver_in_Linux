/* ----------------------------------------------- DRIVER RBprobe--------------------------------------------------
 
 Basic driver example to show skelton methods for several file operations.
 
 ----------------------------------------------------------------------------------------------------------------*/

#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/rbtree.h>
#include <linux/time.h>

#define Number_Of_Devices 1
#define DEVICE_NAME       "RBprobe"  // device name to be created and registered
#define DEVICE_CLASS      "RBrobe_class"     //the device class as shown in /sys/class
#define MAX_LENGTH 256

/* per device structure */
struct kprobe_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
} *kprobe_devp;

typedef struct rb_object {
	struct rb_node rb_node;	
	int key;
	int data ;
} rb_object_t;

static struct kprobe kp;
static dev_t kprobe_dev_number;      	 /* Allotted device number */
struct class *kprobe_dev_class;          /* Tie with the device model */
static struct device *kprobe_dev_device;

static char *user_name = "Dear John";  /* the default user name, can be replaced if a new name is attached in insmod command */

module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user

static uint64_t RDTSC(void);

char output[MAX_LENGTH];
int length = 0;

static uint64_t RDTSC()
{
  unsigned int hi, lo;
  __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

static int handler_pre(struct kprobe *p, struct pt_regs *regs){
	printk(KERN_INFO "pre_handler \n");
	printk(KERN_INFO "p->addr = 0x%p, ip=%lx, flags=0x%lx\n",p->addr,regs->ip,regs->flags);
	print_symbol(KERN_INFO "EIP is at %s\n", regs->ip);
	printk(KERN_INFO "eax: %08lx   ebx: %08lx   ecx: %08lx   edx: %08lx\n", regs->ax, regs->bx, regs->cx, regs->dx);
	printk(KERN_INFO "esi: %08lx   edi: %08lx   ebp: %08lx   esp: %08lx\n", regs->si, regs->di, regs->bp, regs->sp);
	printk(KERN_INFO "Process %s (pid: %d, threadinfo=%p task=%p)", current->comm, current->pid, current_thread_info(), current);
	
	return 0;
}

/* kprobe post_handler: called after the probed instruction is executed */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
				unsigned long flags)
{
	printk(KERN_INFO "post_handler: p->addr = 0x%p, flags = 0x%lx\n",
		p->addr, regs->flags);

	//putting data into the buffer

	int i=0;
	//dumping into buffer	
	length = 0;
	int temp_length=0;
	
	int pid = current->pid;

	temp_length += sprintf(output+temp_length,"pid = %d, ",pid);
	size_t addr = p->addr;
	temp_length += sprintf(output+temp_length,"address = 0x%x, ",addr);
	uint64_t tstamp = RDTSC();
	temp_length += sprintf(output+temp_length,"timestamp (TSC) = %lld, ",tstamp);
	

	struct rb_node *loc =  (struct rb_node *)(void *)regs->r12;
	printk (KERN_INFO "rb_node addr = %x\n",(struct rb_node *)regs->r12);
	struct rb_object *structdata  = container_of(loc, struct rb_object, rb_node);
	int keydata = structdata->key;
	temp_length += sprintf(output+temp_length,"rb_object->key = %d\n",keydata);
	length = temp_length;
	
	
}

static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr){

	printk(KERN_INFO "fault_handler: p->addr = 0x%p, trap #%d \n",p->addr,trapnr);
	return 0;
}

/*
* Open kprobe driver
*/
int kprobe_driver_open(struct inode *inode, struct file *file)
{
	printk("\n%s RBprobe is openning \n", kprobe_devp->name);

	struct kprobe_dev *kprobe_dev;


	/* Get the per-device structure that contains this cdev */
	kprobe_devp = container_of(inode->i_cdev, struct kprobe_dev, cdev);

	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = kprobe_devp;
	printk("\n%s is openning \n", kprobe_devp->name);

	return 0;
}

/*
 * Release kprobe driver
 */
int kprobe_driver_release(struct inode *inode, struct file *file)
{
	struct kprobe_dev *kprobe_devp = file->private_data;
	printk("\n%s is closing\n", kprobe_devp->name);
	
	return 0;
} 

static int register_kp(int opcode){

	kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;

	int ret_val;

	if(opcode == 0){

	printk(KERN_INFO "RB probe inserting opcode value = %d \n",opcode);
		if (kallsyms_lookup_name("rb_node_insert") == NULL){
			printk(KERN_INFO "KPROBE : NULL pointer derefference!\n");
			return -1;
		}
		printk(KERN_INFO "RBprobe lookup : %pi \n", (void **)(kallsyms_lookup_name("rb_node_insert")));
		printk(KERN_INFO "RBprobe lookup offset : %pi \n", (void **)(kallsyms_lookup_name("rb_node_insert")+0x24));
		kp.addr = (kprobe_opcode_t *)(void **)kallsyms_lookup_name("rb_node_insert") + 0x24; // read regs->r12
	}
	else
	{
		printk(KERN_INFO "RB probe rbt530_driver_read \n");
		printk(KERN_INFO "RBprobe lookup : %pi \n", (void **)(kallsyms_lookup_name("rbt530_driver_read")));
		printk(KERN_INFO "RBprobe lookup offset : %pi \n", (void **)(kallsyms_lookup_name("rbt530_driver_read")+0x168));
		kp.addr = (kprobe_opcode_t *)(void **)kallsyms_lookup_name("rbt530_driver_read") + 0x168; //read regs->r12
	}

	ret_val = register_kprobe(&kp);

	/* Registering the kprobe */
  	if (ret_val < 0) {
   	 	printk(KERN_INFO " error in register kprobe\n");
    	return -1;
  	}

	return 0;
}

/*
 * Write to kprobe driver
 */
ssize_t kprobe_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	printk(KERN_INFO "Inside KProbe write \n");

	printk(KERN_INFO "buf values %d %d", buf[0], buf[1]);
	int value = buf[0];
	int flag = buf[1];

	printk(KERN_INFO "flag = %d\n", flag);

	if(flag == 1){
		printk(KERN_INFO "Calling kprobe register \n");
                return register_kp(value);
       } else {
		printk(KERN_INFO "Calling kprobe unregister \n");
                unregister_kprobe(&kp);
                return 0;
        }
}

/*
 * Read to kprobe driver
 */
ssize_t kprobe_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
    struct kprobe_dev *kprobe_devp = file->private_data;
    int result=0;
	int i=0;
	printk(KERN_INFO "Kprobe read: ");
	for (i; i<length; i++) printk (KERN_INFO "%c",output[i]);         
	printk(KERN_INFO "kprobe data \n");
        printk(KERN_INFO "start = %d\n",length);
        int bytes_read = 0;
	while (length == 0) msleep(1000);
        
	while (bytes_read<=length) {
		printk(KERN_INFO "%c,",output[bytes_read]);
		put_user(output[bytes_read], buf++);
                printk(KERN_INFO "%c\n",buf[bytes_read]);
                bytes_read++;
        }
	length = 0;	
        return bytes_read;
}


/* File operations structure. Defined in linux/fs.h */
static struct file_operations kprobe_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= kprobe_driver_open,        /* Open method */
    .release	= kprobe_driver_release,     /* Release method */
    .write		= kprobe_driver_write,       /* Write method */
    .read		= kprobe_driver_read,        /* Read method */
};

/*
 * Driver Initialization
 */
int __init kprobe_driver_init(void)
{
	int ret;
	int time_since_boot;

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&kprobe_dev_number, 10, Number_Of_Devices, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	kprobe_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* Allocate memory for the per-device structure */
	kprobe_devp = kmalloc(sizeof(struct kprobe_dev), GFP_KERNEL);
		
	if (!kprobe_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	/* Request I/O region */
	sprintf(kprobe_devp->name, DEVICE_NAME);
	
	//memset(kprobe_devp->in, 0, 256);

	dev_t my_device = MKDEV(MAJOR(kprobe_dev_number), 10);

	/* Connect the file operations with the cdev */
	cdev_init(&kprobe_devp->cdev, &kprobe_fops);
		kprobe_devp->cdev.owner = THIS_MODULE;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&kprobe_devp->cdev, my_device, 1);
    
    if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

    /* Send uevents to udev, so it'll create /dev nodes */
	kprobe_dev_device = device_create(kprobe_dev_class, NULL, my_device , NULL, DEVICE_NAME);

	printk("RBprobe module initialized.\n'%s'\n");
	return 0;
}

/* Driver Exit */
void __exit kprobe_driver_exit(void)
{

	/* Release the major number */
	unregister_chrdev_region((kprobe_dev_number), 1);

	dev_t my_device = MKDEV(MAJOR(kprobe_dev_number), 10);

	cdev_del(&kprobe_devp->cdev);

	/* Destroy device */
	device_destroy (kprobe_dev_class, my_device);


	kfree(kprobe_devp);

	/* Destroy driver_class */
	class_destroy(kprobe_dev_class);
	printk("RBprobe module removed.\n");
}

module_init(kprobe_driver_init);
module_exit(kprobe_driver_exit);
MODULE_LICENSE("GPL v2");

