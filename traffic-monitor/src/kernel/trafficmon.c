#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h> // Module metadata

#include "../common/protocol.h"
#include "iplookup.h"
#include "readqueue.h"
#include "test_interface.h"
#include "procfile.h"
#include "net.h"
#include "debug.h"

MODULE_AUTHOR("blake1821");
MODULE_DESCRIPTION("Trafficmon");
MODULE_LICENSE("GPL"); // Custom init and exit methods

DEFINE_MUTEX(net4_lock);
IPStatus network_request(struct in_addr ipv4)
{
    mutex_lock(&net4_lock);
    my_debug("Entering network_request for %u\n", ipv4.s_addr);
    IPStatus status = get_ipv4_status(ipv4);
    mutex_unlock(&net4_lock);
    my_debug("Exiting network_request for %u\n", ipv4.s_addr);
    return status;
}

DEFINE_MUTEX(wq4_lock);
DECLARE_WAIT_QUEUE_HEAD(query4_wait_queue);
struct in_addr requested_ipv4;
IPStatus response4_status;
bool pending_request4_for_write = false;

struct SetStatus4Payload query_ipv4(struct in_addr ipv4)
{
    my_debug("Entering query_ipv4 for %u\n", ipv4.s_addr);
    mutex_lock(&wq4_lock);
    struct Query4Payload queryPayload = {.ipv4 = ipv4};
    enqueue_Query4(queryPayload);
    requested_ipv4 = ipv4;
    pending_request4_for_write = true;
    my_debug("About to wait in query_ipv4 for %u\n", ipv4.s_addr);
    spin_lock(&query4_wait_queue.lock);
    mutex_unlock(&wq4_lock);
    wait_event_interruptible_locked(query4_wait_queue, !pending_request4_for_write);
    // waiting...

    spin_unlock(&query4_wait_queue.lock);
    my_debug("Woke up in query_ipv4 for %u\n", ipv4.s_addr);
    mutex_lock(&wq4_lock); // we acquire the lock for cache coherence
    IPStatus status = response4_status;
    mutex_unlock(&wq4_lock);

    my_debug("Exiting in query_ipv4 for %u\n", ipv4.s_addr);
    return (struct SetStatus4Payload){
        .ipv4 = ipv4,
        .status = status};
}

// invoked by writes

void on_SetStatus4(struct SetStatus4Payload *payloads, int count)
{
    my_debug("Entering on_SetStatus4 with count=%d\n", count);
    set_ipv4_status(payloads, count);
    mutex_lock(&wq4_lock);
    if (pending_request4_for_write)
        for (int i = 0; i < count; i++)
        {
            if (payloads[i].ipv4.s_addr == requested_ipv4.s_addr)
            {
                response4_status = payloads[i].status;
                pending_request4_for_write = false;
                my_debug("Match found in on_SetStatus4 for %u\n", requested_ipv4.s_addr);
                spin_lock(&query4_wait_queue.lock);
                wake_up_locked(&query4_wait_queue);
                spin_unlock(&query4_wait_queue.lock);
                break;
            }
        }
    mutex_unlock(&wq4_lock);
}

void on_SetStatus6(struct SetStatus6Payload *payloads, int count)
{
    // todo
    return;
}

// Custom init and exit methods
static int __init custom_init(void)
{
    init_iplookup();
    init_test_interface();
    init_read_queue();
    init_procfile();
    printk(KERN_INFO "Trafficmon driver loaded.\n");
    return 0;
}

static void __exit custom_exit(void)
{
    exit_procfile();
    exit_read_queue();
    exit_test_interface();
}

module_init(custom_init);
module_exit(custom_exit);