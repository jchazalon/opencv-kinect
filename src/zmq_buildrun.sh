#!/bin/sh

set -e

# Build ZMQ files
g++ -Wall -o zmq-publisher zmq-publisher.cpp -lzmq
g++ -Wall -o zmq-subscriber zmq-subscriber.cpp -lzmq


# Run ZMQ files
# ./zmq-subscriber &
# ./zmq-publisher &

