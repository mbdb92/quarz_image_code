FROM debian:buster

RUN apt -y update && apt -y upgrade
RUN apt -y install libfftw3-3 libfftw3-bin libfftw3-dev libmagickcore-6-headers build-essential clang gcc libc6-dev coreutils

RUN mkdir /working && mkdir /data
ADD code/ /working

RUN cd /working && make recorded && cp quarz.out /usr/bin/

#CMD "cd /data && /usr/bin/quarz.out *"
VOLUME ["/data"]
WORKDIR /data
ENTRYPOINT ["/usr/bin/quarz.out"]
#ENTRYPOINT ls /working && ls /working/data && exec /working/quarz.out *.wav && exit 0
