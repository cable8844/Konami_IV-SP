#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <argp.h>
#include <errno.h>

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

int main(int argc, char** argv) {
    /* Parse our arguments */
    struct arguments arguments;
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

    accept_connection(server_fd, valid_ctxt);



    return 0;
}