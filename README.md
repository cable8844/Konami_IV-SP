# KONAMI Coding Interview: Simple C Client/Host
Konami coding interview, using C to create a localhost web server, which forwards xml from clients

Requirements:
Compiles using gcc in x86-64 architecture

## Server
Default to 127.0.0.1:5000

Accept client connection, processes valid XML and reject invalid xml with the appropriate error

### Usage
```
server --help
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
```

## Client
Client is a simple shell script that sends xml documents via netcat.
### Usage
`./client.sh `
`Usage: ./client.sh <IP> <port> <file1> [file2 ... fileN]`

## Building
The project uses CMake to build, with generic make commands integrated: `make` suffices to build the project in a `build` directory, `make clean` will clean up the environment, `make DEB` and `make RPM` build the DEB / RPM package for their own respective infrastructure.

Docker can be utilized to compile and package in a generic environment with the expected dependencies and should be used when compiling the DEB / RPM files to be released.

`docker build -t konami_package_builder -f Dockerfile .`

## Installing
### Ubuntu
`sudo apt install ./Konami_iv_sp-0.0.1-Linux.deb`
### RPM
`sudo yum install ./Konami_iv_sp-0.0.1-Linux.rpm`

## Testing
In one terminal, start the server listener
`server`
In another terminal, send xml data via `client.sh` and xml documents
`./client.sh test/ls_etc.xml`

## Known Limitations & Bugs
* Currently, the application does not exit cleanly
    - <conio.h> may add extra (unhandled) dependencies to the package. It is disabled, therefore 'q' is not handled as a way to exit.
    - <signal.h> may add exra (unhandled) dependencies to the package. It is disabled, therefore, signal interrupts are not handled.
* The queue used to process messages has a capacity limit hard coded to 1024. Messages may get dropped.
* The timestamp associated with messages is currently set after the message has been received, not at the time of receive. 
* The response size is a fixed buffer. A response which overflows the buffer results in undefined behavior.
* I did not have time to properly package libxml2 with the DEB / RPM files. They are only dependencies right now. I proper solution would
  be to additionally bundle it into the package and change the rpath of the executeable to point to the original .so with which it was compiled.