#!/bin/sh
g++ main.cpp msl/file_util.cpp msl/json.cpp msl/socket.cpp msl/socket_util.cpp msl/string_util.cpp msl/time_util.cpp msl/webserver_threaded.cpp -o media_server -O -lpthread -Wall -Wno-unused-variable
