#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h> // Module metadata

#include "net.h"

static ssize_t custom_read(struct file *file, __user char *buffer, size_t size, loff_t *offset){
    return 0;
}

static ssize_t custom_write(struct file *file, __user const char *buffer, size_t size, loff_t *offset){
    if(size == 4){
        struct in_addr ipv4;
        copy_from_user(&ipv4, buffer, size);
        return network_request(ipv4);
    }
    return -1;
}

static int custom_release(struct inode *, struct file *){
    return 0;
}

static const struct proc_ops custom_fops = {
    .proc_open = nonseekable_open,
    .proc_read = custom_read,
    .proc_release = custom_release,
    .proc_write = custom_write
};

static struct proc_dir_entry *test_proc_entry;

void init_test_interface(void){
    test_proc_entry = proc_create(TEST_INTERFACE_PROC_FILE, 0666, NULL, &custom_fops);
}

void exit_test_interface(void){
    proc_remove(test_proc_entry);
}