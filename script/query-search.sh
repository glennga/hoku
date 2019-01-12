#!/bin/bash

echo Setting Up CONFIG.ini
cp $HOKU_PROJECT_PATH/CONFIG.ini $HOKU_PROJECT_PATH/CONFIG.ini.bak
sed -i "s/\(samples * = *\).*/\1 500/" $HOKU_PROJECT_PATH/CONFIG.ini
sed -i "s/\(sl * = *\).*/\1 30/" $HOKU_PROJECT_PATH/CONFIG.ini
sed -i "s/\(ss-iter * = *\).*/\1 1/" $HOKU_PROJECT_PATH/CONFIG.ini

echo Running Linear Search for: Angle
for d in {1..-16..-1}
do
    sed -i "s/\(angle-1 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
    $HOKU_PROJECT_PATH/bin/PerformE angle query &
    sleep 3

    if [ $d = -3 ] || [ $d = -8 ] || [ $d = -13 ]; then
        wait
    fi
done
wait
sleep 3

echo Running Linear Search for: Pyramid
for d in {1..-16..-1}
do
    sed -i "s/\(pyramid-1 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
    $HOKU_PROJECT_PATH/bin/PerformE pyramid query &
    sleep 3

    if [ $d = -3 ] || [ $d = -8 ] || [ $d = -13 ]; then
        wait
    fi
done
wait
sleep 3

echo Running Linear Search for: Sphere
for d in {1..-16..-1}
do
    sed -i "s/\(sphere-1 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
    sed -i "s/\(sphere-2 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini

    $HOKU_PROJECT_PATH/bin/PerformE sphere query &
    sleep 3

    if [ $d = -3 ] || [ $d = -8 ] || [ $d = -13 ]; then
        wait
    fi
done
wait
sleep 3

echo Running Linear Search for: Plane
for d in {1..-16..-1}
do
    sed -i "s/\(plane-1 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
    sed -i "s/\(plane-2 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini

    $HOKU_PROJECT_PATH/bin/PerformE plane query &
    sleep 3

    if [ $d = -3 ] || [ $d = -8 ] || [ $d = -13 ]; then
        wait
    fi
done
wait
sleep 3

echo Running Linear Search for: Dot
for d in {1..-16..-1}
do
   sed -i "s/\(dot-1 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
   sed -i "s/\(dot-2 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
   sed -i "s/\(dot-3 * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
   $HOKU_PROJECT_PATH/bin/PerformE dot query &
   sleep 3

   if [ $d = -3 ] || [ $d = -8 ] || [ $d = -13 ]; then
       wait
   fi
done
wait
sleep 3

echo Cleaning Up CONFIG.ini
mv $HOKU_PROJECT_PATH/CONFIG.ini.bak $HOKU_PROJECT_PATH/CONFIG.ini
