# A dockerfile for locally testing the build process on Ubuntu 20.04
# To run, use the following command:
# docker build .

# Use the official image as a parent image
FROM ubuntu:22.04

# Set the working directory
WORKDIR /usr/src/app

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Install required apt packages
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get install -y \
    build-essential \
    cmake \
    qtbase5-dev \
    libopencv-dev \
    libjpeg-dev \
    libdc1394-dev \
    libopencv-dev \
    qt5-qmake \
    qtbase5-dev \
    qtbase5-dev-tools

# Run the build command, including tests
RUN qmake -makefile
RUN make

