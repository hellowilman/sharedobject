# SharedObject 
This is a c++ verion library to implement the Adobe Action Script SharedObject function. It is a C-S design software which provide a server and multiple clients. 

## Pre request
It require zmq for build the library. You can download zmq from http://zeromq.org/ 

## Compile from source 
1. download the source code and unzip / tar 
```bash 
wget xxxxxxxx -o so.tar.gz 
tar -zxvf so.tar.gz 
```
2. compile 
### Linux 
Please install zeromq > 4.2

```bash
cd so 
./configure
make
```
### Windows 
Please open  vs/so.sln to build the library. (Be sure to add zmq library to the system environment)
If you have install QT, you can also use QT creator to build the library 
3. add library to you project and build it, enjoy it! 

