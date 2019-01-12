#!/bin/bash

$HOKU_PROJECT_PATH/bin/PerformE angle reduction &
echo Running Angle Reduction Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE dot reduction &
echo Running Dot Angle Reduction Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE sphere reduction &
echo Running Spherical Triangle Reduction Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE plane reduction &
echo Running Planar Triangle Reduction Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE pyramid reduction &
echo Running Pyramid Reduction Experiment: PID $!
pids+=($!)
sleep 3
wait

$HOKU_PROJECT_PATH/bin/PerformE composite reduction &
echo Running Composite Pyramid Reduction Experiment: PID $!
pids+=($!)
sleep 3
wait

echo All Experiments Complete
