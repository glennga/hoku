#!/bin/bash

echo Setting Up CONFIG.ini
cp $HOKU_PROJECT_PATH/CONFIG.ini $HOKU_PROJECT_PATH/CONFIG.ini.bak
sed -i "s/\(samples * = *\).*/\1 1000/" $HOKU_PROJECT_PATH/CONFIG.ini
sed -i "s/\(ss-iter * = *\).*/\1 8/" $HOKU_PROJECT_PATH/CONFIG.ini
sed -i "s/\(es-iter * = *\).*/\1 5/" $HOKU_PROJECT_PATH/CONFIG.ini

echo Running Linear Search for: Sigma4
for d in {2..-16..-1}
do
    sed -i "s/\(so * = *\).*/\11.0e$d/" $HOKU_PROJECT_PATH/CONFIG.ini
    $HOKU_PROJECT_PATH/bin/PerformE angle overlay &
    sleep 3

    if [ $d = -4 ] || [ $d = -9 ] || [ $d = -14 ]; then
        wait
    fi
done
wait

echo Cleaning Up CONFIG.ini
mv $HOKU_PROJECT_PATH/CONFIG.ini.bak $HOKU_PROJECT_PATH/CONFIG.ini
