#!/bin/bash

rm -f of.c  # Remove output file if it exists, ignore if it doesn't

while IFS= read -r file; do
    {
        echo "File: $file"
        awk '
        BEGIN {
            block_start = 0;
            for (i = 1; i <= 9; i++) buffer[i] = "";
            context_lines = 9;
            last_block_line = 0;
        }
        {
            # Shift buffer
            for (i = 1; i < context_lines; i++) buffer[i] = buffer[i+1];
            buffer[context_lines] = $0;

            if ($0 == "#ifdef VOICE_ENABLED") {
                if (block_start == 0) {
                    print "--- Previous Block Context -------------------------------------------------------------------------------------------------";
                    # Print context before, avoiding duplicates
                    for (j = 1; j <= context_lines && NR - j > 0; j++) {
                        if (buffer[j] != "" && buffer[j] != "#ifdef VOICE_ENABLED") {
                            printf "%s\n", buffer[j];
                        }
                    }
                }
                print "--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------";
                print;
                block_start = 1;
                last_block_line = NR;
            } else if ($0 == "#endif /* VOICE_ENABLED */") {
                print;
                print "--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------";
                block_start = 0;
                current_line = NR;
                # Capture and print context after, avoiding empty lines
                for (j = 1; j <= context_lines && getline > 0; j++) {
                    if ($0 != "") {
                        printf "%s\n", $0;
                    } else {
                        j--;  # Decrement j to not count empty lines towards context lines
                    }
                }
                # Reset file position
                for (j = 1; j <= context_lines && j <= NR - current_line; j++) getline;
                print "--- After Block Context -------------------------------------------------------------------------------------------------";
                next;
            } else if (block_start == 1) {
                printf "%s\n", $0;
            }
        }
        ' "$file"
    } >> of.c
    echo -e "\n" >> of.c
done < file_list.txt
