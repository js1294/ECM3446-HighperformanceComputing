# ECM3446-HighperformanceComputing

Developer
-----------
Jack Shaw- https://github.com/js1294

Description
-----------
This was a fourth year module with an assignment to calculate a numerical solution to the advection equation to simulate the movement of a cloud of material in the atmospheric boundary layer. 

Requirements
-----------
This is written in ISO C standard from 1999.

Running the Project
-----------
To be able to run the project you must first compile and link the programs.
This can be done by executing the compileSim shell or using this command on linux:

gcc -o advection2D -std=c99 advection2D.c -lm

Once the it has been compiled you can run the executable runSimulations, by running .\advection2D on linux.
