# uvgKvazaarHW_rtl_gen

Contains all Catapult HLS tcl scripts and sources to generates HDL modules

## Getting started

### To create a new module
Module Naming convention:
 * folder name: h265_module_name
 * main filename: main_module_name.cpp
 * testbench filename: tb_module_name.cpp
 
To create a new RTL module, use the python utilitary:
```
./utils/create_module.py --name module_name --category category
```
At first, you should only modify the input_files.tcl.
Then after compiling everything, you can save the different constraint to the constraint.tcl.
If your testbench require data files, you can create a test_data folder with your different data files. REMINDER: Reference the file using relative paths.

### To compile all HLS codes to a target folder
To compile all RTLs to the root of uvgKvazaarHW
```
./utils/run_test.py -l utils/moduleList.json --update --target_project_path ../rtls
```

Replacing reoccurring module names in the generated RTLs to be unique, the following script can be used for single files
```
./utils/replace_reoccurring_module_names.py <file>
```
or for all recursively
```
find ../rtls/ -type f -name "*.v" -exec ./replace_reoccurring_module_names.py {} \;
```