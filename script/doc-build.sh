#!/bin/bash

# Ensure that we have only 1 argument passed.
if [ "$#" -ne 1 ]; then
    echo "Usage: doc-build.sh [file to build]"
    exit 1
fi

# Build everything in the doc directory.
cd $HOKU_PROJECT_PATH/doc

# If "paper" is passed, then we have to build the bibliography as well.
if [[ "$1" = *"paper" ]]; then
    pdflatex $1.tex >> $1-build.log
    sleep 0.1

    bibtex $1 >> $1-build.log 2>&1
    sleep 0.1

    pdflatex $1.tex >> $1-build.log 2>&1
    sleep 0.1

    pdflatex $1.tex >> $1-build.log 2>&1
    sleep 0.1

else
    # Otherwise, just run pdflatex on the file passed.
    pdflatex $1 >> $1-build.log 2>&1
    sleep 0.1

    pdflatex $1 >> $1-build.log 2>&1
    sleep 0.1
fi

# Move everything but the LaTeX files and the PDF to some directory in build.
mkdir $HOKU_PROJECT_PATH/build/latex $HOKU_PROJECT_PATH/doc/pdf 2>/dev/null
mv *.aux *.bbl *.blg *.log *.out *.nav *.snm *.toc $HOKU_PROJECT_PATH/build/latex 2>/dev/null
mv *.pdf $HOKU_PROJECT_PATH/doc/pdf 2>/dev/null
