#pragma once

#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include "../common/protocol.h"

// Read Message state variables
extern struct ReadHeader read_header;
extern char read_payload[MAX_MESSAGE_SIZE];

// (Blockingly) Set the Read Message state variables by reading queues
bool create_read_message(void);

// define enqueue functions
#define ENTRY(name) void enqueue_##name(PAYLOAD_T(name));
READ_MESSAGES
#undef ENTRY

void init_read_queue(void);
void exit_read_queue(void);