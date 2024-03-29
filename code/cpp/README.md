# Bachelor thesis
## Prerequisites 
First clone the project:
```
git clone --recursive git@github.com:lukasstevens/bachelor-thesis
```
Or pull the project and its submodules if you already cloned it:
```
git pull && git submodule update --recursive --init
```

### Dependencies of this project
The build system is [Craftr](https://github.com/craftr-build/craftr).
Since Craftr uses ninja you have to install `ninja-build>=1.7.1`.
Craftr can be installed with pip in the following way:
```
pip3 install --user git+https://github.com/craftr-build/craftr@master
```
Keep in mind that this installs craftr into `~/.local/bin` on most systems which you may have to add to your PATH variable.
The C++ toolchain used for this project is clang so you have to install `clang>=3.8.1` with your package manager.
Furthermore this project uses `libgmp` which you can install with your package manager.

### Submodule dependencies
For building the submodules the following programs/libraries are required.

* KaHIP
    - `scons`
    - `libargtable2-0-dev`
    - `libopenmpi-dev`
* METIS
    - GNU `make`
    - `cmake>=2.8`

## Building
Before your first build or if you added source files or if you changed the build script run `craftr export`. 
After that you can run `craftr build` to build the project.

## Testing
The build system creates a symlink from `build/resources` to `resources` when `craftr export` is executed.
You can put additional resource files into `resources` which will be available to the executable under the path `.`.

## Running
You can run the algorithm with your input by executing `craftr build run`.
With this command the program will read the input from the standard input.
The program expects you to enter a tree first. The format of a tree is specified in the documentation(see Documentation section).
After you entered the tree, the program expects three additional space-seperated integers `k`, `e_n` and `e_d`.
`k` is the number of parts into which the algorithm should partition the tree and `e_n/e_d` specifies the approximation parameter epsilon.
If you want read the tree from a file, but want the program to read `k` and `e_n/e_d` from stdin you can execute `cat your_tree_file - | craftr build run`.

The command line interface can be accessed by running `craftr build cli`.
You can pass arguments to the command line with `craftr build cli[args...]`.
Use `craftr build cli[--help]` to get an overview over the available options.
Note that the CLI reads graphs in the CSR file format.
The format is described in the metis manual which lies in `deps/metis/manual`.

## Documentation
You can generate the doxygen documentation by running `craftr build docs`. The documentation is located in `build/docs`.

## Note about Github
I recommend that you use the latest version of this repository located on Github at https://github.com/lukasstevens/bachelor-thesis if you did not obtain the code from Github.
Furthermore, do not hesistate to open an issue if something is not working.

## License
The code is licensed under the GPLv2. For the license see the LICENSE file.
If you require a more permissive license you can fork the project and remove the dependencies to KaHIP.
I would be willing to license the modified code under the MIT license.
