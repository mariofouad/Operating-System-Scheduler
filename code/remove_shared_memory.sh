#!/bin/bash

# Get the list of shared memory segments
ipcs -m | awk 'NR > 2 { print $2 }' | while read shmid; do
    # Remove each shared memory segment
    ipcrm -m $shmid
    echo "Shared memory segment with shmid $shmid removed"
done