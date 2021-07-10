# Socket Programming
This repository contains code that enables users to create two nodes on a network with the aim of transferring a file. The ```server.c``` file in the Server directory contains the code that should be executed from the source directory, and the ```client.c``` file in the Client directory should be executed in the destination directory. 
### How to run
To setup the server, simply compile the file using ```gcc``` and run the executable. Once the server is setup, the client can be started in the same fashion. 
### Features
- A CLI for the client end with two simple commands, get and exit. To get a file simply run the command ```get <file1> <file2> ...``` and to terminate the connection run ```exit```. 
- Robust and dynamic error handling on both the client and server ends.
- Server error messages in case of missing files and/or connection interruptions.
- The server only supports a single connection, and other clients will have to wait in the queue. 
- Any file size is supported as the data is chunked and transmitted over the network. However only text files are currently supported. 
