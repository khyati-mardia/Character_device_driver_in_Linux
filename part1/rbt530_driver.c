#include <linux/rbtree.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/version.h>

#define Number_Of_Devices 2
#define ASCENDING_ORDER_CMD 0
#define DESCENDING_ORDER_CMD 1
#define READING_TREE_CMD 2
#define READ_ORDER_CMD 100
//#define DUMP_CMD 101
//#define READ_ORDER_CMD _IOW('k', 1, int)

#define DEV_NAME                 "rbt530_dev"  // device name to be created and registered
static DEFINE_MUTEX(rbt_mutex);

dev_t device_nums[Number_Of_Devices]; 

struct devices{
	char name[20];                  /* Name of device*/
	char in[2];			/* buffer for the input string */
	int cmd;
	char output[100];
	struct rb_node *current_node;
	struct rb_root rbt530;
};

/* per device structure */
struct rbt530_dev {
	struct cdev cdev[Number_Of_Devices];               /* The cdev structure */
	struct devices tree_data[Number_Of_Devices];
} *rbt530_devp;

typedef struct rb_object {
	struct rb_node rb_node;	
	int key;
	int data ;
} rb_object_t;


dev_t rbt530_dev_number;      	 /* Allotted device number */
struct class *rbt530_dev_class;          /* Tie with the device model */
static struct device *rbt530_dev_device;
//extern int errno;
char* DEVICE_NAME[] = {"rbt530_dev1", "rbt530_dev2"};

int device_index(int minor_number) {
	int i;
	for(i = 0; i < Number_Of_Devices; i++) {
		if(MINOR(device_nums[i]) == minor_number){
			return i;
		}
	}
	return -1;
}


/*
* Open rbt530 driver
*/
int rbt530_driver_open(struct inode *inode, struct file *file)
{
	struct rbt530_dev *rbt530_devp;
	int idx;
	printk(KERN_INFO "rbt530 Opening the driver");
	

	mutex_lock(&rbt_mutex);
	
	idx = device_index(iminor(file->f_path.dentry->d_inode));
	printk(KERN_INFO "Device index %d\n", idx);

	/* Get the per-device structure that contains this cdev */
	rbt530_devp = container_of(inode->i_cdev, struct rbt530_dev, cdev[idx]);

	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = rbt530_devp;
	printk("\n%s is openning \n", rbt530_devp->tree_data[idx].name);

	mutex_unlock(&rbt_mutex);
	return 0;
}

/*
 * Release rbt530 driver
 */
int rbt530_driver_release(struct inode *inode, struct file *file)
{
	struct rbt530_dev *rbt530_devp = file->private_data;

	int idx;
	mutex_lock(&rbt_mutex);

	idx = device_index(iminor(file->f_path.dentry->d_inode));
	printk(KERN_INFO "Device index %d\n", idx);
	printk("\n%s is closing\n", rbt530_devp->tree_data[idx].name);
	
	mutex_unlock(&rbt_mutex);
	return 0;
} 


//Search a node
struct rb_node *rb_node_search(struct rb_root *root, int value)
    {
        struct rb_node *rbn = root->rb_node;  /* top of the tree */

        while(rbn)
	{
	    rb_object_t *rb = container_of(rbn, rb_object_t, rb_node);

	    if (rb->key > value)
		rbn = rbn->rb_left;
	    else if (rb->key < value)
		rbn = rbn->rb_right;
	    else
		return &rb->rb_node;  /* Found it */
  	}
	return NULL;
    }

//Insert a node

int rb_node_insert(struct rb_root *root, struct rb_object *new)
    {
        struct rb_node **link = &root->rb_node, *parent=NULL;
		struct rb_object *rbo;
		

	/* Go to the bottom of the tree */
	while (*link)
	{
		int value;
	    parent = *link;
	    rbo = container_of(*link, rb_object_t, rb_node);
	    value = new->key;

	    if (rbo->key > value)
		link = &(*link)->rb_left;
	    else
		link = &(*link)->rb_right;
	}

	/* Put the new node there */
	rb_link_node(&new->rb_node, parent, link);
	rb_insert_color(&new->rb_node, root);

	return 0;
    }


