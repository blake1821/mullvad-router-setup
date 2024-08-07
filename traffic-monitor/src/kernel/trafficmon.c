#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h> // Module metadata

#include "../common/protocol.h"
#include "../common/iplookup.h"
#include "readqueue.h"
#include "procfile.h"
#include "nethooks.h"
#include "debug.h"

MODULE_AUTHOR("blake1821");
MODULE_DESCRIPTION("Trafficmon");
MODULE_LICENSE("GPL"); // Custom init and exit methods


// Custom init and exit methods
static int __init custom_init(void)
{
    init_iplookup();
    init_read_queue();
    init_procfile();
    init_nethooks();
    printk(KERN_INFO "Trafficmon driver loaded.\n");
    return 0;
}

static void __exit custom_exit(void)
{
    exit_procfile();
    exit_read_queue();
    exit_nethooks();
}

module_init(custom_init);
module_exit(custom_exit);