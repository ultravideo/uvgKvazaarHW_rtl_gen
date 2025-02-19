#!/usr/bin/python3
#############################################################################
# This file is part of uvgKvazaarHW.
#
# Copyright (c) 2025, Tampere University, ITU/ISO/IEC, project contributors
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
# 
# * Neither the name of the Tampere University or ITU/ISO/IEC nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
#############################################################################

"""
File Word Replacement Script
----------------------------
This script replaces specific words in text files with the basename of the file.
For example, if processing 'example.v', words like 'mgc', 'ccs', etc. will be
replaced with 'example'.
"""

import sys
import os
from pathlib import Path


def process(filepath: str) -> str:
    """
    Process a file by replacing specific words with the file's basename.
    
    Args:
        filepath (str): Path to the file to process
        
    Returns:
        str: Status message indicating success or failure
        
    Example:
        If processing "example.v", the following replacements will occur:
        - "mgc" -> "example"
        - "ccs" -> "example"
        - "main_ip_get_ang_neg_hier" -> "example"
        - "register" -> "example"
    """
    filepath = Path(filepath)
    
    if not filepath.exists():
        return f"Error: File not found - {filepath}"
    
    try:
        # Get the basename without extension
        basename = filepath.stem
        
        # Words to be replaced
        replacements = [
            "mgc",
            "ccs",
            "main_ip_get_ang_neg_hier",
            "register"
        ]
        
        # Create a temporary file
        temp_filepath = filepath.with_suffix(filepath.suffix + ".tmp")
        
        # Process the file
        with open(filepath, 'r') as f_in, open(temp_filepath, 'w') as f_out:
            for line in f_in:
                # Perform all replacements
                for word in replacements:
                    line = line.replace(word, basename)
                f_out.write(line)
        
        # Replace the original file with the modified one
        os.replace(temp_filepath, filepath)
        return f"Successfully processed: {filepath}"
        
    except Exception as e:
        # Clean up temporary file if it exists
        if temp_filepath.exists():
            temp_filepath.unlink()
        return f"Error processing {filepath}: {str(e)}"


def print_help():
    """Print help message with usage instructions."""
    help_text = r"""
File Word Replacement Script
---------------------------
Replaces specific words in text files with the file's basename.

Usage:
    1. Single file:
       python script.py <file>
       
    2. Multiple files:
       python script.py <file1> <file2> <file3>
       
    3. With find command (recursive):
       find . -type f -name "*.v" -exec python script.py {} \;
       
Example:
    If processing "example.v", the following replacements will occur:
    - "mgc" -> "example"
    - "ccs" -> "example"
    - "main_ip_get_ang_neg_hier" -> "example"
    - "register" -> "example"
    """
    print(help_text)


def main():
    """Main function to handle command line arguments and process files."""
    if len(sys.argv) < 2 or sys.argv[1] in ['-h', '--help']:
        print_help()
        sys.exit(0)
    
    # Process all provided files
    for filepath in sys.argv[1:]:
        result = process(filepath)
        print(result)


if __name__ == "__main__":
    main()