/*
 * Write to rbt530 driver
 */
ssize_t rbt530_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
		int idx;
			int err = 0;
	int key, data;
	struct rb_object *rb_object_t = (struct rb_object *)kmalloc(sizeof(struct rb_object), GFP_KERNEL);
		struct rbt530_dev *rbt530_devp = file->private_data;
			idx = device_index(iminor(file->f_path.dentry->d_inode));
			
		struct rb_node *find = rb_node_search(&rbt530_devp->tree_data[idx].rbt530,key);

	printk(KERN_INFO "Inside Write function");



	mutex_lock(&rbt_mutex);


	printk(KERN_INFO "Device index %d\n", idx);
	


	err = copy_from_user(rbt530_devp->tree_data[idx].in, buf, count);

	if(err!=0)
	{
		printk( KERN_DEBUG "Error while writing data");
		mutex_unlock(&rbt_mutex);
		return -1;
	}

	key = rbt530_devp->tree_data[idx].in[0];
	data = rbt530_devp->tree_data[idx].in[1];



	rb_object_t->key = key;
	rb_object_t->data = data;

	printk(KERN_INFO "key = %d, data = %d",key, data);
	printk(KERN_INFO "Search if key exists");

	//find for the key



	if(find == 	NULL){
		printk(KERN_INFO "Key is not found in the tree");
		printk(KERN_INFO "Node is inserted %d \n",key);	
		rb_node_insert(&rbt530_devp->tree_data[idx].rbt530, rb_object_t);
	}
	if(find) {
		printk(KERN_INFO "Key is found in the tree");
		if(data != 0){
			//replace a node
			printk(KERN_INFO "Node is replaced %d",data);
			rb_replace_node(find, &rb_object_t->rb_node, &rbt530_devp->tree_data[idx].rbt530);
		}
		else if(data==0){
			//Erase a node.
			rb_erase(find,&rbt530_devp->tree_data[idx].rbt530);
			printk(KERN_INFO "Node is erased.");
		}
	}
	mutex_unlock(&rbt_mutex);
	return 0;
}

/*
 * Read to kbuf driver
 */
