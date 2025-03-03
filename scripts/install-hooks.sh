#!/bin/bash

# Get the directory containing this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Get the root directory of the repository
REPO_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Make the pre-commit hook executable and copy it to .git/hooks
chmod +x "$REPO_ROOT/.git/hooks/pre-commit"
cp "$REPO_ROOT/scripts/pre-commit" "$REPO_ROOT/.git/hooks/"

echo "Git hooks installed successfully!" 