# Introduction
A linux based file-transfer system in terminal. 
Share Files Over A Network 



# Use

```
./build.sh
cd build
```
run server 

```./server [ip] ```
default port can be changed in the file server.c

run client

```./client [ip] [port]```

by default port is 4000 

every in built command has a prefix of 's'

# Commands

shelp -> Lists all the in built commands 

sopen -> it changes the directory/file

sls -> shows the current directory or if file it prints the content of file

squit -> exit 

sclear -> clear the screen 

sload -> download file from the server it requires path and name of file as  arguments

sstats -> print's detailed info of a file

# features
Download File From Server 

Open File From Server 

File Info 

