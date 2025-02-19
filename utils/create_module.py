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
# This scripts should be used to create the base project of a module

import argparse
import os
import shutil as sh
import re
    

def createModule(name, category):
    # Get path of the git directory
    git_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    # Strip trailing / from category
    category = category.strip('/')
    folder_name = 'h265_' + name

    print('Module name: ', name)
    print('Folder name: ', folder_name)
    print('Category: ', category)
    folder_path = git_dir + '/hls/' + category + '/' + folder_name
    print('Folder path: ', folder_path)
    
    # Create directory
    try:
        os.makedirs(folder_path)
    except FileExistsError:
        print('Folder already exits, %s is not created' % folder_path)

    # Copy contents of template directory to the module directory
    for file in os.listdir(git_dir+ '/hls/template'):
        if file == "directives.tcl":
            os.symlink(git_dir + '/hls/template/' + file, folder_path + '/' + file)
        else:
            try:
                sh.copy(git_dir + '/hls/template/' + file, folder_path)
            except IsADirectoryError:
                print(git_dir + '/hls/template/' + file + ' is a directory, nothing is done.')
    # Read in the file
    with open(folder_path + '/input_files.tcl', 'r') as file :
        filedata = file.read()

    # Replace the target string
    filedata = filedata.replace('module', name)

    # Write the file out again
    with open(folder_path + '/input_files.tcl', 'w') as file:
        file.write(filedata)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--name', type=str, required=True)
    parser.add_argument('--category', type=str, required=True)
    args = parser.parse_args()
    
    # Check if the module name rescpect the naming convention
    if re.match('^([A-z]*_?)+$', args.name):
        createModule(args.name, args.category)
    else:
        print('ERROR: Module name is not following the naming convention.')
        print('Should be \'module\' or \'module_name\'.')

