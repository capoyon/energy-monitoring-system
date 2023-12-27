#!/bin/bash
# This script is used to compress the html files to .gz in current directory
# Then converts it to .h file using xxd

# Script filename
script_filename="compress.sh"
source_dir=$(pwd)/html
output_dir=$(pwd)/web/html

rm -r ${source_dir}/tmp
mkdir ${source_dir}/tmp

# Compress all html to gz
for file in $source_dir/*.html; do
    filename=$(basename "$file")
    echo "$filename > gz"
    gzip -c --best "$file" > "${source_dir}/tmp/${filename%.*}"
done

cd $source_dir/tmp

for file in *; do
    filename=$(basename "$file")
    echo "$filename > $output_dir/$filename.h"
    xxd -i "$file" "$output_dir/$filename.h"
    rm "$file"
done

rm -r ${source_dir}/tmp
