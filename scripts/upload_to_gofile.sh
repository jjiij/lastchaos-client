#!/bin/bash
set -e

if [ $# -lt 2 ]; then
    echo "Usage: $0 <file_path> <folder_id>"
    exit 1
fi

FILE_PATH="$1"
FOLDER_ID="$2"
API_TOKEN="${GOFILE_API_TOKEN:-}"

if [ -z "$API_TOKEN" ]; then
    echo "Error: GOFILE_API_TOKEN environment variable not set"
    exit 1
fi

if [ ! -f "$FILE_PATH" ]; then
    echo "Error: File not found: $FILE_PATH"
    exit 1
fi

echo "Uploading $(basename "$FILE_PATH") to GoFile..."

RESPONSE=$(curl -sS \
    -F "file=@$FILE_PATH" \
    -F "folderId=$FOLDER_ID" \
    https://api.gofile.io/uploadFile)

UPLOAD_URL=$(echo "$RESPONSE" | grep -o '"downloadPage":"[^"]*"' | head -1 | cut -d'"' -f4)
DOWNLOAD_URL=$(echo "$RESPONSE" | grep -o '"downloadUrl":"[^"]*"' | head -1 | cut -d'"' -f4)

if [ -n "$UPLOAD_URL" ] && [ -n "$DOWNLOAD_URL" ]; then
    echo "Upload successful!"
    echo "Download link: $DOWNLOAD_URL"
    echo "Share link: $UPLOAD_URL"
else
    echo "Upload failed"
    echo "Response: $RESPONSE"
    exit 1
fi
