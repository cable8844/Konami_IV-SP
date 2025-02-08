# Use the official Ubuntu 24.04 as a base image
FROM ubuntu:24.04

# Update the package list and install dependencies
RUN apt-get update && \
    apt-get install -y \
    cmake gcc g++ pkg-config \
    libxml2-dev netcat-traditional && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /workspace

# Copy the source code into the container
COPY . /workspace
RUN make clean
