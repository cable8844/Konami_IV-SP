#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stddef.h>
#include <time.h>
#include <pthread.h>

#define MAX_QUEUE_SIZE 1024

typedef struct xml_message {
    char* command;
    char* description;
    char* value;
    time_t receive_date;
} Message;

typedef struct {
    Message* messages;
    size_t front;
    size_t back;

    size_t size;
    pthread_mutex_t lock;
} MessageQueue;

/**
 * Extend the queue by doubling the size
 * @param queue The message queue
 * @param message The message to enqueue
 * @return 0 on success, 1 on failure (queue is full)
 */
int enqueue(MessageQueue* queue, Message message);

/**
 * Dequeue a message from the queue
 * @param queue The message queue
 * @param message The message from the queue
 * @return 0 if a message was successfully dequeued, 1 if the queue is empty
 */
int dequeue(MessageQueue* queue, Message* message);

#endif // MESSAGE_QUEUE_H