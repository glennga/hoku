# Hoku (Image Star Recognition)

------

## Motivation

In 1976, two researchers at JPL published a paper regarding a device that could detect stars using a CCD camera. The first star tracker was born, and the authors noted the use of this device for accurate attitude determination. The approach to attitude determination at the time had been to use two sensors (most likely cameras to detect the Sun) to draw two vectors to a known reference and extract an orientation from this. Star trackers allow the use of one sensor to draw a numerous amount of vectors to a known reference, increasing the accuracy of the estimated orientation.

Fast forward to today, the number of solutions to the star identification problem has grown quite a bit. There have been improvements in various aspects of star tracking, but a good portion of this research has been separate from each other. The goal of this project is to create an optimal complete solution for the lost-in-space version of the problem based on strengths of existing algorithms and emperical observation. 

## Existing Implementations

1. LIS Stellar Attitude Acquisition Method
2. Cole & Crassidus Planar Triangles Method
3. Cole & Crassidus Spherical Triangles Method
4. Mortari Pyramid Method
5. Astrometry.net Method

## Problem

All algorithms will be modified to solve the problem below.

Given a set of three-dimensional points **N** within **fov** degrees from a focus point **q**, as well as feature data/operation parmeters **S** from a star catalog, output a set **U** containing **N** with star IDs attached **H**.

```
(N,fov,S) -> (solve) -> U where U_i = [N_i,H_i]
```

## Repo Contents

This repository is for all procedures and documentation relating to the creation, testing, and analysis of different methods of star recognition. This project uses the Yale BSC for the data set and does not deal with the hardware implementation of the detector. The items below are included in this repo.

1. Log describing steps, thoughts, and motivation.
2. Generation of test data from BSC to use for problem. 
3. Implementations of existing algorithms to solve problem.
4. Tests of all algorithms under introduction of various false data.
5. Empirical analysis of correctness, storage, and speed for each algorithm.

## Usage

First things first, clone the repository into your desired directory.

```
> git clone https://github.com/glennga/hoku.git
```



#### Generating Documentation

To generate PDF's from the TeX files, navigate to the documentation folder and
run `pdflatex`.

```
> cd doc
(doc)> pdflatex main.tex --aux-directory=build --output-directory
```