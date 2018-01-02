#!/bin/bash
LD_PRELOAD=/usr/local/lib/libtcmalloc.so HEAPPROFILE=/tmp/kakap HEAP_PROFILE_ALLOCATION_INTERVAL=1024 ./sender
