# ECM3446-HighperformanceComputing

Developer
-----------
Jack Shaw- https://github.com/js1294

Description
-----------
This was a fourth year module with an assignment to calculate a numerical solution to the advection equation to simulate the movement of a cloud of material in the atmospheric boundary layer.
This would only create a 2D slice of the cloud.

Task one was to create and parallise the a program to create a 2D cross-section of a cloud with OpenMP.

Task two was to add wind to the cloud creating an inital cloud and another cloud after 800 ticks.

Task three was to add vertical shear by increasing the wind with height.

Example output
-----------

A cloud without vertical shear due to constant wind

<img width="640" height="480" alt="final-task 2" src="https://github.com/user-attachments/assets/3c207249-a928-48d2-8190-013d58e87eb8" />

A cloud with vertical shear due to increasing wind with height

<img width="640" height="480" alt="final-task 3" src="https://github.com/user-attachments/assets/c0f3a90d-af8a-49c6-b524-01978335d2f5" />

Requirements
-----------
This is written in ISO C standard from 1999.

Running the Project
-----------
To be able to run the project you must first compile and link the programs.
This can be done by executing the compileSim shell or using this command on linux:

Serial: gcc -o advection2D -std=c99 advection2D.c -lm

Parallel: gcc -fopenmp -o advection2D -std=c99 advection2D.c -lm

Once the it has been compiled you can run the executable runSimulations, by running .\advection2D on linux.
