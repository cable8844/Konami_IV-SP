#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

// Validate XML
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>

#include "message_queue.h"

/**
 * Bind and listen on the given address and port
 * @param address_str The address to bind to
 * @param port The port to listen on
 * @return The fd of the server socket or EXIT_FAILURE if an error occurred
 */
int open_connection(const char* address_str, int port);

/**
 * Receive data from the client given the client fd, with a preallocated buffer
 * @param client_fd The fd of the client socket
 * @param buffer The buffer to store the data. Reallocated 2 * buffer size if more space is needed
 * @param buffer_size The size of the buffer
 * @return The number of bytes received or EXIT_FAILURE if an error occurred
 */
int receive(int client_fd, char** buffer, size_t* buffer_size);

/**
 * Load the XML schema from the given file
 * @param schema_file The path to the XML schema file
 * @param schema The pointer to the schema
 * @param valid_ctxt The pointer to the validation context
 * @return EXIT_FAILURE if an error occurred
 */
int load_schema(const char* schema_file, xmlSchemaPtr* schema, xmlSchemaValidCtxtPtr* valid_ctxt);

/**
 * Validate the XML data in the buffer
 * @param buffer The buffer containing the XML data
 * @param buffer_size The size of the buffer
 * @param valid_ctxt The validation context, created from load_schema
 * @param message The message to populate
 * @return 1 if the XML is valid, 0 if invalid, EXIT_FAILURE if error occurred
 */
int get_xml(const char* buffer, size_t buffer_size, xmlSchemaValidCtxtPtr valid_ctxt, Message* message);

/**
 * Get the XML data from the client and validate it
 * @param server_fd The fd of the server socket
 * @param valid_ctxt The validation context
 * @param queue The message queue to which messages will be enqueued
 * @return EXIT_FAILURE if an error occurred
 */
int accept_connection(int server_fd, xmlSchemaValidCtxtPtr valid_ctxt, MessageQueue* queue);


/**
 * Default schema saved into memory, so that a default schema does not need to be bundled into the install.
 * Set based on example from the prompt
 */
#ifndef DEFAULT_SCHEMA
#define DEFAULT_SCHEMA                                                                                         \
    "<?xml version=\"1.0\"?>"                                                                                  \
    "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" elementFormDefault=\"qualified\">"               \
    "    <xs:element name=\"Message\">"                                                                        \
    "        <xs:complexType>"                                                                                 \
    "            <xs:sequence>"                                                                                \
    "                <xs:element name=\"Command\" type=\"xs:string\"/>"                                        \
    "                <xs:element name=\"Data\">"                                                               \
    "                    <xs:complexType>"                                                                     \
    "                        <xs:sequence>"                                                                    \
    "                            <xs:element name=\"Row\" maxOccurs=\"unbounded\">"                            \
    "                                <xs:complexType>"                                                         \
    "                                    <xs:sequence>"                                                        \
    "                                        <xs:element name=\"Description\" type=\"xs:string\"/>"            \
    "                                        <xs:element name=\"Value\" type=\"xs:string\"/>"                  \
    "                                    </xs:sequence>"                                                       \
    "                                </xs:complexType>"                                                        \
    "                            </xs:element>"                                                                \
    "                        </xs:sequence>"                                                                   \
    "                    </xs:complexType>"                                                                    \
    "                </xs:element>"                                                                            \
    "            </xs:sequence>"                                                                               \
    "        </xs:complexType>"                                                                                \
    "    </xs:element>"                                                                                        \
    "</xs:schema>"
#endif // DEFAULT_SCHEMA

#endif // SERVER_H

