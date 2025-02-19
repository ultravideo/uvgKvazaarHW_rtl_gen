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

# This script is used to generate the RTL files and to test each module.

import argparse
import os
import shutil as sh
import re
import subprocess
import json
from pprint import pprint
from multiprocessing import Pool
from datetime import datetime

class bcolors:
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'


def run_catapult_workflow(module, category, update, target_project_path):
    error=0
    p = subprocess.run([catapult, "-shell" ,"-file", git_dir + "/hls/" + category + "/" + module + "/directives.tcl"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=False)
    # Raise errors form subprocess exxecution.
    output = p.stdout.decode()
    pass_test = False
    if output.find('HDL file has been generated.') >= 0:
        pass_test = True
        if output.find('SIMULATION PASSED') >= 0:
            print('['+ bcolors.OKGREEN + 'SUCCESS' + bcolors.ENDC + '] %s/%s testbench passed.' % (category, module))
        else:
            # Testbench failed but compiled
            print('['+ bcolors.WARNING + 'WARNING' + bcolors.ENDC + '] %s/%s testbench failed.' % (category, module))
            print('Check out the logs for more details: ' + git_dir + "/hls/" + category + "/" + module + "/working_dir/catapult.log")
    else:
        print('['+ bcolors.FAIL + 'ERROR' + bcolors.ENDC + '] %s/%s compilation failed.' % (category, module))
        print('Check out the logs for more details: ' + git_dir + "/hls/" + category + "/" + module + "/working_dir/catapult.log")
        error=1
    # Update target project 
    if update and pass_test: 
        if target_project_path != None and os.path.isdir(target_project_path):
            folder_path = target_project_path + '/' + category
            try:
                os.makedirs(folder_path)
            except FileExistsError:
                pass
            # Find the directory containing generated rtl files
            # ls_workdir = os.listdir(git_dir + "/hls/" + category + "/" + module + "/working_dir/")
            rtl_dir = 'Catapult'
            ls_workdir = [f.split('_') for f in os.listdir(git_dir + "/hls/" + category + "/" + module + "/working_dir/") if re.match(r'^Catapult_[0-9]+$', f)]
            if ls_workdir :
                ls_workdir.sort(key=lambda l:int(l[1]), reverse=True)
                rtl_dir = '_'.join(ls_workdir[0])
            tmp_subdir = [f for f in os.listdir(git_dir + "/hls/" + category + "/" + module + "/working_dir/" + rtl_dir) if re.match(r'[A-z0-9]+.v[0-9]+', f)]
            rtl_file = git_dir + "/hls/" + category + "/" + module + "/working_dir/" + rtl_dir + '/' + tmp_subdir[0] + '/concat_rtl.v'
            
            # Get git SHA1
            sha1 = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()
            now = datetime.now()
            isdirty = 0
            if subprocess.check_output(['git', 'diff', '--stat']) != '':
                isdirty = '-dirty'
            if os.path.exists(os.path.abspath(rtl_file)):
                with open(os.path.abspath(rtl_file), 'r') as infile:
                    lines = infile.readlines()
                    try:
                        os.makedirs(folder_path + '/' + os.path.dirname(module))
                    except FileExistsError:
                        pass
                    with open(os.path.abspath(folder_path + '/' + module + '_rtl.v'), 'w') as outfile:
                        print('// kvz_hls_rtl_gen repository SHA1: ', sha1 + isdirty, ' - ' ,now.strftime("Generated on %m/%d/%Y at %H:%M:%S"), file=outfile, end='')
                        for line in lines:
                            line.replace("mgc",line)
                            line.replace("ccs",line)
                            outfile.write(line)
    return error

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--list', type=str, required=True, help="Link to the JSON file containing the list of modules.")
    parser.add_argument('-m', '--module', type=str, required=False, help="Name of the module to compile.")
    parser.add_argument('-c', '--catapult', type=str, required=False, help="Name of the category containing the specified module.")
    parser.add_argument('-u', '--update', action='store_true', required=False, help="Update the generated rtl files in the target project.")
    parser.add_argument('--target_project_path', type=str, required=False, help="Path of to the target project folder to update")
    args = parser.parse_args()

    # Check if catapult is available
    if args.catapult is not None:
        catapult=sh.which(args.catapult)
        if  catapult is None:
            print("Path given for catapult seems wrong. Did you make a mistake ?")
            exit(1)
    else:
        catapult=sh.which("catapult")
        if catapult is None:
            print("Catapult is not in the PATH, please add it to your PATH or use --catapult /path/to/catapult.")
            exit(1)

    # Check if target_project_path is defined
    if args.update and args.target_project_path is None:
        print("Path to the target project is not defined. Please use --target_project_path.")
        exit(1)


    # Check which catapult version is used
    p = subprocess.run([catapult, "-v"], capture_output=True)
    stdout = p.stdout.decode()
    print("Catapult is available:")
    print("Path: " + catapult)
    if stdout.find("Catapult Ultra Synthesis") >= 0:
        catapult_version = stdout.split(" ")[5]
        print("Version: " + catapult_version)
    else:
        print("No Catapult version found.")
        exit(1)

    # Open the module list
    git_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    jsonList = args.list
    try:
        with open(jsonList) as json_file:
            module_list = json.load(json_file)
    except:
        print("Failed to load JSON list from '" + jsonList + "'")
        exit(1)

    if args.module == None:
        print('No module specified.')
        print('Running tests for all the modules, specified in %s.' % jsonList)
    # Sanitize input
    #elif re.match('^(([A-z0-9]+_?)+\/{1}([A-z0-9]*_?)+)+$', args.module):
    elif re.match(r'^(([A-Za-z0-9]+_?)+/([A-Za-z0-9]*_?)+)+$', args.module):
        split_mod_name = args.module.split('/')
        category = split_mod_name[0]
        module = "/".join(split_mod_name[1:])
        print('Category specified: ' + category)
        print('Module specified: ' + module)
        # Check if module exists in module_list
        try:
            module_list["category"][category]
            if module not in module_list["category"][category]:
                print("Module \"%s\" does not exist in %s." % (module, jsonList))
                exit(1)
        except KeyError:
            print("Category \"%s\" does not exist in %s." % (module, jsonList))
            exit(1)
        # Module Exists in module_list, running the test.
        err=run_catapult_workflow(module, category, args.update, args.target_project_path)
        exit(err)
    else:
        print("Entry does not respect the naming convention. Should be category/module.")
        exit(1)

    # No module specified, running tests for all modules in moduleList.json
    module_category_tuple = []
    for category in module_list["category"]:
        for module in module_list["category"][category]:
            module_category_tuple.append((module, category, args.update, args.target_project_path))
    print("%d Modules found." % len(module_category_tuple))
    with Pool(None) as pool:
        return_val=pool.starmap(run_catapult_workflow, module_category_tuple)
    exit(sum(return_val) != 0)
