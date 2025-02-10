#include "message_queue.h"
#include <stdio.h>
#include <string.h> // strcmp
#include <sys/socket.h> // send
#include <unistd.h> // close
#include <sys/wait.h> // waitpid
#include <stdlib.h> // exit

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
        fprintf(stderr, "WARNING: Queue Full - Dropping Message from (%ld.%ld)!\n", message.receive_date.tv_sec, message.receive_date.tv_usec);
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

/**
 * @brief Run a command with an argument and send the output to the client
 * @param message The message to process
 * @param response The response to send back to the client
 * @param response_size The size of the response buffer
 * @return 0 on success, 1 on failure
 */
static int run_command(const Message* message, char* response, size_t response_size) {
    int pipe_stdout[2], pipe_stderr[2];

    if (pipe(pipe_stdout) == -1 || pipe(pipe_stderr) == -1) {
        perror("Pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) { // Child process
        /* Redirect stdout / stderr */
        dup2(pipe_stdout[1], STDOUT_FILENO);
        dup2(pipe_stderr[1], STDERR_FILENO);
        close(pipe_stdout[0]);
        close(pipe_stderr[0]);

        execlp(message->command, message->command, message->value, NULL);
        perror("execlp failed"); // Only reached if execlp fails
        exit(1);
    } else { // Parent process
        /* Unused write end */
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);

        ssize_t bytes_read;

        while ((bytes_read = read(pipe_stdout[0], response, response_size - 1)) > 0) {
            response[bytes_read] = '\0';
            printf("%s", response);
            write(message->client_fd, response, bytes_read);
        }
        close(pipe_stdout[0]);

        while ((bytes_read = read(pipe_stderr[0], response, response_size - 1)) > 0) {
            response[bytes_read] = '\0';
            printf("%s", response);
            write(message->client_fd, response, bytes_read);
        }
        close(pipe_stderr[0]);

        waitpid(pid, NULL, 0); 
    }
}

/**
 * @brief Process a message and send the response back to the client
 * @param message The message to process
 * @param response The response to send back to the client
 * @param response_size The size of the response buffer
 * @return 0 on success, 1 on failure
 */
int process_message(const Message* message, char* response, int response_size) {
    if (strcmp(message->value, "Unknown Command\n") == 0) {
        send(message->client_fd, message->value, strlen(message->value), 0);
        printf("Unknown Command\n");
        close(message->client_fd);
        return 0;
    }

    /* Print the message command and receive time */
    sprintf(response, "Command: %s received at %ld.%ld epoch time\n", message->command, message->receive_date.tv_sec, message->receive_date.tv_usec);
    printf("%s", response);

    /* Given in the prompt, print is an action, where we should print the command and the time it was received and then send
       The response back to the client. */
    if (strcmp(message->command, "print") == 0 || strcmp(message->command, "Print") == 0) {
        send(message->client_fd, response, strlen(response), 0);
        close(message->client_fd);
        return 0;
    }

    /* Outside of the prompt, assume the command is a shell command. Run it in a fork and then print + send the output back as
       a response. */
    /* Note: run_command prints & sends the response itself, that way the risk of overflowing the response buffer is lower */
    run_command(message, response, response_size);
    close(message->client_fd);
    return 0;
}

int process_queue(MessageQueue* queue) {
    Message message;
    char response[2048];
    while (1) {
        if (!dequeue(queue, &message)) {
            /* Queue is empty - sleep */
            usleep(500);
        } else {
            process_message(&message, response, sizeof(response));
        }
    }
    return 0;
}
