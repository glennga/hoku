# Hoku (Lost-in-Space Star Identification for Spacecraft)

**Note: this project is no longer being maintained.**


## Overview
Ancient mariners could look up the night sky, point out what stars they were looking at, and navigate across the globe 
with precision. _Star identification algorithm_ refers to a computational approach to pointing out which stars are in 
the sky. Given an image of the sky, star identification is matching the bright spots in an image, to stars in an 
astronomical catalog. The device that performs these computations is the star tracker, much like the navigators on the 
ship. _Lost-in-space_ refers to an additional constraint on the problem: the absence of knowing where we took the 
picture and how we pointed the camera.

This repository holds research toward the analysis of various lost-in-space _star identification_ procedures for 
spacecraft. This includes a study of feature uniqueness, permutation order, candidate reduction, and identification 
under the introduction of various noise. The process of identifying blobs in an image, constructing the image 
coordinate system, and efficiently querying static databases is __not__ addressed here, but there does exist some 
rudimentary programs to execute the attitude determination process end-to-end.

## Getting Started (Quick Start)
This repository requires the following:
1. `python3` (with `numpy, matplotlib, opencv`). Used for the data analysis and visualization.
2. `CMake (2.8.10)` or above. Used to manage and build the C++ code here.
3. `git` or some Git client. Used to clone this repository, and to grab GoogleTest for testing.
4. C++ build tools (`make 4.1`, `gcc 5.4`, `g++ 5.4`).

To get started, clone this repository:
```cmd
git clone https://github.com/glennga/hoku.git
```

Generate the executables. The output will reside in the `bin` folder. 
```cmd
# Create the Makefiles.
cd hoku/build
cmake -G"Unix Makefiles" ..

# Build the executables.
cd src
make -j8 install
```

Run the setup script to construct the Nibble database. Modify the generation of this database through `hoku/hoku.cfg`.
```cmd
./hoku/hoku.setup
```

## Running Experiments
There exist four experiments in this research:
1. Feature Uniqueness (`query`)
2. Candidate Reduction (`reduction`)
3. Identification (`identification`)
4. Overlay (`overlay`)

There exist six different identification methods implemented here:
1. Gottlieb's Angle Method (`angle`)
2. Liebe's Dot Angle Method (`dot`)
3. Cole and Crassidus's Spherical Triangle Method (`sphere`)
4. Cole and Crassidus's Planar Triangle Method (`plane`)
5. Mortari's Pyramid Method (`pyramid`)
6. Toloei's Composite Pyramid Method (`composite`)

To run experiments for all methods, use the `hoku/hoku.run` script. The results will be logged in the
Lumberjack database (`lumberjack.db`) by default, stored in tables according to the experiments and grouped by the
experiment timestamp.  

## Google Test Generation
Google Test is attached as a Git Submodule. Run the following commands to get Google Test in this repository.
```cmd
cd hoku
git submodule init
git submodule update
```

Generate the test runner. The output will reside in the `bin` folder.
```cmd
# Create the Makefiles.
cd hoku/build
cmake -G"Unix Makefiles" -DBUILD_TEST=ON ..

# Build the test runner.
cd test
make -j8 install
```

Execute the test runner. By default, this runs all of tests in the tests folder. If desired, you can restrict the tests 
to a certain pattern using the `--gtest_filter` option. Note that all classes are sorted into their own test suite. 
More information on this argument can be found 
[here](https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#running-a-subset-of-the-tests).
```cmd
cd hoku/bin

# Run all of the tests in the 'tests' folder.
./PerformT

# Run only the Benchmark tests.
./PerformT --gtest_filter=Benchmark*
```
