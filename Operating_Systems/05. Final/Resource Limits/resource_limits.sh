#!/usr/bin/env bash
spawner() {
    # Check the number of arguments
    if [ "$#" -ne 3 ]; then
        echo "Error: Incorrect number of arguments. Usage: $0 memoryInKB timeoutInSec binaryName"
        exit 2
    fi

    memory_limit="$1"
    timeout_limit="$2"
    binary_name="$3"

    # Check if the binary exists
    if [ ! -e "$binary_name" ]; then
        echo "Error: Binary '$binary_name' doesn't exist"
        exit 3
    fi

    # Check if the binary has the +x flag
    if [ ! -x "$binary_name" ]; then
        echo "Error: Binary '$binary_name' doesn't have the +x flag."
        exit 4
    fi

   # Run the binary with timeout and memory limit
    "./$binary_name" &

    # Capture the process ID of the background process
    pid=$!

    start_time=$(date +%s%N)
    # Loop while waiting for the process to finish
    while kill -0 "$pid" 2>/dev/null; do

        # Check for elapsed time
        end_time=$(date +%s%N)
        elapsed_time=$(( (end_time - start_time) / 1000000 ))

        if [ "$elapsed_time" -gt "$timeout_limit" ]; then
            echo "Error: Timeout. Exiting gracefully."
            # Terminate the process
            kill "$pid"
            exit 5
        fi

        # Check for memory limit using ps command
        memory_usage=$(ps -p "$pid" -o rss=)
        
        if [ "$memory_usage" -gt "$memory_limit" ]; then
            echo "Error: Memory limit reached. Exiting gracefully."
            # Terminate the process
            kill "$pid"
            exit 1
        fi
    done


    echo "Execution completed successfully."
    # Exit with success
    exit 0
}