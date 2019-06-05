Name  : Srinivas C Lingutla
UIN   : 655115444
NetID : slingu2

------------------------------ Homework 5 -----------------------------------

 === BUILD ====
 make all
 or
 gcc -std=c99 -pthread -D_SVID_SOURCE -o raceTest racetest.c

------------------------------

=== RUN ===
make run

runs with the following command
./raceTest 31 30 0.1 0.2 42 -noLock

------------------------------

=== CLEAN ===
make clean
- this will remove the executables

------------------------------

The program currently outputs my name, project, and my netid
along with the RaceTest output 

The user will have to enter the correct command line arguments for the program to run
The program will output the arguments, sleeptimes and the results along with the number of errors. 

-------------------------------
OPTIONAL ENHANCEMENTS

I have implemented the Qsort in my program which sorts the sleep times
in descending order so that the first threads sleep the longest

-------------------------------
FILES INCLUDED

FILES INCLUDED

├── HW5-361-Fall2017.pdf
├── makefile
├── racetest.cpp
└── readme.txt