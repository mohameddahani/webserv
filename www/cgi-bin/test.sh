#!/bin/bash
echo "Content-Type: text/plain"
echo

# Hello message
echo "Hello from Bash CGI!"
echo "===================="
echo

# Print environment variables
echo "Environment Variables:"
echo "----------------------"

# Print all env vars sorted
env | sort

echo
echo "===================="

# If POST request, print body
if [ "$REQUEST_METHOD" = "POST" ]; then
    echo "POST Body:"
    echo "----------"

    # Read exactly CONTENT_LENGTH bytes from stdin
    if [ -n "$CONTENT_LENGTH" ]; then
        read -n "$CONTENT_LENGTH" POST_DATA
        echo "$POST_DATA"
    else
        echo "(No Content-Length provided)"
    fi
fi
