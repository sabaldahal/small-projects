#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "lkm_module"
#define EXAMPLE_MSG "Hello from the lkm_module!\n"
#define MSG_BUFFER_LEN 40




MODULE_AUTHOR("Sabal Dahal");
MODULE_DESCRIPTION("A simple character driver that does only one thing");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");



static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int major_num;



static int device_open_count = 0;  


static char msg_buffer[MSG_BUFFER_LEN];   

static char *msg_ptr;                       

//I USED A STATIC GLOBAL VARIABLE THAT STORES THE NUMBER OF MESSAGES PRODUCED
//THIS VARIABLE IS ACCESSIBLE BY ALL "HANDLER METHODS" AND 
//ONLY WITHIN THIS SOURCE FILE
//IT IS SET TO ZERO AT THE TIME THIS MODULE IS INITIALIZED
static int printCount = 0;

static struct file_operations file_ops =
   { .read = device_read,
     .write = device_write,
     .open = device_open,
     .release = device_release
   };


static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset)
  { int bytes_read = 0;
	
	//CHECK THE PRINT COUNT AND IF IT IS GREATER THAN 
	//OR EQUAL TO 10, RETURN bytes_read
	//IN THIS CASE IT WILL RETURN 0 INDICATING THAT IT IS
	//END-OF-FILE, AFTER WHICH THE CONSUMER PROCESS WILL STOP CALLING IT
	if(printCount >= 10) return bytes_read;
	
        if (*msg_ptr == 0) msg_ptr = msg_buffer;
	

        while (len && *msg_ptr)
           { put_user(*(msg_ptr++), buffer++);
             len--;
             bytes_read++;
           }
           
        //INCREMENT THE printCount VALUE BY 1
	//THIS INDICATES THE NUMBER OF TIMES THE MESSAGE HAS BEEN PRODUCED
	printCount++;
	
        return bytes_read;
   }


static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset)
  { printk(KERN_ALERT "Write operation is not supported in module lkm_example.\n");
    return -EINVAL;
   }


static int device_open(struct inode *inode, struct file *file)
   { 
     if (device_open_count) return -EBUSY;
     device_open_count++;
     
     //RESET THE PRINT COUNT TO 0 EVERY TIME THE DEVICE IS OPENED
     //THIS WILL ENSURE THAT THE MESSAGE WILL BE PRODUCED 10 TIMES EVERY TIME IT IS OPENED AGAIN
     printCount = 0;
     
     try_module_get(THIS_MODULE);
     return 0;
   }


static int device_release(struct inode *inode, struct file *file)
  { device_open_count--;
    module_put(THIS_MODULE);
    return 0;
  }



static int __init lkm_example_init(void)
   { 
     strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);

     msg_ptr = msg_buffer;


     major_num = register_chrdev(0, "lkm_example", &file_ops);

     if (major_num < 0)
        { printk(KERN_ALERT "Could not register device: %d\n", major_num);
          return major_num; }
     else
        { printk(KERN_ALERT "lkm_module loaded with device major number %d\n",
                 major_num);
          return 0;
        }
   }



static void __exit lkm_example_exit(void)
  { unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_ALERT "the lkm_module has left the building...\n");
  }



module_init(lkm_example_init);
module_exit(lkm_example_exit);