#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define CLASS_NAME    "pa3_class"
#define DEVICE_NAME   "pa3"
#define BUFFER_LENGTH 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Derya Hancock <deryahancock@knights.ucf.edu>, Jerasimos Strakosha <jstrakosha@knights.ucf.edu>, Richard Zarth <rlziii@knights.ucf.edu>");
MODULE_DESCRIPTION("A simple character-mode device driver");
MODULE_VERSION("1.0");

/* FUNCTION PROTOTYPES */
int int_module(void);
void cleanup_module(void);
static int dev_open(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int dev_release(struct inode *, struct file *);

/* GLOBAL VARIABLES */
static int majorNumber;
static int numberOfOpens = 0;
static char message[BUFFER_LENGTH] = {0};
static char receivedMessage[BUFFER_LENGTH] = {0};
//static short messageSize;
static struct class *pa3Class = NULL;
static struct device *pa3Device = NULL;
static DEFINE_MUTEX(pa3_mutex);
static int messageLen;

static struct file_operations fops =
{
    .open    = dev_open,
    .read    = dev_read,
    .write   = dev_write,
    .release = dev_release,
};

int init_module(void)
{
    printk(KERN_INFO "PA3: Initializing module.\n");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

    if (majorNumber < 0)
    {
        printk(KERN_ALERT "PA3: Failed to register a major number.\n");

        return majorNumber;
    }

    printk(KERN_INFO "PA3: Registered with major number %d.\n", majorNumber);

    pa3Class = class_create(THIS_MODULE, CLASS_NAME);

    if (IS_ERR(pa3Class))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "PA3: Failed to register a class.\n");

        return PTR_ERR(pa3Class);
    }

    printk(KERN_INFO "PA3: Device class registered.\n");

    pa3Device = device_create(pa3Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

    if (IS_ERR(pa3Device))
    {
        class_destroy(pa3Class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "PA3: Failed to create device.\n");

        return PTR_ERR(pa3Device);
    }

    mutex_init(&pa3_mutex);

    printk(KERN_INFO "PA3: Device created successfully.\n");

    return 0;
}

void cleanup_module(void)
{
    mutex_destroy(&pa3_mutex);

    device_destroy(pa3Class, MKDEV(majorNumber, 0));
    class_unregister(pa3Class);
    class_destroy(pa3Class);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "PA3: Removing module.\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "\nPA3: OPEN Full string: %s\n", message);

    if (!mutex_trylock(&pa3_mutex))
    {
        printk(KERN_ALERT "PA3: Device already in use by another process.\n");

        return -EBUSY;
    }

    numberOfOpens++;

    printk(KERN_INFO "PA3: Device has been opened %d time(s).\n", numberOfOpens);

    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int errorCount = 0;
    int i= 0;
    int stringLen= messageLen;

    printk(KERN_INFO "\nPA3: READ Full string: %s\n", message);

    // If the requested read length is more than the available space
    // Then reduce the read length to the maximum available
    // Else use the requested read length

    if (len>messageLen)
    {
        len= messageLen;
    }

    errorCount = copy_to_user(buffer, message, len);

     while (stringLen > 0)
     {
        message[i] = message[i + len];
        i = i + 1;
        stringLen--;
     }

    //Reduce the message lenght each time we read
    messageLen -= len;

    if (errorCount == 0)
    {
        printk(KERN_INFO "PA3: Sent %d characters to the user.\n", len);

        printk(KERN_INFO "PA3: READ Full string: %s\n", message);

        return len;
    } else {
        printk(KERN_INFO "PA3: Failed to send %d characters to the user.\n", errorCount);

        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int errorCount = 0;
    int j = 0;
    int startLen = messageLen;


    printk(KERN_INFO "\nPA3: WRITE Full string: %s\n", message);

    // If the requested write length is more than the available space
    // Then reduce the write length to the maximum available
    // Else use the requested write length
    if  ((len+ messageLen) > BUFFER_LENGTH)
    {
        len= BUFFER_LENGTH- messageLen;
        messageLen=BUFFER_LENGTH;
    } else {
        messageLen= messageLen + len;
    }

    errorCount = copy_from_user(receivedMessage, buffer, len);

   printk(KERN_INFO"PA3: before the loop j=%d, startLen=%d, len=%d, messageLen=%d",j,startLen,len,messageLen);

   for (j=0; j<len ;j++)
    {
        // Before writing check again if where we are writing is not bigger than the buffer
        if (startLen > BUFFER_LENGTH)
        {
            break;
        } else {
            message[startLen] = receivedMessage[j];
            startLen++;
        }
    }

    printk(KERN_INFO "PA3: Received %zu characters from the user.\n", len);

    printk(KERN_INFO "PA3: WRITE Full string: %s\n", message);

    return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&pa3_mutex);

    printk(KERN_INFO "\nPA3: RELEASE Full string: %s\n", message);

    printk(KERN_INFO "PA3: Device successfully closed.\n");

    return 0;
}
