#!/bin/bash
IP="127.0.0.1"
PORT="5000"
if [ "$#" -eq 0 ]; then
    echo "Usage: $0 <IP> <port> <file1> [file2 ... fileN]"
    exit 1
fi

if [[ "$1" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    IP="$1"
    shift
fi

if [[ "$1" =~ ^[0-9]+$ ]]; then
    PORT="$1"
    shift
fi

for FILE in "$@"; do
    if [ -f "$FILE" ]; then
        nc $IP $PORT < "$FILE"
    else
        echo "File $FILE does not exist."
    fi
done
