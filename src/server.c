#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

// Validate XML
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>

xmlSchemaPtr schema = NULL;
xmlSchemaValidCtxtPtr valid_ctxt = NULL;

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

    printf("Listening on %s:%d\n", address, port);

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
            if (*buffer == NULL) {
                perror("realloc");
                return EXIT_FAILURE;
            }
        }
    }

    *buffer[total_bytes_recvd] = '\0'; // Make sure the buffer is null terminated

    return total_bytes_recvd;
}

int load_schema(const char* schema_file) {
    xmlSchemaParserCtxtPtr parser_ctxt;
    if (schema_file != NULL) {
        parser_ctxt = xmlSchemaNewParserCtxt(schema_file);
    } else {
        parser_ctxt = xmlSchemaNewMemParserCtxt(default_schema, strlen(default_schema));
    }
    
    if (!parser_ctxt) {
        fprintf(stderr, "Could not create schema parser context\n");
        return EXIT_FAILURE;
    }

    schema = xmlSchemaParse(parser_ctxt);
    xmlSchemaFreeParserCtxt(parser_ctxt);

    if (!schema) {
        fprintf(stderr, "Failed to load XML Schema\n");
        return EXIT_FAILURE;
    }

    valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (!valid_ctxt) {
        fprintf(stderr, "Failed to create schema validation context\n");
        xmlSchemaFree(schema);
        return EXIT_FAILURE;
    }
}

/**
 * Validate the XML data buffer. Buffer must be null terminated.
 * @param xml_data The char* XML data
 * @param buffer_size The length of the XML data
 * @return 1 if the XML is valid, 0 if invalid, EXIT_FAILURE if error occurred
 */
int validate_xml(const char* xml_data, size_t xml_length) {
    assert(xml_data[xml_length] == '\0');

    // Parse the XML from memory. Use "noname.xml" in this case to represent the file name
    xmlDocPtr doc = xmlReadMemory(xml_data, xml_length, "noname.xml", NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse document\n");
        return EXIT_FAILURE;
    }

    int valid = xmlSchemaValidateDoc(valid_ctxt, doc);
    if (valid == 0) {
        printf("%s", xml_data);
    } else {
        printf("XML validation failed.\n");
    }

    xmlFreeDoc(doc);
    return valid == 0;
}

int get_xml(int server_fd) {
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

        ssize_t bytes_recvd;
        if ((bytes_recvd = receive(new_socket, &buffer, &buffer_size)) < 0) {
            close(new_socket);
            return EXIT_FAILURE;
        }

        validate_xml(buffer, bytes_recvd);
    }

    return new_socket;
}