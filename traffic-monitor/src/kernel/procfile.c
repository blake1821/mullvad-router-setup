#include "procfile.h"

// proc dir entries
#define ENTRY(name) static struct proc_dir_entry *proc_entry_##name;
READ_MESSAGES
WRITE_MESSAGES
#undef ENTRY

#define ENTRY(name)                                                                                     \
    static ssize_t write_##name(struct file *file, __user const char *buffer, size_t n, loff_t *offset) \
    {                                                                                                   \
        char payload[MAX_MESSAGE_SIZE];                                                                 \
        /* this is an EXTREMELY dangerous copy, but we trust the caller*/                               \
        copy_from_user(payload, buffer, n * sizeof(PAYLOAD_T(name)));                                   \
        on_##name((PAYLOAD_T(name) *)payload, n);                                                       \
        return n;                                                                                       \
    }
WRITE_MESSAGES
#undef ENTRY

static ssize_t no_op_read(struct file *f, char *buf, size_t n, loff_t *off)
{
    return -1;
}

static ssize_t no_op_write(struct file *f, const char *buf, size_t n, loff_t *off)
{
    return -1;
}

// called when the proc file is read() from

static int custom_release(struct inode *, struct file *)
{
    return 0;
}

// write proc files
#define ENTRY(name)                              \
    static const struct proc_ops fops_##name = { \
        .proc_open = nonseekable_open,           \
        .proc_read = no_op_read,                   \
        .proc_release = custom_release,          \
        .proc_write = write_##name};
WRITE_MESSAGES
#undef ENTRY

// read proc files
#define ENTRY(name)                              \
    static const struct proc_ops fops_##name = { \
        .proc_open = nonseekable_open,           \
        .proc_read = read_##name,                \
        .proc_release = custom_release,          \
        .proc_write = no_op_write};
READ_MESSAGES
#undef ENTRY

struct proc_dir_entry *proc_entry;

void init_procfile(void)
{
    proc_entry = proc_mkdir(TRAFFICMON_PROC_FILE, NULL);
    #define ENTRY(name) proc_entry_##name = proc_create(#name, 0666, proc_entry, &fops_##name);
    READ_MESSAGES
    WRITE_MESSAGES
    #undef ENTRY
}

void exit_procfile(void)
{
    proc_remove(proc_entry);
    #define ENTRY(name) proc_remove(proc_entry_##name);
    READ_MESSAGES
    WRITE_MESSAGES
    #undef ENTRY
}