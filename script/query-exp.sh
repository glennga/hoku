#!/bin/bash

$HOKU_PROJECT_PATH/bin/PerformE angle query &
echo Running Angle Query Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE dot query &
echo Running Dot Angle Query Experiment: PID $!
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE sphere query &
echo Running Spherical Triangle Query Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE plane query &
echo Running Planar Triangle Query Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE pyramid query &
echo Running Pyramid Query Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE composite query &
echo Running Composite Pyramid Query Experiment: PID $!
pids+=($!)
sleep 3
wait

echo All Experiments Complete
