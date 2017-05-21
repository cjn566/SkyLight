// Gates
volatile bool move_gate = false;

// Soft Enums
#define ALL_SEXTANTS 6

// Objects
IntervalTimer movement_ticker;				// Timer to keep things a-ticking
CRGB ledsraw[6 * SEXTANT_LED_COUNT];	// Data structure for all LEDs
CHSV leds[6 * SEXTANT_LED_COUNT];


// Interrupt handler for gating all timed events
void mvmntTick() {
	if (move_gate) {
		//Serial.println("!!!!!!!!");
	}
	move_gate = true;
}

void setSV(uint8_t S, uint8_t V) {
	for (uint16_t i = 0; i < SEXTANT_LED_COUNT*6; i++)
	{
		leds[i].s = S;
		leds[i].v = V;
	}
}

// Light a particular box in one or all sextants
void lightBox(int sextant, int box, CHSV color) {

	// Ad Hoc fix for central box being in all six quadrants
	if (box == 0) {
		for (int s = 0; s < 6; s++)
		{
			for (int i = shape_start_addr[s][0]; i < shape_start_addr[s][1]; i++)
			{
				leds[i] = color;
			}
		}
	}
	else {
		// Determine if this is to light all sextants or just one.
		for (int s = (sextant == ALL_SEXTANTS ? 0 : sextant); s <= (sextant == ALL_SEXTANTS ? 5 : sextant); s++)
		{
			// Set color for all px indices in the box
			for (int i = shape_start_addr[s][box]; i < shape_start_addr[s][box + 1]; i++)
			{
				leds[i] = color;
			}
		}
	}
}


// Light all boxes in a particular level (depth) in one or all sextants
void lightLevel(int sextant, int level, CHSV color) {
	for (int j = 0; j < (level == 0 ? 2 : 4); j++) // There are only 2 shapes on level 0, but 4 shapes on all other levels
	{
		lightBox(sextant, levels[level][j], color);
	}
}

// Light all boxes of a particular shape in one or all sextants
void lightShapes(int sextant, int shape, CHSV color) {
	for (int j = 0; j < shape_count[shape]; j++)
	{
		lightBox(sextant, shapes[shape][j], color);
	}
}
