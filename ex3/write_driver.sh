#!/bin/bash
ptxdist clean driver-gamepad
ptxdist clean game
ptxdist compile driver-gamepad
ptxdist compile game
ptxdist targetinstall driver-gamepad
ptxdist targetinstall game
ptxdist targetinstall kernel
ptxdist image root.romfs
ptxdist test flash-rootfs
