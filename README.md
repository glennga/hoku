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
coordinate system, and efficiently querying static databases is __not__ addressed here. 

There has been an increasing number of approaches toward stellar attitude determination, but little comparison between
each of these methods in a more controlled manner. Interchangeable factors are abstracted away (camera hardware, blob
detection, etc...) to focus more on how each method matches stars in an image to a catalog.

## Getting Started
This repository requires the following:
1. `python3` (with `numpy` and `matplotlib`). Used for the data analysis and visualization.
    1. Install Python 3: [https://www.python.org/downloads/](https://www.python.org/downloads/)
    2. Install Anaconda (`numpy` and `matplotlib`): [https://conda.io/docs/user-guide/install/index.html](https://conda.io/docs/user-guide/install/index.html)
    3. Install OpenCV for Python: Enter `conda install -c conda-forge opencv`
2. `CMake 3.7` or above. Used to manage and build the C++ code here.
    1. Install CMake: [https://cmake.org/install/](https://cmake.org/install/)
    2. CMake tutorial with CLion IDE: [https://www.jetbrains.com/help/clion/quick-cmake-tutorial.html](https://www.jetbrains.com/help/clion/quick-cmake-tutorial.html)
3. `git` or some Git client. Used to clone this repository, and to grab GoogleTest for testing.
    1. Install Git: [https://git-scm.com/book/en/v2/Getting-Started-Installing-Git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git)
    2. Install GitKraken (optional): [https://www.gitkraken.com/](https://www.gitkraken.com/)
4. C++ build tools (`make`, compiler, ...).
    1. Use `sudo apt-get install build-essential` on Linux.
    2. Download CLion IDE (optional): [https://www.jetbrains.com/clion/download](https://www.jetbrains.com/clion/download)

To get started, clone this repository:
```cmd
git clone https://github.com/glennga/hoku.git
```

Build the necessary directories and create the `HOKU_PROJECT_PATH` variable by running the first build script. Windows
users have to execute `first-build.bat`, and Linux users execute `first-build.sh`. _Note that you may have to restart 
your shell (exit, reopen) for the results to take place._
```cmd
cd hoku 
chmod +x script/first-build.sh

# You must execute this script from the top level directory of Hoku!
sudo script/first-build.sh
```

Generate the executables. The output will reside in the `bin` folder. 
```cmd
cd hoku/build
cmake -G"Unix Makefiles" ..
cd src
make -j8 install
```

Modify the `CONFIG.ini` file to fit your hardware and experiment parameters. A more descriptive version of the 
**id-parameters** section is below:
1. `sq` = Value in degrees used to vary the selectivity of a catalog search. This value corresponds to noise. 
Increasing this value raises your chances of collecting false positives, but decreasing this value may lead to more 
false negatives.
2. `sl` = Maximum number of tuples to select while querying the catalog.
3. `nr` = If toggled to 1, the reduction requirements are removed. Instead of going through each 
identification's method specified reduction process, the first element of the list is simply selected.
4. `fbr` = If toggled to 1, the results chosen from the reduction process will favor the bright star sets, as opposed
to the more dimmer ones. 
5. `so` = Value in degrees used to determine if an image star overlays with a catalog star. This value corresponds to
noise. Increasing this value raises your chances of collecting false positives, but decreasing this value may lead 
to more false negatives.
6. `nu-m` = Maximum number of query star comparisons. To prevent an identification method from exhausting every 
possible option and consuming time, set this appropriately.
7. `wbs` = Wabha's problem solver. Select the choices: `TRIAD`, `QUEST`, or `Q`. These are different methods of 
determining a rotation given vector observations in both frame. For every instance where Wahba's problem occurs, this
method will be applied.

The `nibble.db` database holds all the data each identification method will reference (the catalog). The link 
[here](https://drive.google.com/file/d/1fxId8hLzxEX9VJxO1-p_1ye_J8NRKlVK/view?usp=sharing) provides this database 
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

To generate the complete `nibble.db`, enter the following:
```cmd
cd hoku/bin
./GenerateN hip
./Generate angle
./Generate sphere
./Generate plane
./Generate pyramid
./Generate composite
```

## Running Experiments
There exist three experiments in this research:
1. Feature Uniqueness (`query`)
2. Candidate Reduction (`reduction`)
3. Identification (`identification`)

There exist six different identification methods implemented here:
1. Gottlieb's Angle Method (`angle`)
2. Liebe's Interior Angle Method (`dot`)
3. Cole and Crassidus's Spherical Triangle Method (`sphere`)
4. Cole and Crassidus's Planar Triangle Method (`plane`)
5. Mortari's Pyramid Method (`pyramid`)
6. Toloei's Composite Pyramid Method (`composite`)

The program `PerformE` will execute all combinations of the experiments and identification methods above. The first 
argument is the identification method, and the second is the type of experiment. To execute the query experiment for
Gottlieb's Angle method, enter the following:
```cmd
cd hoku/bin
./PerformE angle query
```

The results will be logged in the Lumberjack database (`lumberjack.db`), stored in tables according to the experiments 
and grouped by the experiment timestamp. To view the results of these experiments, use the visualize script.

TODO: Finish the `visualize_results.py` portion.

## Star Identification Procedure Usage

### IdentifyFITS Executable
To identify stars in an image and view the results, use the executable `IdentifyFITS`:
```cmd
cd hoku/bin
./IdentifyFITS [id-method] [image-file]
```

The first argument specifies the type of identification method to run. The second argument specifies the stars in 
the image. To run the Angle identification method on `my-image.csv` with a field of view of 20 degrees, enter the 
following:
```cmd
./IdentifyFITS angle my-image.csv
```

The image file must be formatted in a comma separated manner, specifying the centroid coordinates in terms of a 
standard FITS image:
```cmd
# Use the FITS coordinates of each centroid [top-left = (0, 0), bottom-right = (max-width, max-height)]
[x-coordinate-1],[y-coordinate-1]
[x-coordinate-2],[y-coordinate-2]
.
.
.
[x-coordinate-N],[y-coordinate-N]
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
Generate the test executables. The output will reside in the `test/bin` folder.
```cmd
cd hoku/build
cmake -G"Unix Makefiles" -DBUILD_TEST=ON ..
cd test
make -j8 install
```

TODO: Finish the Google Test generation.

## LaTeX Generation

TODO: Finish the LaTeX generation.