ssize_t rbt530_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{

	int idx;
	struct rbt530_dev *rbt530_devp = file->private_data;

	mutex_lock(&rbt_mutex);


	idx = device_index(iminor(file->f_path.dentry->d_inode));
	printk(KERN_INFO "Device index %d\n", idx);
	printk(KERN_INFO "Reading for cmd = %d",rbt530_devp->tree_data[idx].cmd);


	
	//struct rb_node *find = rbt530_devp->tree_data[idx].rbt530.rb_node;
	struct rb_root *root;
	struct rb_node* next_node;
	struct rb_node *node;
	rb_object_t* read_node;
	char keys[count] ;

	int result;
	int i = 0, ret;


	if(rbt530_devp->tree_data[idx].cmd == 0){
		printk(KERN_INFO"cmd Ascending == 0");

		if(rbt530_devp->tree_data[idx].current_node == NULL) {
			rbt530_devp->tree_data[idx].current_node = rb_first(root);
		} else {
			next_node = rb_next(rbt530_devp->tree_data[idx].current_node);
			rbt530_devp->tree_data[idx].current_node = next_node;
		}
		if(rbt530_devp->tree_data[idx].current_node) {
			rb_object_t *rbt = container_of(rbt530_devp->tree_data[idx].current_node, rb_object_t, rb_node);
			keys[0] = rbt->key;
			keys[1] = rbt->data;
			printk(KERN_ALERT"Retrived key is %d data is %d\n", rbt->key, rbt->data);
	    	mutex_unlock(&rbt_mutex);
			return copy_to_user(buf, &keys, count);
		} else {
			printk(KERN_ALERT"Next node is null\n");
			mutex_unlock(&rbt_mutex);
			return -EINVAL;
		}
	}
	else if(rbt530_devp->tree_data[idx].cmd == 1){
		printk(KERN_INFO"cmd Descending == 1");

		if(rbt530_devp->tree_data[idx].current_node == NULL) {
			rbt530_devp->tree_data[idx].current_node = rb_last(root);
		} else {
			next_node = rb_prev(rbt530_devp->tree_data[idx].current_node);
			rbt530_devp->tree_data[idx].current_node = next_node;
		}
		if(rbt530_devp->tree_data[idx].current_node) {
			rb_object_t *rbt = container_of(rbt530_devp->tree_data[idx].current_node, rb_object_t, rb_node);
			keys[0] = rbt->key;
			keys[1] = rbt->data;
			printk(KERN_ALERT"Retrived key is %d data is %d\n", rbt->key, rbt->data);
			mutex_unlock(&rbt_mutex);
			return copy_to_user(buf, &keys, count);

		} else {
			printk(KERN_ALERT"Next node is null\n");
	    	mutex_unlock(&rbt_mutex);
			return -EINVAL;
		}

	}
	else if(rbt530_devp->tree_data[idx].cmd == 2){	
		printk(KERN_INFO"cmd tree data == 2");

		for (node = rb_first(&rbt530_devp->tree_data[idx].rbt530); node; node = rb_next(node)) {
			if(i > count) {
				printk(KERN_ALERT"Tree size more than buffer of %lu\n", count);
				break;
			}
			read_node = rb_entry(node, rb_object_t, rb_node);
			keys[i] = read_node->key;
			printk(KERN_ALERT"Read: Key %d value %d\n",keys[i], read_node->data);
			++i;
		}
		ret = copy_to_user(buf, &keys, count);
		printk(KERN_ALERT"Copy_to_user ret %d\n",ret);
		if(ret == 0) {
			mutex_unlock(&rbt_mutex);
			return count;
		} else {
			mutex_unlock(&rbt_mutex);
			return ret; //Return the number of bytes failed to copy
		}
	}
	mutex_unlock(&rbt_mutex);
	return -1;

		//Read all tree data
/*
		struct rb_node *Array[100]={0};
		Array[0] = find;
		// printk("Khyati Array[0] %d",Array[0]);
		int end = 1,start=0;

		while(Array[start]!=0){
		 		
			printk(KERN_INFO "Reading tree data%d",start);
				
			rb_object_t *rb = container_of(Array[start], rb_object_t, rb_node);
			rbt530_devp->tree_data[idx].output[start] = rb->key;
			
			printk (KERN_INFO "Reading left part of a tree \n");
			if(Array[start]->rb_left){
				Array[end] = Array[start]->rb_left;
				end++;
			}

			printk(KERN_INFO "Reading right part of a tree \n");

                        if(Array[start]->rb_right){
			        Array[end] = Array[start]->rb_right;
				end++;
			}

			printk(KERN_INFO "Read value %d\n",start);
			start++;

		}
		
		printk(KERN_INFO "All tree data are collected \n");
		start--;
		printk(KERN_INFO "start = %d\n",start);
		int bytes_read = 0;
	        while (start>=0) {
			printk(KERN_INFO "%c,",rbt530_devp->tree_data[idx].output[bytes_read]);
        	        put_user(rbt530_devp->tree_data[idx].output[bytes_read], buf++);
			printk(KERN_INFO "%c\n",buf[bytes_read]);
			start--;
            bytes_read++;
        	}
		printk(KERN_INFO "All Data read done.");
		result = bytes_read;
	}
	else{
		result-1;
	}

	return result;
*/
}


