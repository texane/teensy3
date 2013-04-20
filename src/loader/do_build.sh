#!/usr/bin/env sh
gcc -Wall -O2 -DUSE_LIBUSB=1 load_linux_only.c -lusb
