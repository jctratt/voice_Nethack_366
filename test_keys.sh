#!/bin/bash
# Test what escape sequences are sent by Ctrl+arrow keys
echo "Press Ctrl+Right arrow, then Enter:"
read -r response
echo "Got: $response"
echo "Hex dump:"
echo "$response" | od -A x -t x1z -v
echo ""
echo "Press Ctrl+Left arrow, then Enter:"
read -r response
echo "Got: $response"
echo "Hex dump:"
echo "$response" | od -A x -t x1z -v
