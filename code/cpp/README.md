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

# Building
Before your first build or if you added source files or if you changed the build script run `craftr export`. 
After that you can run `craftr build` to build the project. To run the project with your input you can use `craftr build run`.

# Testing
When invoking `craftr export` the directory `src/test/resources` is automatically copied to `build/`. If you have added new resource files which you need for your tests you have to rerun `craftr export`.
