/*
 * Copyright 2011 IPOL Image Processing On Line http://www.ipol.im/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file canny.c
 * @brief Canny filter
 *
 * @author Vincent Maioli <vincent.maioli@crans.org>
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "io_png.h"

/**
 * @brief unsigned char comparison

 static int cmp_u8(const void *a, const void *b)
 {
 return (int) (*(const unsigned char *) a - *(const unsigned char *) b);
 }
 */
/**
 * @brief main function call
 *
 */
int main(int argc, char *const *argv)
{
	size_t nx, ny;              /* data size */
	unsigned char *data;        /* input/output data */
	int channel;

	/* "-v" option : version info */
	if (2 <= argc && 0 == strcmp("-v", argv[1]))
	{
		fprintf(stdout, "%s version " __DATE__ "\n", argv[0]);
		return EXIT_SUCCESS;
	}
	/* wrong number of parameters : simple help info */
	if (3 != argc)
	{
		fprintf(stderr, "usage : %s in.png out.png\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* read the PNG image */
	if (NULL == (data = read_png_u8_rgb(argv[1], &nx, &ny)))
	{
		fprintf(stderr, "the image could not be properly read\n");
		return EXIT_FAILURE;
	}

	//TODO : Pour le moment : ne travaille que sur le premier canal

	//TODO : filtrage gaussien

	//TODO : sanity check on the pointers

	unsigned char* mask = malloc(sizeof(unsigned char) * nx * ny);

	double* grad = malloc(sizeof(double) * nx * ny);
	int* theta = malloc(sizeof(int) * nx * ny);

	//Valeur du gradient :

	double hgrad,vgrad;

	for(int x = 0 ; x < nx ; x++) {
		for(int y = 0 ; y < ny ; y++) {
			//Gradient horizontal
			hgrad = (x==0) ? (data[y*nx+1] - data[y*nx]) : ((x==nx-1) ? (data[y*nx+(nx-1)] - data[y*nx+(nx-2)]) : (data[y*nx+(x+1)] - data[y*nx+(x-1)]));
			//Gradient vertical
			vgrad = (y==0) ? (data[x+nx] - data[x]) : ((y==ny-1) ? (data[x+nx*(ny-1)] - data[x + nx*(ny-2)]) : (data[y*nx+x+1] - data[y*nx+x-1]));
			//Norme (L1) du gradient
			grad[y*nx+x] = fabs(hgrad) + fabs(vgrad); 
			//Direction du gradient
			// si vgrad est à zero :
			// hgrad neg : -pi/2 sinon pi/2
			// automatiquement fait par atan <3
			//Mais on veut pas (sinon on a pas -pi/4
			if(vgrad==0 ) 
				theta[y*nx+x] = 0; 
			else
				theta[y*nx+x] = floor((M_PI_2 + atan(hgrad / vgrad))/M_PI_4);
			
		}
	}

	//Suppression des non-maxima

	for(int x = 0 ; x < nx ; x++) {
		for(int y = 0 ; y < ny ; y++) {
			int t = theta[y*nx+x];
			int ex,ey;
			switch(t) {
				case 0:
					ex = 0;
					ey = 1;
					break;
				case 1:
					ex = 1;
					ey = 1;
					break;
				case 2:	
					ex = 1;
					ey = 0;
					break;
				case 3:
					ex = 1;
					ey = -1;
					break;
				default: 
printf("Une couille dans le potage : \ndirection= %d\ngrad= %g",t,grad[y*nx+x]);
					 return EXIT_FAILURE;
			}	
			double past =  grad[(x-ex)+nx*(y-ey)];
			double future = grad[(x+ex)+nx*(y+ey)];
			double present = grad[y*nx+x];
		//	mask[y*nx+x] = (present < past) ? 0 : ((present < future) ? 0 : data[x*y*nx+x]);
mask[y*nx + x] = (present < past) ? 0 : ((present < future) ? 0 : HUGE);
//if(mask[x*y*nx+x]==1) printf("Got one\n");

		}		
	}


	//TODO : rajouter hysteresis


	/* write the mask as a PNG image */
	write_png_u8(argv[2], mask, nx, ny, 1);

	free(data);
	free(mask);

	return EXIT_SUCCESS;

}
