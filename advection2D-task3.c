/*******************************************************************************
2D advection example program which advects a Gaussian u(x,y) at a fixed velocity

Outputs: initial.dat - inital values of u(x,y) 
		 final.dat   - final values of u(x,y)

		 The output files have three columns: x, y, u

		 Compile with: gcc -o advection2D -std=c99 advection2D.c -lm

Notes: The time step is calculated using the CFL condition

********************************************************************************/

/*********************************************************************
					 Include header files 
**********************************************************************/

#ifdef omp
	#include <omp.h>
#endif
#include <stdio.h>
#include <math.h>

/*********************************************************************
					 Structures
**********************************************************************/

struct point {
	float x;
	float y;
};

/*********************************************************************
					 Main function
**********************************************************************/

int main() {
	/* Grid properties */
	const int NUM_X = 1000;                                                     // Number of x points in the grid
	const int NUM_Y = 1000;                                                     // Number of y points in the grid
	const struct point MIN_POINTS = { 0.0f, 0.0f };                             // Minumum points value in the grid
	const struct point MAX_POINTS = { 30.0f, 30.0f };                           // Maximum points value in the grid
	const struct point POINTS_DIST = { (MAX_POINTS.x - MIN_POINTS.x) / NUM_X,
									   (MAX_POINTS.y - MIN_POINTS.y) / NUM_Y }; // Distance between each point in the grid
  
	/* Parameters for the Gaussian initial conditions */
	const struct point CENTRE = { 3.0f, 15.0f };         // Centre
	const struct point WIDTH = { 1.0f, 5.0f };           // Width
	const struct point WIDTH_SQ = { WIDTH.x * WIDTH.x,
									WIDTH.y * WIDTH.y }; // Width Squared
	struct point coord_sq;                               // X and Y coordinates squared

	/* Boundary conditions */
	const float BOUND_LEFT = 0.0f;  // Left boundary value
	const float BOUND_RIGHT = 0.0f; // Right boundary value
	const float BOUND_LOWER = 0.0f; // Lower boundary value
	const float BOUND_UPPER = 0.0f; // Upper boundary value
  
	/* Time stepping parameters */
	const float CFL = 0.9f;      // Courant–Friedrichs–Lewy (CFL) number used to ensure stability
	const int NUM_STEPS = 800;  // Number of time-steps

	/* Velocity parameters*/
	float vel_x[NUM_X + 2];
	const float VEL_Y = 0.0f;
	float max_vel_x;
	float height;
	const float FRICT_VEL = 0.2f;
	const float ROUGH_LEN = 1.0f;
	const float VON_KAR = 0.41f;

	/* Calculate time step using the CFL condition. */
	/* The fabs function gives the absolute value in case the velocity is -ve. */
	float time_step;

	/* Arrays to store variables. These have number of points plus
	two elements to allow boundary values to be stored at both ends. */
	float x[NUM_X + 2];               // X-axis values
	float y[NUM_Y + 2];               // Y-axis values
	float u[NUM_X + 2][NUM_Y + 2];    // Array of u values
	float dudt[NUM_X + 2][NUM_Y + 2]; // Rate of change of u

	/* File pointers for writing results to */
	FILE* initial_file;
	FILE* final_file;

	/*** Place x points in the middle of the cell ***/
	#pragma omp parallel for default(shared) 
	for (int xi = 0; xi < NUM_X + 2; xi++) {
		x[xi] = ((float) xi - 0.5f) * POINTS_DIST.x;
	}

	/*** Place y points in the middle of the cell ***/
	#pragma omp parallel for default(shared)
	for (int yi = 0; yi < NUM_Y + 2; yi++) {
		y[yi] = ((float) yi - 0.5f) * POINTS_DIST.y;
	}

	/*** Calculate the x velocity using a logarithmic profile ***/
	if (VEL_Y == 0.0f) {
		#pragma omp parallel for default(shared) private(height)
		for (int yi = 0; yi < NUM_Y + 2; yi++) {
			height = y[yi];
			if (height <= ROUGH_LEN) {
				vel_x[yi] = 0.0f;
				continue;
			}
			vel_x[yi] = (FRICT_VEL / VON_KAR) * log(height / ROUGH_LEN);
		}
	}
	else {
		#pragma omp parallel for default(shared)
		for (int i = 0; i < NUM_Y + 2; i++) {
			vel_x[i] = 1.0f;
		}
	}

	/*** Calculate the maximum velocity to then be used to calculate the timestep***/
	#pragma omp parallel for default(shared)
	for (int i = 0; i < NUM_Y + 2; i++) {
		if (vel_x[i] > max_vel_x) {
			max_vel_x = vel_x[i];
		}
	}

	time_step = CFL / ((fabs(max_vel_x) / POINTS_DIST.x) + (fabs(VEL_Y) / POINTS_DIST.y));

	/*** Report information about the calculation ***/
	printf("Grid spacing x      = %g\n", POINTS_DIST.x);
	printf("Grid spacing y      = %g\n", POINTS_DIST.y);
	printf("CFL number          = %g\n", CFL);
	printf("Time step           = %g\n", time_step);
	printf("No. of time steps   = %d\n", NUM_STEPS);
	printf("End time            = %g\n", time_step * (float)NUM_STEPS);
	printf("Distance advected x = %g\n", max_vel_x * time_step * (float)NUM_STEPS);
	printf("Distance advected y = %g\n", VEL_Y * time_step * (float)NUM_STEPS);


	/*** Set up Gaussian initial conditions ***/
	// The variable coord_sq can be private since it is only calculated and used in each thread.
	// It does not need to be shared between them. 
	#pragma omp parallel for default(shared) private(coord_sq)
	for (int xi = 0; xi < NUM_X + 2; xi++) {
		for (int yi = 0; yi < NUM_Y + 2; yi++) {
			coord_sq.x = (x[xi] - CENTRE.x) * (x[xi] - CENTRE.x);
			coord_sq.y = (y[yi] - CENTRE.y) * (y[yi] - CENTRE.y);
			u[xi][yi] = exp(-1.0 * ((coord_sq.x / (2.0 * WIDTH_SQ.x)) +
									(coord_sq.y / (2.0 * WIDTH_SQ.y))));
		}
	}

	/*** Write array of initial u values out to file ***/
	initial_file = fopen("initial.dat", "w");

	if (initial_file == NULL) {
		printf("\nError cannot open file initial.dat");
		return 1;
	}

	// Cannot be in parallel since the writing to file must be done in order.
	// This is not possible when executed in parallel as threads will executed in order.
	for (int xi = 0; xi < NUM_X + 2; xi++) {
		for (int yi = 0; yi < NUM_Y + 2; yi++) {
			fprintf(initial_file, "%g %g %g\n", x[xi], y[yi], u[xi][yi]);
		}
	}
	fclose(initial_file);

	/*** Update solution by looping over time steps ***/
	// Cannot be in parallel since there is a flow dependancy.
	// Each loop requires the values of u to have been updated by the previous step.
	// If this has not been done then the current values of u cannot be calculated correctly.
	for (int step = 0; step < NUM_STEPS; step++) {
		/*** Apply boundary conditions at u[0][:] and u[NUM_X+1][:] ***/
		#pragma omp parallel for default(shared)
		for (int yi = 0; yi < NUM_Y + 2; yi++) {
			u[0][yi] = BOUND_LEFT;
			u[NUM_X + 1][yi] = BOUND_RIGHT;
		}

		/*** Apply boundary conditions at u[:][0] and u[:][NUM_Y+1] ***/
		#pragma omp parallel for default(shared)
		for (int xi = 0; xi < NUM_X + 2; xi++) {
			u[xi][0] = BOUND_LOWER;
			u[xi][NUM_Y + 1] = BOUND_UPPER;
		}
	
		/*** Calculate rate of change of u using leftward difference ***/
		/* Loop over points in the domain but not boundary values */
		#pragma omp parallel for default(shared)
		for (int xi = 1; xi < NUM_X + 1; xi++) {
			for (int yi = 1; yi < NUM_Y + 1; yi++) {
				dudt[xi][yi] = -((vel_x[yi] * ((u[xi][yi] - u[xi - 1][yi]) / POINTS_DIST.x)) + (VEL_Y * ((u[xi][yi] - u[xi][yi - 1]) / POINTS_DIST.y)));
			}
		}
	
		/*** Update u from t to t+dt ***/
		/* Loop over points in the domain but not boundary values */
		#pragma omp parallel for default(shared)
		for	(int xi = 1; xi < NUM_X + 1; xi++) {
			for (int yi = 1; yi < NUM_Y + 1; yi++) {
				u[xi][yi] += dudt[xi][yi] * time_step;
			}
		}
	}
  
	/*** Write array of final u values out to file ***/
	final_file = fopen("final.dat", "w");

	if (final_file == NULL) {
		printf("\nError cannot open file final.dat");
		return 1;
	}

	// Cannot be in parallel since the writing to file must be done in order.
	// This is not possible when executed in parallel as threads will executed in order.
	for (int xi = 0; xi < NUM_X + 2; xi++) {
		for (int j = 0; j < NUM_Y + 2; j++) {
			fprintf(final_file, "%g %g %g\n", x[xi], y[j], u[xi][j]);
		}
	}
	fclose(final_file);

	return 0;
}

/* End of file ******************************************************/
