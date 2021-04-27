#!/bin/zsh

DOCKERDIR="/home/bene/BA"
SOURCE_DIR="/home/bene/BA/test"
#SOURCE_DIR=$PWD
TARGET_DIR="/home/bene/BA/trainingsdata"

docker build --rm -t md/quarz:latest $DOCKERDIR

runner() {
    for file in "$1"/*
    do
        if  [[ $file == *.wav ]]
        then
            wav=$(basename -s .wav $file)
            label=$(basename $PWD)
            directory="$TARGET_DIR"/"$label-$wav"
            mkdir -p $directory
            cp $file $directory
	    cd $directory
            docker run --rm -v $directory:/data -it md/quarz:latest $wav.wav
	    cd -
        else
            if [ -d $file ]
            then
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

#combiner $TARGET_DIR

