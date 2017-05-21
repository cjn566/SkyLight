// Map.h

#ifndef _MAP_h
#define _MAP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define SEXTANT_LED_COUNT 173

enum Shape
{
	HEXAGON,
	SM_TRIANGLE,
	LG_TRIANGLE,
	J_SHAPE,
	SM_CHEVRON,
	LG_CHEVRON,
	SM_TRUNC_TRI,
	LG_TRUNC_TRI
};

int shape_start_addr[6][12] =
{
	{0,5,17,35,50,65,80,119,131,143,155,173},
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
};


// How many of each shape are in a sextant. Needed to prevent for-loop spillover.
// this describes how long the inner array is for each inner array in 'shapes' below. 'hexagon' has 3 members, etc...
const uint8_t shape_count[8] = {3,1,3,3,4,2,1,1};

// Holds indices of shapes in shape_start_addr that correspond to like shapes
// the numbers in the below structure are describing the data in shape_start_addr above... they are saying that the indexes in the shape_start_addr at position 0, 2, and 15 are hexagons, etc...
const uint8_t shapes[8][4] = 
{
	{ 0,2,15 },			// HEXAGON		
	{ 3 },				// SM_TRIANGLE
	{ 8,9,10 },			// LG_TRIANGLE
	{ 4,5,6 },			// J_SHAPE
	{ 1,12,13,14 },		// SM_CHEVRON
	{ 11,16 },			// LG_CHEVRON
	{ 7 },				// SM_TRUNC_TRI
	{ 17 },				// LG_TRUNC_TRI
};


// Holds indices of shapes in shape_start_addr that correspond to levels
const uint8_t levels[5][4] =
{
	{ 0,15,0,0 },		// Closest to audience. Only 2 Shapes in this one, so last two elements are unused
	{ 1,12,13,14 },
	{ 2,7,11,16 },
	{ 4,5,6,17 },
	{ 3,8,9,10 }		// Farthest from audience.
};


// This function populates all the data for the rest of the pixel address array.
void mapInit() {
	for (int i = 1; i < 6; i++) {
		for (int j = 0; j < 12; j++)
		{
			shape_start_addr[i][j] = shape_start_addr[i - 1][j] + SEXTANT_LED_COUNT;
		}
	}
}

#endif

//const uint8_t coswave[60] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,2,2,3,3,4,4,5,4,5,5,5,6,5,5,5,5,5,4,4,4,3,3,2,2,1,1,1 };
const uint8_t coswave[60] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };