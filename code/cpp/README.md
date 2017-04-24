# Prerequisites 
First clone the project:
```
git clone --recursive git@github.com:lukasstevens/bachelor-thesis
```
For building the project the build system [craftr](https://github.com/craftr-build/craftr) is used. Craftr is a build system based on ninja and written in python3.
Therefore you first have to install `ninja-build>=1.7.1` with your package manager. 
Then you have to install the development version of craftr:
```
pip3 install --user git+https://github.com/craftr-build/craftr@development
```
The c++ toolchain used for this project is clang so you have to install `clang>=3.8.1` with your package manager.

The project also depends on one library namely `libgmp`. You will need to install it with your package manager.

# Building
Before your first build or if you added source files or if you changed the build script run `craftr export`. 
After that you can run `craftr build` to build the project.

# Testing
The build system creates a symlink from `build/resources` to `src/test/resources` when `craftr export` is executed.
You can put additional resource files into `src/test/resources`.

# Running
You can run the algorithm with your input by executing `craftr build run`.
With this command the program will read the input from the standard input.
The program expects you to enter a tree first. The format of a tree is specified in the documentation(see Documentation section).
After you entered the tree, the program expects three additional space-seperated integers `k`, `e_n` and `e_d`.
`k` is the number of parts into which the algorithm should partition the tree and `e_n/e_d` specifies the approximation parameter epsilon.
If you want read the tree from a file, but want the program to read `k` and `e_n/e_d` from stdin you can execute `cat your_tree_file - | craftr build run`.

# Documentation
You can generate the doxygen documentation by running `craftr build docs`. The documentation is located in `build/docs`.
