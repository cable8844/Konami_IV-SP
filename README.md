# KONAMI Coding Interview: Simple C Client/Host
Konami coding interview, using C to create a localhost web server, which forwards xml from clients

Requirements:
Compiles using gcc in x86-64 architecture

## Server
Default to 127.0.0.1:5000
Accept client connection
Process valid XML, reject invalid with appropriate error

## Client
Client is a simple shell script that sends xml documents via netcat.

## Building
The project uses CMake to build, with generic make commands integrated
`make` suffices to build the project in a `build` directory
`make clean` will clean up the environment
`make DEB` builds the project and uses CPack to pack it into a Debian package.
`make RPM` builds the project and uses CPack to pack it into an rpm package.

Docker can be utilized to compile and package in a generic environment with the expected dependencies.
Docker should be used when compiling the DEB / RPM files to be released.
`docker build -t konami_package_builder -f Dockerfile .`

## Installing

## Usage
Usage: server [OPTION...] 
A simple server program that listens for XML messages, adds them to a queue and
processes them.

  -a, --address=ADDRESS      Address to listen on (default: 127.0.0.1)
  -p, --port=PORT            Port to listen on (default: 5000)
  -s, --schema=SCHEMA        Path to the XML schema file (default: load from
                             memory)
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <calebfarrand@gmail.com>.

## Testing

## Known Limitations & Bugs
* Currently, the application does not exit cleanly
    - <conio.h> may add extra (unhandled) dependencies to the package. It is disabled, therefore 'q' is not handled as a way to exit.
    - <signal.h> may add exra (unhandled) dependencies to the package. It is disabled, therefore, signal interrupts are not handled.
* The queue used to process messages has a capacity limit hard coded to 1024. Messages may get dropped.
* The timestamp associated with messages is currently set after the message has been received, not at the time of receive. 
* The response size is a fixed buffer. A response which overflows the buffer results in undefined behavior.