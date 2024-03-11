FROM ubuntu:latest

LABEL orca_eaa5a <orca_eaa5a@korea.ac.kr> 

RUN \
apt-get update && \
apt-get install -y wget

# install dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get install asciidoc source-highlight doxygen graphviz -y

# install cmake & g++ & boost-lib for build vsomeip-lib
RUN apt-get install cmake -y
RUN apt-get install g++ -y
RUN apt-get install git -y
RUN apt-get install libboost-all-dev -y

# install python3 & pip
RUN apt install python3
RUN wget 'https://bootstrap.pypa.io/get-pip.py'
RUN python3 get-pip.py

# Download vsomeip library
WORKDIR /usr/local/lib

# get v.3.4.10 vsomeip
RUN wget 'https://github.com/COVESA/vsomeip/archive/refs/tags/3.4.9-r1.tar.gz'
RUN mv '3.4.9-r1.tar.gz' 'vsomeip-3.4.9-r1.tar.gz'
# unpack
RUN tar -xzf 'vsomeip-3.4.9-r1.tar.gz'

# change working directory to build it
WORKDIR /usr/local/lib/vsomeip-3.4.9-r1

RUN mkdir ./build

WORKDIR /usr/local/lib/vsomeip-3.4.9-r1/build

RUN cmake ..
# replace g++ build option for stable building (add flag -Werror=stringop-overflow=0)
COPY ./patch/vsomeip/flags.make /usr/local/lib/vsomeip-3.4.9-r1/build/CMakeFiles/vsomeip3.dir/flags.make

RUN make

# vsomeip dist library path : /usr/local/lib/vsomeip-3.4.10/build

# setup vsomeip python wrapper
RUN mkdir /usr/local/pysomeip

WORKDIR /usr/local/pysomeip

COPY ./vsomeip_python.cpp .
COPY ./patch/pyvsomeip/setup.py .
COPY ./.clang-format .
COPY ./pysomeip .

RUN python3 setup.py build
RUN python3 setup.py install

