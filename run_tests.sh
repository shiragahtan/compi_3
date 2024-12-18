#!/bin/bash

for input_file in hw2-tests/*.in; do
    base=$(basename "$input_file" .in)
    ./hw2 < "$input_file" > "my_${base}.out"
    diff "my_${base}.out" "hw2-tests/${base}.out"
    if [ $? -eq 0 ]; then
        echo "Test ${base}: Passed"
    else
        echo "Test ${base}: Failed"
    fi
done

