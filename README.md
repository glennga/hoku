# Hoku (Lost-in-Space Star Identification for Spacecraft)

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

There has been an increasing number of approaches toward stellar attitude determination, but little comparison between
each of these methods in a more controlled manner. Interchangeable factors are abstracted away (camera hardware, blob
detection, etc...) to focus more on how each method matches stars in an image to a catalog.

## Getting Started
This repository requires the following:
1. `python3` (with `numpy, matplotlib, opencv`). Used for the data analysis and visualization.
2. `CMake (2.8.10)` or above. Used to manage and build the C++ code here.
3. `git` or some Git client. Used to clone this repository, and to grab GoogleTest for testing.
4. C++ build tools (`make 4.1`, `gcc 5.4`, `g++ 5.4`).

To get started, clone this repository:
```cmd
git clone https://github.com/glennga/hoku.git
```

Build the necessary directories and create the `HOKU_PROJECT_PATH` variable by running the first build script. Execute
the `first-build.sh` script. 
```cmd
cd hoku 
chmod +x script/first-build.sh

# You must execute this script from the top level directory of Hoku!
sudo script/first-build.sh
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

Modify the `CONFIG.ini` file to fit your hardware and experiment parameters. A more descriptive version of the 
**id-parameters** section is below:
1. `sl` = Maximum number of tuples to select while querying the catalog.
2. `nr` = If toggled to 1, the reduction requirements are removed. Instead of going through each 
identification's method specified reduction process, the first element of the list is simply selected.
3. `fbr` = If toggled to 1, the results chosen from the reduction process will favor the bright star sets, as opposed
to the more dimmer ones. 
4. `so` = Value in degrees used to determine if an image star overlays with a catalog star. This value corresponds to
noise. Increasing this value raises your chances of collecting false positives, but decreasing this value may lead 
to more false negatives.
5. `nu-m` = Maximum number of query star comparisons. To prevent an identification method from exhausting every 
possible option and consuming time, set this appropriately.
6. `wbs` = Wabha's problem solver. Select the choices: `TRIAD`, `SVD`, or `Q`. These are different methods of 
determining a rotation given vector observations in both frame. For every instance where Wahba's problem occurs, this
method will be applied.

The **query-sigma** section is also important to modify, as this controls how many results should be returned from 
the initial query.

The `nibble.db` database holds all the data each identification method will reference (the catalog). The link 
[here](https://drive.google.com/file/d/14ZOKBnu3MJV_Fiw9LmygzdUokeYpNVVz/view?usp=sharing) provides this database 
with the given parameters:
- Field-of-view (`fov` in `CONFIG.ini`) < 20
- Maximum apparent magnitude seen by detector (`m-bright` in `CONFIG.ini`) = 6
- Representation of sky at January 2018 (`time` in `CONFIG.ini`)

For different parameters, modify `CONFIG.ini` and generate your own database using `GenerateN`. This program 
generates the lookup tables for each identification method. To generate the `nibble.db` lookup tables for the Angle 
method, enter the following:
```cmd
cd hoku/bin
./GenerateN angle 
```

To generate the complete `nibble.db` use the `script/generate-all-n.sh` script:
```cmd
cd hoku
chmod +x ./script/generate-all-n.sh
./script/generate-all-n.sh
```

## Running Experiments
There exist three experiments in this research:
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

The program `PerformE` will execute all combinations of the experiments and identification methods above. The first 
argument is the identification method, the second is the type of experiment, and third optional argument is the name
of the database to record to. To execute the query experiment for Gottlieb's Angle method, enter the following:
```cmd
cd hoku/bin
./PerformE angle query
```
The results will be logged in the Lumberjack database (`lumberjack.db`) by default, stored in tables according to 
the experiments and grouped by the experiment timestamp. 

To run experiments for all methods, use the `script/trial-all-methods.sh` script. This 



To view the results of these experiments, use the visualize script. 

TODO: Finish the `visualize_results.py` portion.

## Star Identification Procedure Usage

### IdentifyFITS Executable
To identify stars in an image and view the results, use the executable `IdentifyFITS`:
```cmd
cd hoku/bin
./IdentifyFITS [id-method] [image-file]
```

The first argument specifies the type of identification method to run. The second argument specifies the FITS file to
read. To run the Angle identification method on `my-image.fits`, enter the following:
```cmd
./IdentifyFITS angle my-image.fits
```

The output runs the `visualize-image.py` script to display your image, with Hipparcos labels attached to each point. 

### Library Integration
To use a star identification procedure in another application (such as an actual star tracker), start by copying the 
entire Hoku directory into your project. 

Include the desired star identification procedure you want to use:
```cpp
#include "hoku/include/identification/angle.h"
```

Example usage of Gottlieb's Angle identification procedure is depicted below. 
```cpp
/// INIReader to hold configuration associated with experiments.
INIReader cf(std::getenv("HOKU_PROJECT_PATH") + std::string("/CONFIG.ini"));

// Input: A std::vector (Star::list) of Star objects, representing stars as 3D vectors in 
//        the image coordinate system.
Star::list image = {Star(0, 0, 1), ...};  

// Input: A Star object representing the center of the image in the image coordinate system.
Star focus = ...;

// Wrap this in a Benchmark object. 20 here represents the field-of-view.
Benchmark input (image, focus, 20);

// Define any hyperparameters a Parameter struct.
Angle::Parameters p;
Identification::collect_parameters(p, cf);

// Define the location of the comparison count, and the table name.
p.table_name = cf.Get("table-names", "angle", ""), p.nu = std::make_shared<unsigned int> (0);

// Output: A std::vector (Star::list) of Star objects, holding all stars in the image that were 
//         identified, and with labels attached to them.
Star::list output = Angle(input, p).identify()

// To view the labels, use the 'get_label()' method for each Star.
int ell_0 = output[0].get_label();
int ell_1 = output[1].get_label();

// View the output with the visualize script.
Benchmark output(output, focus, 20);
output.display_plot();
```

Only CMake builds are supported at the moment. The final step is to link the `HOKU_LIBS` to your project in your 
`CMakeLists.txt` file. 
```cmake
add_executable(MyStarTracker my-star-tracker.cpp)
target_link_libraries(MyStarTracker ${SOME_OTHER_LIBS} ${HOKU_LIBS})
```

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

## LaTeX Generation
To build any presentation or paper that resides in the `doc` folder, use the build script. There currently exist the following
targets:
```cmd
cd hoku/script
mkdir hoku/doc/pdf
chmod +x build-doc.sh

# Build the paper and presentations.
./doc-build.sh arxiv-paper
./doc-build.sh vldb-paper
./doc-build.sh icde-paper
./doc-build.sh present
./doc-build.sh present-wahba
```

The PDFs reside in the `doc/pdf` folder.
```cmd
cd hoku/doc/pdf

# Open the PDFs from the console.
xdg-open arxiv-paper.pdf
xdg-open vldb-paper.pdf
xdg-open icde-paper.pdf
xdg-open present.pdf
xdg-open present-wahba.pdf
```
