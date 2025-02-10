#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h> // tolower

int open_connection(const char* address_str, int port) {
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed, %s\n");
        return EXIT_FAILURE;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(address_str);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Listening on %s:%d\n", address_str, port);

    return server_fd;
}

int receive(int client_fd, char** buffer, size_t* buffer_size) {
    int bytes_recvd;
    ssize_t total_bytes_recvd = 0;
    while (1) {
        bytes_recvd = recv(client_fd, *buffer, *buffer_size, 0);
        if (bytes_recvd < 0) {
            perror("recv");
            return EXIT_FAILURE;
        }
        if (bytes_recvd == 0) {
            break;
        }

        total_bytes_recvd += bytes_recvd;

        if (bytes_recvd == *buffer_size) {
            *buffer_size *= 2;
            *buffer = realloc(*buffer, *buffer_size);
            if ((*buffer) == NULL) {
                perror("realloc");
                return EXIT_FAILURE;
            }
        }
    }

    /* Make sure the buffer is null terminated */
    (*buffer)[total_bytes_recvd] = '\0';

    return total_bytes_recvd;
}

int load_schema(const char* schema_file, xmlSchemaPtr* schema, xmlSchemaValidCtxtPtr* valid_ctxt) {
    xmlSchemaParserCtxtPtr parser_ctxt;
    if (schema_file != NULL) {
        parser_ctxt = xmlSchemaNewParserCtxt(schema_file);
    } else {
        parser_ctxt = xmlSchemaNewMemParserCtxt(DEFAULT_SCHEMA, strlen(DEFAULT_SCHEMA));
    }

    if (!parser_ctxt) {
        fprintf(stderr, "Could not create schema parser context\n");
        return EXIT_FAILURE;
    }

    *schema = xmlSchemaParse(parser_ctxt);
    xmlSchemaFreeParserCtxt(parser_ctxt);

    if (!(*schema)) {
        fprintf(stderr, "Failed to load XML Schema\n");
        return EXIT_FAILURE;
    }

    *valid_ctxt = xmlSchemaNewValidCtxt(*schema);
    if (!(*valid_ctxt)) {
        fprintf(stderr, "Failed to create schema validation context\n");
        xmlSchemaFree(*schema);
        return EXIT_FAILURE;
    }
}

/**
 * Parse the XML data buffer and populate the Message struct
 * @param root_element The root element of the XML document
 * @param message The message to populate
 */
static void parse_xml(xmlNodePtr root_element, Message* message) {
    assert(root_element != NULL);
    char* data = NULL;

    for (xmlNodePtr node = root_element->children; node != NULL; node = node->next) {
        if (node->type != XML_ELEMENT_NODE) {
            continue;
        }

        char* name = (char*)node->name;
        name[0] = tolower(name[0]);
        if (strcmp(name, "command") == 0) {
            message->command = (char*)xmlNodeGetContent(node);
            #ifdef DEBUG
            printf("DEBUG: Command: %s\n", message->command);
            #endif
        } else if (strcmp(name, "description") == 0) {
            message->description = (char*)xmlNodeGetContent(node);
            #ifdef DEBUG
            printf("DEBUG: Description: %s\n", message->description);
            #endif
        } else if (strcmp(name, "value") == 0) {
            message->value = (char*)xmlNodeGetContent(node);
            #ifdef DEBUG
            printf("DEBUG: Value: %s\n", message->value);
            #endif
        } else if (strcmp(name, "data") == 0 || strcmp(name, "row") == 0) {
            #ifdef DEBUG
            printf("DEBUG: Recursing down to lower layer of %s\n", name);
            #endif
            parse_xml(node, message);
        }
        #ifdef DEBUG
        else {
            fprintf(stderr, "DEBUG: Unknown element: %s\n", node->name);
        }
        #endif
    }

}

/**
 * Validate the XML data buffer. Buffer must be null terminated.
 * @param xml_data The char* XML data
 * @param buffer_size The length of the XML data
 * @return 1 if the XML is valid, 0 if invalid, EXIT_FAILURE if error occurred
 */
int get_xml(const char* xml_data, size_t xml_length, xmlSchemaValidCtxtPtr valid_ctxt, Message* message) {
    assert(xml_data[xml_length] == '\0');

    /* Parse the XML from memory. Use "noname.xml" in this case to represent the file name */
    xmlDocPtr doc = xmlReadMemory(xml_data, xml_length, "noname.xml", NULL, 0);
    if (doc == NULL) {
        message->value = "Unknown Command\n";
        fprintf(stderr, "Failed to parse document\n");
        return EXIT_FAILURE;
    }

    int valid = xmlSchemaValidateDoc(valid_ctxt, doc);
    if (valid != 0) {
        xmlFreeDoc(doc);
        return valid == 0;
    }

    xmlNodePtr root_element = xmlDocGetRootElement(doc);
    if (root_element == NULL) {
        message->value = "Unknown Command\n";
        fprintf(stderr, "Empty XML document\n");
        return EXIT_FAILURE;
    }

    parse_xml(root_element, message);
    xmlFreeDoc(doc);
    return valid == 0;
}

int accept_connection(int server_fd, xmlSchemaValidCtxtPtr valid_ctxt, MessageQueue* queue) {
    int new_socket;
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    size_t buffer_size = 1024;
    char* buffer = (char*)malloc(buffer_size);
    if (buffer == NULL) {
        perror("malloc");
        close(server_fd);
        return EXIT_FAILURE;
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addr_len)) < 0) {
            perror("accept");
            continue;
        }

        /* Received the data into the buffer */
        ssize_t bytes_recvd;
        if ((bytes_recvd = receive(new_socket, &buffer, &buffer_size)) < 0) {
            close(new_socket);
            return EXIT_FAILURE;
        }

        /* Get the relevant data out of the xml buffer we have received */
        Message message = {0};
        message.client_fd = new_socket;
        /* Set the receive time of the message */
        if (gettimeofday(&message.receive_date, NULL) < 0) {
            perror("gettimeofday");
            close(message.client_fd);
            return EXIT_FAILURE;
        }

        get_xml(buffer, bytes_recvd, valid_ctxt, &message);

        /* We've gotten the message, now add it to the processing queue */
        int retry = 0;
        while (enqueue(queue, message)) {
            usleep(500);
            retry++;
            if (retry > 10) {
                fprintf(stderr, "ERROR: Could not enqueue message after 10 retries\n");
                break;
            }
        }

        usleep(5);
    }

    return new_socket;
}
