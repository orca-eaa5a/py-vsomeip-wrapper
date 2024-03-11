FROM ubuntu:latest

LABEL orca_eaa5a <orca_eaa5a@korea.ac.kr> 

RUN \
apt-get update && \
apt-get install -y wget

# install dependencies
RUN sudo apt-get install asciidoc source-highlight doxygen graphviz -y

# install python3 & pip
RUN sudo apt install python3
RUN wget 'https://bootstrap.pypa.io/get-pip.py'
RUN python3 get-pip.py

# Download vsomeip library
WORKDIR /usr/local/lib

# get v.3.4.10 vsomeip
RUN wget 'https://github.com/COVESA/vsomeip/archive/refs/tags/3.4.10.zip'

# unpack
RUN tar -xzf 'vsomeip-3.4.10.tar.gz'

# change working directory to build it
WORKDIR /usr/local/lib/vsomeip-3.4.10

RUN mkdir ./build

WORKDIR /usr/local/lib/vsomeip-3.4.10/build

RUN cmake ..
# replace g++ build option for stable building
COPY ./patch/vsomeip/flags.make /usr/local/lib/vsomeip-3.4.10/build/CMakeFiles/vsomeip3.dir/flags.make

RUN make

# vsomeip dist library path : /usr/local/lib/vsomeip-3.4.10/build

# setup vsomeip python wrapper
RUN mkdir /usr/local/pysomeip

WORKDIR /usr/local/pysomeip

COPY ./vsomeip_python.cpp .
COPY ./setup.py .
COPY ./.clang-format .
COPY ./pysomeip .

RUN python3 setup.py build
RUN python3 setup.py install