//ioctl to rbt30 driver
ssize_t rbt530_driver_ioctl(struct file *file, unsigned int ioctl_num, unsigned long arg)
{

	struct rbt530_dev *rbt530_devp = file->private_data;

	int idx;
	printk(KERN_ALERT "inside IOCTL\n");
 
	mutex_lock(&rbt_mutex);
	idx = device_index(iminor(file->f_path.dentry->d_inode));
	printk(KERN_INFO "Device index inside IOCTL %d\n", idx);


 switch(ioctl_num)
 {
 	printk("cmd IOCTL %d",ioctl_num);
 	case READ_ORDER_CMD:
 	switch(arg){
 	case ASCENDING_ORDER_CMD:
 	printk("Ascending \n");
 	case DESCENDING_ORDER_CMD:
 	printk("descending \n");
 	case READING_TREE_CMD:
 	printk("read tree \n");
 		rbt530_devp->tree_data[idx].cmd = arg;
 		break;
 	default:
 	printk("error \n");
 	//errno = EINVAL;
 	//return -1;
 		mutex_unlock(&rbt_mutex);
 		return -EINVAL;
 }
}
 	printk(KERN_INFO "RB cmd  = %d \n",rbt530_devp->tree_data[idx].cmd);

 	mutex_unlock(&rbt_mutex);
	return 0;

}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations rbt530_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= rbt530_driver_open,        /* Open method */
    .release		= rbt530_driver_release,     /* Release method */
    .write		= rbt530_driver_write,       /* Write method */
    .read		= rbt530_driver_read,        /* Read method */
    .unlocked_ioctl	= rbt530_driver_ioctl,   /*IOCTL method */
};

/*
 * Driver Initialization
 */
static int __init rbt530_driver_init(void)
{
	int ret;
	printk(KERN_INFO "Initialization");
	
	//int time_since_boot;
	int i = 0;

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&rbt530_dev_number, 0, Number_Of_Devices, DEV_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	rbt530_dev_class = class_create(THIS_MODULE, DEV_NAME);

	/* Allocate memory for the per-device structure */
	rbt530_devp = kmalloc(sizeof(struct rbt530_dev), GFP_KERNEL);
		
	if (!rbt530_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}



	for(i=0; i<Number_Of_Devices; i++)
	{

	/* Request I/O region */
	sprintf(rbt530_devp->tree_data[i].name, DEV_NAME);
	memset(rbt530_devp->tree_data[i].in, 0, 256);

	dev_t my_device = MKDEV(MAJOR(rbt530_dev_number), i);
	device_nums[i]= my_device;
	/* Connect the file operations with the cdev */
	cdev_init(&rbt530_devp->cdev[i], &rbt530_fops);
		rbt530_devp->cdev[i].owner = THIS_MODULE;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&rbt530_devp->cdev[i], my_device, 1);
    
    if (ret) {
		printk("Bad cdev\n");
		return ret;
	}
    /* Send uevents to udev, so it'll create /dev nodes */
	rbt530_dev_device = device_create(rbt530_dev_class, NULL, my_device , NULL, DEVICE_NAME[i]);
	rbt530_devp->tree_data[i].rbt530=RB_ROOT;
    }
	printk("rbt530 driver initialized.\n\n");
	return 0;
}

/* Driver Exit */
static void __exit rbt530_driver_exit(void)
{
	printk(KERN_INFO "Exit the driver");
	// device_remove_file(kbuf_dev_device, &dev_attr_xxx);
	/* Release the major number */
	unregister_chrdev_region((rbt530_dev_number), 1);
	int i;

for(i=0; i<Number_Of_Devices; i++)
	{
	dev_t my_device = MKDEV(MAJOR(rbt530_dev_number), i);
	cdev_del(&rbt530_devp->cdev[i]);
	/* Destroy device */
	device_destroy (rbt530_dev_class, my_device);

}
	kfree(rbt530_devp);
	/* Destroy driver_class */
	class_destroy(rbt530_dev_class);
	printk("rbt530 driver removed.\n");
}

module_init(rbt530_driver_init);
module_exit(rbt530_driver_exit);
MODULE_LICENSE("GPL v2");