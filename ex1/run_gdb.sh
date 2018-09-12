#!/usr/bin/env bash

JLinkGDBServer >/dev/null &
server_pid=$!

arm-none-eabi-gdb -x .gdbinit ex1.elf 

kill $server_pid
