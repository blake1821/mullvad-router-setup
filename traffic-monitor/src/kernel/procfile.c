#include "procfile.h"

static struct proc_dir_entry *proc_entry;

typedef enum
{
    Header,
    Payload
} MessageState;

MessageState write_state = Header;
struct WriteHeader write_header;
char write_payloads[MAX_MESSAGE_SIZE];

// called when write() is called on the proc file
static ssize_t custom_write(struct file *file, __user const char *buffer, size_t size, loff_t *offset)
{
    switch (write_state)
    {
    case Header:
        if (size != sizeof(struct WriteHeader))
        {
            return -1;
        }
        copy_from_user(&write_header, buffer, size);
        write_state = Payload;
        return size;
    case Payload:
        if (
            size != write_header.count * get_write_payload_size(write_header.type) ||
            size > MAX_MESSAGE_SIZE)
        {
            return -1;
        }
        copy_from_user(write_payloads, buffer, size);
        write_state = Header;
        switch (write_header.type)
        {
#define ENTRY(name)                                                                    \
    case name:                                                                         \
        on_##name((struct CONCAT(name, Payload) *)write_payloads, write_header.count); \
        break;
            WRITE_MESSAGES
#undef ENTRY
        }
        return size;
    }
    return -1;
}

MessageState read_state = Header;
struct ReadHeader read_header;
char read_payload[MAX_MESSAGE_SIZE];

// called when the proc file is read() from
static ssize_t custom_read(struct file * file, char * buffer, size_t size, loff_t * offset)
{
    switch (read_state)
    {
    case Header:
        if (size != sizeof(struct ReadHeader))
        {
            return -1;
        }
        if(create_read_message())
            return -1;
        copy_to_user(buffer, &read_header, size);
        read_state = Payload;
        return size;
    case Payload:
        if (
            size != read_header.count * get_read_payload_size(read_header.type) ||
            size > MAX_MESSAGE_SIZE)
        {
            return -1;
        }
        copy_to_user(buffer, read_payload, size);
        read_state = Header;
        return size;
    }
    return -1;
}

static int custom_release(struct inode *, struct file *){
    return 0;
}


// proc file operations
static const struct proc_ops custom_fops = {
    .proc_open = nonseekable_open,
    .proc_read = custom_read,
    .proc_release = custom_release,
    .proc_write = custom_write};

void init_procfile(void)
{
    proc_entry = proc_create(TRAFFICMON_PROC_FILE, 0666, NULL, &custom_fops);
}

void exit_procfile(void)
{
    proc_remove(proc_entry);
}