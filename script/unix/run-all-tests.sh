#!/bin/bash

for f in *Test*; do
    bash "$f" -H
done