#!/bin/bash

$HOKU_PROJECT_PATH/bin/PerformE angle identification &
echo Running Angle Identification Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE dot identification &
echo Running Dot Angle Identification Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE sphere identification &
echo Running Spherical Triangle Identification Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE plane identification &
echo Running Planar Triangle Identification Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE pyramid identification &
echo Running Pyramid Identification Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE composite identification &
echo Running Composite Pyramid Identification Experiment: PID $!
pids+=($!)
sleep 3
wait

echo All Experiments Complete
