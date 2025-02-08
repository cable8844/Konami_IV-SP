#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

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
 * Validate the XML data in the buffer
 * @param buffer The buffer containing the XML data
 * @param buffer_size The size of the buffer
 * @return 1 if the XML is valid, 0 if invalid, EXIT_FAILURE if error occurred
 */
int validate_xml(const char* buffer, size_t buffer_size);

int get_xml(int server_fd);

const char* default_schema = 
    "<?xml version=\"1.0\"?>"
    "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" elementFormDefault=\"qualified\">"
    "    <xs:element name=\"Message\">"
    "        <xs:complexType>"
    "            <xs:sequence>"
    "                <xs:element name=\"Command\" type=\"xs:string\"/>"
    "                <xs:element name=\"Data\">"
    "                    <xs:complexType>"
    "                        <xs:sequence>"
    "                            <xs:element name=\"Row\" maxOccurs=\"unbounded\">"
    "                                <xs:complexType>"
    "                                    <xs:sequence>"
    "                                        <xs:element name=\"Description\" type=\"xs:string\"/>"
    "                                        <xs:element name=\"Value\" type=\"xs:string\"/>"
    "                                    </xs:sequence>"
    "                                </xs:complexType>"
    "                            </xs:element>"
    "                        </xs:sequence>"
    "                    </xs:complexType>"
    "                </xs:element>"
    "            </xs:sequence>"
    "        </xs:complexType>"
    "    </xs:element>"
    "</xs:schema>";

#endif // SERVER_H

