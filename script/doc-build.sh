#!/bin/bash

# Ensure that we have only 1 argument passed.
if [ "$#" -ne 1 ]; then
    echo "Usage: doc-build.sh [file to build]"
    exit 1
fi

# Build everything in the doc directory.
cd $HOKU_PROJECT_PATH/doc

# If "paper" is passed, then we have to build the bibliography as well.
if [ "$1" = "paper" ]; then
    yes "" | pdflatex paper.tex >> paper-build.log
    sleep 1

    yes "" | bibtex paper >> paper-build.log 2>&1
    sleep 1

    yes "" | pdflatex paper.tex >> paper-build.log 2>&1
    sleep 1

    yes "" | pdflatex paper.tex >> paper-build.log 2>&1
    sleep 1
else
    # Otherwise, just run pdflatex on the file passed.
    yes '\n' | pdflatex $1 >> $1-build.log 2>&1
    sleep 1

    yes '\n' | pdflatex $1 >> $1-build.log 2>&1
    sleep 1
fi

# Move everything but the LaTeX files and the PDF to some directory in build.
mkdir $HOKU_PROJECT_PATH/build/latex 2>/dev/null
mv *.aux *.bbl *.blg *.log *.out *.nav *.snm *.toc $HOKU_PROJECT_PATH/build/latex 2>/dev/null
