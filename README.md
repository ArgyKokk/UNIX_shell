# UNIX_shell

Design of a simple UNIX shell in C. This project is part of the course "Operating Systems"
AUTH , December 2018

## Operation
 The shell operates in 2 modes. 
 1) Interactive mode : the user executes one command at a time. Using the dellimeters ';' or '&&' the user may enter one or more commands that will be executed sequentially.
 2) Batch mode: the user enters a batchfile containing all the commands that will be executed sequentially till EOF.
 
 ## Run
 Create the object file
 ``` bash 
 make 
 ```
 Execute
 ``` bash
 ./myShell
 ```
 or
 ```bash
 ./myShell batchfile
 ```
 
 
 

