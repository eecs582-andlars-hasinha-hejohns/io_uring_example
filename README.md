# Requirements
Our implementation will only work on linux kernel version >= 5.11.x, but has only been tested on linux kernel version >= 6.2.x  

You will have to install liburing-dev onto your system as follows:  
`sudo apt install -y liburing-dev`

# Overview
It is recommended to build in the order that everything is listed (i.e., sample_program, libmonkey, glibc)  
## sample_program
The program that does the following:
* remove file 'sample.txt'
* open the file 'sample.txt'
  * Thus creating the file
* create a random string that has the following format
  * 'random number generated for run = XXXXX'
  * XXXXX is a randomly generated number
* write the random string to the file
* fsync file
* read from file and print contents to screen
* close file
  
You may have to modify the read/write bits on the file to view the content to verify what was reported as read is actually present in the file:  
`sudo chmod +rw sample.txt`  
`cat sample.txt`  
### building sample_program
`make`
### running sample_program
`./sample_program`
## libmonkey
This houses our implementation with io_uring that performs error checking and supports multi-threaded programs. It is implemented in C++ to allow us to use higher-level mechanisms such as 'thread_local'. Moreover, we have also implemented static user level bpftrace tracepoints (entry + exit) for all 5 of the syscalls we modified.
### build libmonkey
`make`  
### using libmonkey.so
We make use of the LD_PRELOAD mechanism to link programs against our implementation of the open, read, write, fsync, and close syscalls. In your terminal navigate to the 'libmonkey' folder. The following command is used to run a program with libmonkey.so:  
`LD_PRELOAD=./libmonkey.so ../relative/path/to/your_program`  
Alternatively, you can use libmonkey.so to run the provided sample program:  
`LD_PRELOAD=./libmonkey.so ../sample_program/sample_program`  
## glibc
This is a proof of concept where we showcase that io_uring can be embedded into glibc. Our modified version of glibc contains a basic implementation of io_uring without any multi-threaded support and proper error checking. We make no promises that this will work with all sample programs. It serves as proof that glibc is fundametally capable of linking against liburing. 
### build glibc
`cd glibc`  
`mkdir build`  
`cd build`  
The following command only needs to run once when a new build folder is created:  
`../src/configure --prefix=/usr`  
The above command may error out with something like so:  
`*** These critical programs are missing or too old: exampleA exampleB etc`  
Most likely all you will have to do is install them with the following command  
`sudo apt install -y exampleA exampleB etc`  
If the version is too old then go through the process of upgrading the version (most likely won't happen). Once you have installed all required libraries run the following command again:  
`../src/configure --prefix=/usr`  
Now we build... this can take up to 15 min!  
`make -j 4`  
### using glibc
In your terminal, navigate to the build folder you created and built glibc in 'glibc/build'. The following command is used to run a program using the built modified glibc:  
`./testrun.sh ../relative/path/to/your_program`  
Alternatively, you can use the modified glibc to run the provided sample program:  
`./testrun.sh ../../sample_program/sample_program`
