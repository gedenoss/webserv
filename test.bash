#!/bin/bash

# ANSI colors
GREEN='\033[0;32m'
WHITE='\033[0;37m'
BLUE='\033[0;34m'
RED='\033[0;31m'
ORANGE='\033[0;33m'
RESET='\033[0m'
	# ANSI Bold
BOLDGREEN='\033[1;32m'
BOLDWHITE='\033[1;37m'
BOLDBLUE='\033[1;34m'
BOLDRED='\033[1;31m'
BOLDORANGE='\033[1;33m'

# Build the web server
make re

# Number of servers
N=${1:-4} # The first argument passed to the script

# Start of the file
echo "" > test.conf

# Loop to create each server block
# Test1: Create a configuration file with N servers
# for (( i=1; i<=N; i++ ))
# do
#     echo "server {" >> test.conf
#     echo "    listen $((i+8000));" >> test.conf
# 	echo "    server_name localhost;" >> test.conf
# 	echo "    location / {" >> test.conf
# 	echo "        root ./static/;" >> test.conf
# 	echo "        index index.html;" >> test.conf
#     echo "      allow_methods GET POST DELETE;" >> test.conf
# 	echo "    }" >> test.conf
#     echo "}" >> test.conf
#     echo "" >> test.conf
# done

# Test2: Create a N servers with the same port and different server_name
# for (( i=1; i<=N; i++ ))
# do
#     echo "server {" >> test.conf
#     echo "    listen 6060;" >> test.conf
# 	echo "    server_name localhost$((i+8000));" >> test.conf
# 	echo "    location / {" >> test.conf
# 	echo "        root ./static;" >> test.conf
# 	echo "        index index.html;" >> test.conf
#     echo "      allow_methods GET POST DELETE;" >> test.conf
# 	echo "    }" >> test.conf
#     echo "}" >> test.conf
#     echo "" >> test.conf
# done

# # Test3: Limit body size
echo "server {" >> test.conf
echo "    listen 6000;" >> test.conf
echo "    server_name localhost;" >> test.conf
echo "    client_max_body_size 11M;" >> test.conf
echo "    location / {" >> test.conf
echo "        root ./static;" >> test.conf
echo "        index index.html;" >> test.conf
echo "        allow_methods GET POST DELETE;" >> test.conf
echo "    }" >> test.conf
echo "}" >> test.conf

# Start the web server in the background
./webserv "test.conf"& # & at the end to run the process in the background
pid=$! # $! is the PID of the last background process
# Non-blocking check
for i in {1..5}; do
    if ! kill -0 $pid 2> /dev/null; then
        echo -e "[❌]${RED} Could Not Start Server${RESET}"
        exit 1
    fi
    sleep 1
done
sleep 1 # Wait for the server to start

# Run the tests

# Test 1
# echo -e "${BOLDBLUE} Test 1: ${RESET} Server with different ports"
# # Iterate over the servers and test each one of them with curl
# test1=1
# for (( i=1; i<=N; i++ ))
# do
# 	curl -s http://localhost:$((i+8000))/index.html | diff - static/index.html > /dev/null
# 	if [ $? -eq 0 ]; then
# 		test1=$((test1+1))
# 	else
# 		echo -e "[❌]${RED} Server $((i)) failed ${RESET}"
# 	fi
# done

# if [ $test1 -eq $((N+1)) ]; then
# 	echo -e "[✅]${GREEN} Handle ${N} servers ${RESET}"
# else
# 	echo -e "[❌]${RED} Handle ${N} servers ${RESET}"
# fi

# Test 2
# echo -e "${BOLDBLUE} Test 2: ${RESET} Server with the same port and different server_name"
# test2=1
# for (( i=1; i<=N; i++ ))
# do
# 	curl -s -H "Host: localhost$((i+8000))" http://localhost:6060/index.html | diff - static/index.html > /dev/null
# 	if [ $? -eq 0 ]; then
# 		test2=$((test2+1))
# 	else
# 		echo -e "[❌]${RED} Server $((i)) ${RESET}"
# 	fi
# done

# if [ $test2 -eq $((N+1)) ]; then
# 	echo -e "[✅]${GREEN} Handle ${N} servers ${RESET}"
# else
# 	echo -e "[❌]${RED} Handle ${N} servers ${RESET}"
# fi

# Test 3
echo -e "${BOLDBLUE} Test 2: ${RESET}"
# Test the server with a body size greater than the limit
dd if=/dev/zero of=testfile bs=1024 count=10240 # 1MB file called testfile

curl -s -X POST -d @testtext http://localhost:6000/index.html
 # Send the file to the server

if [ "$status_code" -eq 413 ]; then
	echo -e "[❌]${RED} Body size greater than the limit ${RESET}"
else
	echo -e "[✅]${GREEN} Body size greater than the limit ${RESET}"
fi
rm -f testfile # Remove the file

# Sleep
sleep 4

# Stop the web server
kill $pid

# Clean up
rm -f test.conf