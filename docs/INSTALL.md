# Building and Installing

- [Building and Installing](#building-and-installing)
  - [Building](#building)
    - [Dependencies](#dependencies)
      - [Linux](#linux)
    - [Build](#build)
  - [Installing](#installing)

## Building

Currently only building on Linux systems is supported, if you wish to compile on another system, please reach out because help is needed for more builds/

### Dependencies

#### Linux

To get all the required dependencies, run the command below:

    sudo apt-get install build-essential libcurl4-openssl-dev libboost-log-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev libzmq3-dev -y

### Build

To build, make sure you have installed all the dependencies, then use the `make` command to build.

## Installing

To install, after you have built Lightning Rod, use `make install` to install lightning rod to your PATH.  You may need to use `sudo make install` depending on your permissions.

Afterwards use `lrod` to run Lighting Rod.
