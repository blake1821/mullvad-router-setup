#pragma once

#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include "../common/protocol.h"

// define enqueue and read functions
#define ENTRY(name)                         \
    void enqueue_##name(PAYLOAD_T(name) *); \
    ssize_t read_##name(struct file *file, char *buffer, size_t size, loff_t *offset);

READ_MESSAGES
#undef ENTRY

void init_read_queue(void);
void exit_read_queue(void);