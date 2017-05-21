
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// Directives
#define USE_OCTOWS2811
// External Libraries
#include "OctoWS2811.h"
#include "FastLED.h"
#include "IntervalTimer.h"
#include "ArtMap.h"
#include "Skylight.h"
// Settings
#define NUM_BOXES_PER_SEXTANT 10

#define NUM_BOXES NUM_BOXES_PER_SEXTANT*6+1

#define MVMNT_TICK_HZ 30
#define NUM_ANIMS 3
#define TRANS_TIME 5
#define FADE_RATE (UINT8_MAX / TRANS_TIME)

#define OVERLAY_WAIT_TIME 4
#define OVERLAY_FADE_TIME 1

#define FADE_START_TIMER toSecs(OVERLAY_FADE_TIME)

// Settings - Animations
//		in to out spectrum
#define HUE_DIF_COEF 13
uint16_t toSecs(uint16_t in);
// Variables
uint16_t animtimer[NUM_ANIMS];
uint8_t hue = 0;
uint8_t curr_anim = 1;
uint16_t anim_reset[NUM_ANIMS] = { toSecs(10), toSecs(10), toSecs(10)};
bool transition = false;
bool trans_fade = true;
unsigned int trans_count = TRANS_TIME;


uint16_t toSecs(uint16_t in){
	return in * MVMNT_TICK_HZ;
}	


// ANIMATIONS
void in_out_rainbow() {
	for (int i = 0; i < 11; i++)
	{
		lightBox(ALL_SEXTANTS, i, CHSV(hue + (i*HUE_DIF_COEF), 255, 255));
	}
	hue++;
}

//void 
class Radar {
public:
	Radar() {}
	void go() {
		countdown--;
		if (!countdown) {
			lightBox(sextant, box, CHSV(hue, 255, 255));
			box++;
			if (box == (NUM_BOXES_PER_SEXTANT + 1)) {
				box = 0;
				sextant = (sextant + 1) % 6;
			}
			hue += 2;
			countdown = cd_reset;
		}
	}
private:
	const uint8_t boxmap[NUM_BOXES_PER_SEXTANT] = {};
	const uint8_t cd_reset = 3;
	uint8_t sextant = 0;
	uint8_t box = 0;
	uint8_t hue = 0;
	uint8_t countdown = cd_reset;
};

class Breathe {
public:
	Breathe(){}
	void start() {
		for (int i = 0; i < 11; i++)
		{
			box_hue[i] = 0;
			waiting[i] = true;
			counter[i] = toSecs(3)-(20 * i);
		}
		Serial.println();

	}

	void go() {
		for (int i = 0; i < 11; i++)
		{
			if (!waiting[i])
			{
				box_hue[i]+=2;
			}
			lightBox(ALL_SEXTANTS, i, CHSV(box_hue[i], 255, 255));
			counter[i]--;
			if (!counter[i]) {
				if (waiting[i]) {
					waiting[i] = false;
					counter[i] = toSecs(2);
				}
				else {
					waiting[i] = true;
					counter[i] = toSecs(3);
				}
			}
		}
	}
private:
	uint8_t box_hue[11];
	uint16_t counter[11]; 
	bool waiting[11];
};

class ColorInjector
{
public:
	void inject(uint8_t hue=0)
	{
		uint8_t target = random(0, NUM_BOXES);
		overlays[target].hue = hue;
		overlays[target].timer = toSecs(OVERLAY_WAIT_TIME + OVERLAY_FADE_TIME);
	}

	void doOverlay()
	{
		for (uint8_t i = 0; i < NUM_BOXES; i++)
		{
			if (overlays[i].timer)
			{
				uint8_t sextant = i == 0 ? 0 : (int)((i - 1) / NUM_BOXES_PER_SEXTANT);
				uint8_t box = i == 0 ? 0 : ((i - 1) % NUM_BOXES_PER_SEXTANT) + 1;

				CHSV newcolor = CHSV(overlays[i].hue, 255, 255);
				if (overlays[i].timer <= FADE_START_TIMER) {
					uint8_t fraction = 255 * (FADE_START_TIMER / overlays[i].timer);
					CHSV oldcolor = leds[shape_start_addr[sextant][box]];
					newcolor = blend(oldcolor, CHSV(overlays[i].hue, 255, 255), fraction);
				}
				lightBox( sextant, box, newcolor);
				overlays[i].timer--;
			}
		}
	}
private:

	struct Overlay
	{
		uint8_t timer;
		uint8_t hue;
	};

	Overlay overlays[NUM_BOXES];

};

ColorInjector injector;
Radar radar;
Breathe breathe;

void setup() {
	LEDS.addLeds<OCTOWS2811>(ledsraw, SEXTANT_LED_COUNT);
	mapInit();

	Serial.begin(115200);
	movement_ticker.begin(mvmntTick, (1000000 / MVMNT_TICK_HZ));
	for (int i = 0; i < NUM_ANIMS; i++)
	{

		animtimer[i]= anim_reset[i];
	}
	
	// Initialize All Leds
	for (int i = 0; i < SEXTANT_LED_COUNT * 6; i++)
		leds[i] = CHSV(80, 255, 255);
}

uint8_t bright = 255;

void loop() {
	if (move_gate) {
		// Convert HSV to RGB
		for (uint16_t i = 0; i < SEXTANT_LED_COUNT * 6; i++)
		{
			ledsraw[i] = leds[i];
		}
		FastLED.show();
		move_gate = false;
		
		/*
		// Led 0 to red
		leds[0] = CHSV(0, 255, 255);
		
		// Led 0 to yellow
		leds[1] = CHSV(80, 255, 255);

	
	
		*///   <--------- This block comment is just for testing pixel 0 fuckiness. move down when ready to test patterns.
		
		// Switch animationsx
		if (!animtimer[curr_anim]--) {
			animtimer[curr_anim] = anim_reset[curr_anim];
			curr_anim = (curr_anim + 1) % NUM_ANIMS;
			if (curr_anim == 2)
				breathe.start();
		}
		//animtimer[curr_anim]--;
		// ! switch animations

		switch (curr_anim)
		{
		case 0: 
			in_out_rainbow();
			//Serial.print("0 - "); //debug
			break;
		case 1: 
			radar.go();
			//Serial.print("1 - "); //debug
			break;
		case 2:
			breathe.go();
			break;
		default:
			break;
		}
		
		injector.doOverlay();
		
		
		/* <-------  block comment starts here
		// Test dark boxes
		lightBox(0, 1, CHSV(0, 255, 0));
		lightBox(2, 1, CHSV(0, 255, 0));
		lightBox(4, 1, CHSV(0, 255, 0));
		
		
		// Transition between animations
		if (transition) {
			// Fade to half brightness, zero saturation. FADE_RATE * full trans_count = ~256
			uint8_t sat = UINT8_MAX - (FADE_RATE*trans_count);
			uint8_t val = UINT8_MAX - (FADE_RATE*trans_count)/2;
			setSV(sat, val);

			Serial.print(sat); Serial.print(", "); Serial.println(val);

			// Switch from fading out to fading in, then end transistion
			if (trans_fade) {
				trans_count++;
				if (trans_count == TRANS_TIME)
				{
					trans_fade = false;
				}
			}
			else{
				trans_count--;
				if (!trans_count) {
					transition = false;
				}
			}
		}
		// Start transistion
		else if (anim_counter == TRANS_TIME+1) {
			transition = true;
			trans_fade = true;
			trans_count = 0;
		}
		// ! transition between animations
		*/
		

	}
}

