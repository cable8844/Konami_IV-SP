#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <argp.h>
#include <errno.h>

/* Note: For some reason VSCode doesn't like conio and signal handler, so I'm leaving it out for now. */
#if 0
#include <conio.h> // Detect if 'q' was pressed to exit
#include <signal.h>

int signal_caught;
#endif

#include "server.h"

const char* argp_program_version = "Konami Simple Server 0.0.1";
const char* argp_program_bug_address = "<calebfarrand@gmail.com>";

/* Program documentation. */
static char doc[] = "A simple server program that listens for XML messages, adds them to a queue and processes them.";

/* A description of the arguments we accept. */
static char args_doc[] = "";

/* The options we understand. */
static struct argp_option options[] = {
    {"address", 'a', "ADDRESS", 0, "Address to listen on (default: 127.0.0.1)"},
    {"port", 'p', "PORT", 0, "Port to listen on (default: 5000)"},
    {"schema", 's', "SCHEMA", 0, "Path to the XML schema file (default: load from memory)"},
    {0}
};

/* Used by main to communicate with parse_opt. */
struct arguments {
    char* address;
    int port;
    char* schema;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state* state) {
    struct arguments* arguments = state->input;

    switch (key) {
        case 'a':
            arguments->address = arg;
            break;
        case 'p':
            arguments->port = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            return 0;
        case ARGP_KEY_END:
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    if (arguments->port < 0 || arguments->port > 65535) {
        argp_failure(state, 1, 0, "Port must be between 0 and 65535");
        return ERANGE;
    }
    if (arguments->address == NULL) {
        argp_failure(state, 1, 0, "Address must be provided");
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

/* Note: For some reason, VSCode does not like signals and conio.h, so I'm removing it for now. */
#if 0
/**
 * @brief Detect if q was hit
 * @return true if q was hit, false otherwise
 */
static bool was_q_pressed() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN_FILENO, &fds)) {
        char c = fgetc(stdin);
        if (c == 'q') {
            return true;
        }
    }

    return false;
}

/**
 * @brief Signal handler function
 * @param signum Signal number
 */
static void signal_handler(int signum)
{
    switch(signum) {
        case SIGHUP:
        case SIGINT:
        case SIGQUIT:
        case SIGABRT:
        case SIGTERM:
            signal_caught = 1;
            break;
    }
}

/**
 * @brief Signal handler initialization.
 */
static void set_signal_handler()
{
    signal_caught = 0;
    struct sigaction action;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGABRT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}
#endif


typedef struct {
    int server_fd;
    xmlSchemaValidCtxtPtr valid_ctxt;
    MessageQueue* queue;
} AcceptConnectionParams;

/**
 * @brief Accept a connection from a client
 * @param server_fd The server file descriptor
 */
static void pthread_accept_connection_wrapper(void* params) {
    AcceptConnectionParams* accept_params = (AcceptConnectionParams*)params;
    accept_connection(accept_params->server_fd, accept_params->valid_ctxt, accept_params->queue);
}

int main(int argc, char** argv) {
    /* Note: For some reason, VSCode does not like signals and conio.h, so I'm removing it for now. */
    #if 0
    set_signal_handler();
    #endif
    int ret = 0;
    /* Parse our arguments */
    struct arguments arguments;
    /* Set defaults */
    arguments.address = "127.0.0.1";
    arguments.port = 5000;
    arguments.schema = NULL;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    int server_fd;
    if ((server_fd = open_connection(arguments.address, arguments.port)) < 0) {
        exit(server_fd);
    }

    xmlSchemaPtr schema = NULL;
    xmlSchemaValidCtxtPtr valid_ctxt = NULL;
    if (load_schema(arguments.schema, &schema, &valid_ctxt) < 0) {
        exit(EXIT_FAILURE);
    }


    MessageQueue* queue = (MessageQueue*)calloc(1, sizeof(MessageQueue));
    if (queue == NULL) {
        perror("calloc");
        ret = EXIT_FAILURE;
        goto exit;
    }
    queue->messages = (Message*)calloc(1, MAX_QUEUE_SIZE * sizeof(Message));
    if (queue->messages == NULL) {
        perror("calloc");
        ret = EXIT_FAILURE;
        goto exit;
    }

    AcceptConnectionParams accept_params = {server_fd, valid_ctxt, queue};
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, NULL, (void* (*)(void*))pthread_accept_connection_wrapper, (void*)&accept_params) != 0) {
        perror("pthread_create");
        ret = EXIT_FAILURE;
        goto exit;
    }
    
    pthread_t process_thread;
    if (pthread_create(&process_thread, NULL, (void* (*)(void*))process_queue, (void*)queue) != 0) {
        perror("pthread_create");
        ret = EXIT_FAILURE;
        goto exit;
    }

    while (1) {
        /* Note: For some reason, VSCode does not like signals and conio.h, so I'm removing it for now. */
        #if 0
        if (was_q_pressed() || signal_caught) {
            break;
        }
        #endif
        usleep(100);
    }

exit:
    /* Cleanup */
    if (schema) {
        xmlSchemaFree(schema);
    }
    if (queue) {
        if (queue->messages) {
            free(queue->messages);
        }
        free(queue);
    }

    close(server_fd);
    return ret;
}