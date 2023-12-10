# Requirements
Our implementation will only work on linux kernel version >= 5.11.x, but has only been tested on linux kernel version >= 6.2.x  

You will have to install liburing-dev onto your system as follows:  
`sudo apt install -y liburing-dev`

# Init
We first have to install the latest version of liburing and its headers onto our system.
From the root directory of the repo:
cd ./install_instructions
./install-liburing.sh

# Running the read example
cd read_example
mkdir build && cd build
cmake ../
make
./test_read

# Building glibc
cd glibc
mkdir build
cd build
The following command only needs to run once when a new build folder is created
../src/configure --prefix=/usr
The above command may error out with something like so:
*** These critical programs are missing or too old: exampleA exampleB
Most likely all you will have to do is install them with the following command
sudo apt install -y exampleA
If the version is too old then go through the process of upgrading the version (most likely won't happen)

Now we build... this can take up to 15 min!
make -j 4

# Using modified glibc
navigate to the build folder we created for glibc
make use of the ./testrun.sh to launch a program

example: ./testrun.sh ../../my_custom_app
where my_custom app is a program I wrote and is specified with a relative path to the folder we are currently in
