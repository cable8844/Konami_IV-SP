#include "message_queue.h"
#include <stdio.h>

#if 0
// Not implemented, return 1 if enqueue fails to try again.
static void extend_queue(MessageQueue* queue)
#endif

int enqueue(MessageQueue* queue, Message message) {
    int ret = 0;
    pthread_mutex_lock(&queue->lock);
    if (queue->size < MAX_QUEUE_SIZE) {
        queue->messages[queue->back] = message;
        queue->back = (queue->back + 1) % MAX_QUEUE_SIZE;
        queue->size++;
    } else {
        ret = 1;
        fprintf(stderr, "WARNING: Queue Full - Dropping Message from (%s)!\n", message.receive_date);
    }
    pthread_mutex_unlock(&queue->lock);
    return ret;
}

int dequeue(MessageQueue* queue, Message* message) {
    pthread_mutex_lock(&queue->lock);
    if (queue->size == 0) {
        /* Empty queue */
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }
    *message = queue->messages[queue->front];
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    queue->size--;
    pthread_mutex_unlock(&queue->lock);
    return 1;
}
