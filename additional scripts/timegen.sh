#!/bin/bash

gen_times() {
    cd code
    i=0
    while [ $i -le 1000 ]; do
        echo $i
        TIMESTAMP=$(date +%F-%T)
        ./quarz.out >> ../timeruns/$TIMESTAMP
        sleep 10
        ((i++))
    done
}

analyse() {
    for file in ./timeruns/*; do
        cat $file | grep fft | sed -e 's/^.*total //' >> fft_time
        cat $file | grep magick | sed -e 's/^.*total //' >> magick_time
        cat $file | grep fft | sed -e 's/^.*used //' | sed -e 's/ to .*$//' >> fft_time_start
        cat $file | grep magick | sed -e 's/^.*used //' | sed -e 's/ to .*$//' >> magick_time_start
        cat $file | grep fft | sed -e 's/^.* to //' | sed -e 's/,.*$//' >> fft_time_end
        cat $file | grep magick | sed -e 's/^.* to //' | sed -e 's/,.*$//' >> magick_time_end
    done

    SUMME=$(addup fft_time)
    LINECOUNT=$(cat fft_time | wc -l)
    echo fft:
    echo Summe:$SUMME , Linecount:$LINECOUNT
    echo "scale=2 ; $SUMME / $LINECOUNT" | bc
    SUMME=$(addup magick_time)
    LINECOUNT=$(cat magick_time | wc -l)
    echo magick:
    echo Summe:$SUMME , Linecount:$LINECOUNT
    echo "scale=2 ; $SUMME / $LINECOUNT" | bc
    SUMME=$(addup fft_time_start)
    LINECOUNT=$(cat fft_time | wc -l)
    echo fft_start:
    echo Summe:$SUMME , Linecount:$LINECOUNT
    echo "scale=2 ; $SUMME / $LINECOUNT" | bc
    SUMME=$(addup magick_time_start)
    LINECOUNT=$(cat fft_time | wc -l)
    echo magick_start:
    echo Summe:$SUMME , Linecount:$LINECOUNT
    echo "scale=2 ; $SUMME / $LINECOUNT" | bc
    SUMME=$(addup fft_time_end)
    LINECOUNT=$(cat fft_time | wc -l)
    echo fft_end:
    echo Summe:$SUMME , Linecount:$LINECOUNT
    echo "scale=2 ; $SUMME / $LINECOUNT" | bc
    SUMME=$(addup magick_time_end)
    LINECOUNT=$(cat fft_time | wc -l)
    echo magick_end:
    echo Summe:$SUMME , Linecount:$LINECOUNT
    echo "scale=2 ; $SUMME / $LINECOUNT" | bc
    SUMME=$(addup fft_time_end)
    SUMME2=$(addup magick_time_start)
    LINECOUNT=$(cat fft_time | wc -l)
    echo in between:
    echo "scale=2 ; ($SUMME2 - $SUMME) / $LINECOUNT" | bc
    rm magick_time fft_time magick_time_start magick_time_end fft_time_start fft_time_end

}

analyse


