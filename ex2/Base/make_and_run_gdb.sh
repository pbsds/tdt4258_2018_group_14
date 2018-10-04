#!/usr/bin/env bash

make &&
make upload && (

JLinkGDBServer >/dev/null &
server_pid=$!

arm-none-eabi-gdb -x .gdbinit ex1.elf

kill $server_pid
)
