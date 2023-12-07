#!/bin/bash

# Script filename
script_filename="compress.sh"

rm -r gz
mkdir gz

# Compress all files in the current directory (excluding the script file) and save them as filename.gz
for file in *; do
    # Skip the script file
    [ "$file" == "$script_filename" ] && continue

    # Skip directories
    [ -f "$file" ] || continue
    
    # Compress the file and save as filename.gz
    gzip -c --best "$file" > "gz/${file%.*}"
    echo "Compressed: $file to gz/${file%.*}"
done

cd gz
for file in *; do
    [[ "$file" == *.h ]] && continue
    # Skip directories
    [ -f "$file" ] || continue
    xxd -i "$file" "$file.h"
    rm "$file"
done
