
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


// DEBUG MODE???
const bool DEBUG = true;
const bool DEBUG_CONTROLLER_INPUT = true;


// Directives
#define USE_OCTOWS2811
// External Libraries
#include "OctoWS2811.h"
#include "FastLED.h"
#include "IntervalTimer.h"
#include "ArtMap.h"
#include "Skylight.h"
#include "Bounce2.h"
#include "Encoder.h"

// Settings
#define NUM_BOXES_PER_SEXTANT 10
#define MVMNT_TICK_HZ 30
//#define ANIM_COUNT (120 * MVMNT_TICK_HZ)
#define NUM_ANIMS 3
#define TRANS_TIME 5
#define FADE_RATE (UINT8_MAX / TRANS_TIME)

// Settings - Animations
//		in to out spectrum
#define HUE_DIF_COEF 13


// Controller pins
const int rotEncoderPinA = 9;
const int rotEncoderPinB = 10;
const int buttonPin = 11;     // the number of the pushbutton pin


// Controller objects
Bounce button_debounced = Bounce(); 
Encoder RotaryEncoder(rotEncoderPinA, rotEncoderPinB);
long oldEncoderPosition = -999;
long newEncoderPosition = 0;


// Variables
uint16_t animtimer[NUM_ANIMS];
uint8_t hue = 0;
uint8_t curr_anim = 1;
uint16_t anim_reset[3] = { 120 * MVMNT_TICK_HZ, 12 * MVMNT_TICK_HZ, 120 * MVMNT_TICK_HZ };
bool transition = false;
bool trans_fade = true;
unsigned int trans_count = TRANS_TIME;


// Loggers that only log if you have the DEBUG flag set to true
void log(const char *message) {
  if(DEBUG) {  
    Serial.println(message);
  }
}

void log(bool message_type, const char *message) {
  if(message_type) {
    log(message);
  }
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
			counter[i] = 400-(20 * i);
		}
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
					counter[i] = 60;
				}
				else {
					waiting[i] = true;
					counter[i] = 400;
				}
			}
		}
	}
private:
	uint8_t box_hue[11];
	uint16_t counter[11]; 
	bool waiting[11];
};

Radar radar;
Breathe breathe;

void setup() {
	LEDS.addLeds<OCTOWS2811>(ledsraw, SEXTANT_LED_COUNT);
	mapInit();

  // set up the controller
  pinMode(rotEncoderPinA, INPUT_PULLUP);
  pinMode(rotEncoderPinB, INPUT_PULLUP);
  pinMode(buttonPin, INPUT);

  button_debounced.attach(buttonPin);
  button_debounced.interval(5); // interval in ms

  if(DEBUG) {
    // set up the serial connection to log out to the console.  
  	Serial.begin(9600);
  }
  
	movement_ticker.begin(mvmntTick, (1000000 / MVMNT_TICK_HZ));
	for (int i = 0; i < NUM_ANIMS; i++)
	{
		animtimer[i]= anim_reset[i];
	}
}

uint8_t bright = 255;

void get_controller_input() {
  // Update the Bounce instance :
   button_debounced.update();
   
   // Call code if Bounce fell (transition from HIGH to LOW) :
   if ( button_debounced.rose() ) {
     log(DEBUG_CONTROLLER_INPUT, "the button is pressed"); 
   }

  newEncoderPosition = RotaryEncoder.read();
  if (newEncoderPosition != oldEncoderPosition) {
    oldEncoderPosition = newEncoderPosition;
    Serial.println("new position: ");
    Serial.println(newEncoderPosition);
  }
}

void loop() {
  get_controller_input();
  
	if (move_gate) {
		// Convert HSV to RGB
		for (uint16_t i = 0; i < SEXTANT_LED_COUNT * 6; i++)
		{
			ledsraw[i] = leds[i];
		}
		FastLED.show();
		move_gate = false;
    
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
		/*
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

