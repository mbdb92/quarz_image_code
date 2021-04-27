#!/bin/bash

SOURCE_DIR=$PWD
TARGET_DIR="/srv/trainingsdata"
CODE_DIR="/srv/code"
runner() {
    for file in "$1"/*
    do
        if  [[ $file == *.wav ]]
        then
            wav=$(basename $file)
            label=$(basename $PWD)
            directory="$TARGET_DIR"/"$label-$wav"
            mkdir -p $directory
            cp $file $directory
	    cp $CODE_DIR/quarz.out $directory
	    cd $directory
	    ./quarz.out $wav
	    rm quarz.out
	    cd -
            #echo $file
        else
            if [ -d $file ]
            then
                #echo $file
                cd $file
                echo $PWD
                runner ${file}
                cd --
            fi
        fi
    done
}

runner $SOURCE_DIR

combiner() {
    for file in "$1"/*
    do
        if [ -d $file ]
        then
            cd $file
            combiner ${file}
            cd -
        else
            nodir=1
        fi
    done
    if [ $nodir ]
    then
        dirname=$(basename $PWD)
        echo $PWD $dirname
        magick convert *.ppm +append $dirname.jpg
    fi
}

combiner $TARGET_DIR

