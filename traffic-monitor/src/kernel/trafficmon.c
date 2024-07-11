#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h> // Module metadata

#include "../common/protocol.h"

MODULE_AUTHOR("Ruan de Bruyn");
MODULE_DESCRIPTION("Hello world driver");
MODULE_LICENSE("GPL"); // Custom init and exit methods


char databus[MAX_MESSAGE_SIZE];

static int custom_show(struct seq_file *m, void *v){
    seq_printf(m, "Hello world 2!\n");
    return 0;
}

static int custom_show_open(struct inode *inode, struct file *file){
    return single_open(file, custom_show, NULL);
}

enum WriteState{
    MessageType,
    MessagePayload
};

static ssize_t custom_write(struct file *file, __user const char *buffer, size_t size, loff_t *offset){

}

static const struct proc_ops custom_fops = {
    .proc_open = custom_show_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = custom_write
};

static struct proc_dir_entry *proc_entry;

// Custom init and exit methods
static int __init custom_init(void)
{
    init_iplookup();
    proc_entry = proc_create("helloworlddriver", 0666, NULL, &custom_fops);
    printk(KERN_INFO "Hello world driver loaded.");
    return 0;
}

static void __exit custom_exit(void)
{
    proc_remove(proc_entry);
    printk(KERN_INFO "Goodbye my friend, I shall miss you dearly...");
}
module_init(custom_init);
module_exit(custom_exit);