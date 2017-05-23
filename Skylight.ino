
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// Directives
#define USE_OCTOWS2811
// External Libraries
#include "OctoWS2811.h"
#include "FastLED/FastLED.h"
#include "IntervalTimer.h"
#include "ArtMap.h"
#include "Skylight.h"
#include "Bounce2.h"

// DEBUG MODE???
const bool DEBUG = true;
const bool DEBUG_CONTROLLER_INPUT = true;


// Settings
#define NUM_BOXES_PER_SEXTANT 10

#define NUM_BOXES NUM_BOXES_PER_SEXTANT*6+1

#define MVMNT_TICK_HZ 30
#define NUM_ANIMS 5
#define TRANS_TIME 5
#define FADE_RATE (UINT8_MAX / TRANS_TIME)

#define OVERLAY_WAIT_TIME 4
#define OVERLAY_FADE_TIME 1


// Utils
uint16_t toSecs(uint16_t in){
  return in * MVMNT_TICK_HZ;
} 

#define FADE_START_TIMER toSecs(OVERLAY_FADE_TIME)
const int rotEncoderPinA = 9;
const int rotEncoderPinB = 10;
const int buttonPin = 11;     // the number of the pushbutton pin

// button objects
Bounce button_debounced = Bounce();

/*
// Rotary Encoder Data
static int rotaryEncoderPinA = 9; // Our first hardware interrupt pin is digital pin 2
static int rotaryEncoderPinB = 10; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 99; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
*/

// Settings - Animations
//		in to out spectrum
#define HUE_DIF_COEF 13
uint16_t toSecs(uint16_t in);
// Variables
uint16_t animtimer[NUM_ANIMS];
uint8_t hue = 0;
uint8_t curr_anim = 0;
uint16_t anim_reset[NUM_ANIMS] = { toSecs(30), toSecs(20), toSecs(30), toSecs(30),  toSecs(30) };
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

/*
void RotaryEncoderPinAInterrupt(){
  cli(); //stop interrupts happening before we read pin values
  Serial.println("in pin A interrupt");
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void RotaryEncoderPinBInterrupt(){
  cli(); //stop interrupts happening before we read pin values
  Serial.println("in pin B interrupt");
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}
*/

// ANIMATIONS
void in_out_rainbow() {
	for (int i = 0; i < 11; i++)
	{
		lightBox(ALL_SEXTANTS, i, CHSV(hue + (i*HUE_DIF_COEF), 255, 255));
	}
	hue++;
}


class DiagonalRainbow {
public:
  DiagonalRainbow() {}
  void go() {
    color_index++;
    if(color_index >= colors_in_rainbow) {
      color_index = 0; // reset if we've gone through the colors
    }

    for (sextant_index=0; sextant_index < 6; sextant_index++) {
      color_offset = sextant_index * colorShift;
      for (pixel_index_in_sextant=0; pixel_index_in_sextant < SEXTANT_LED_COUNT; pixel_index_in_sextant++) {

          pixel_address = pixel_index_in_sextant + (sextant_index*SEXTANT_LED_COUNT);
          color_index_with_offset = (color_index + color_offset) % colors_in_rainbow;

          leds[pixel_address] = rainbowColors[color_index_with_offset];
      }
    }
    // FastLED.show();
  }

  void setup() {
    for (int i=0; i<colors_in_rainbow; i++) {
      int hue = i * 2;
      // pre-compute the 'colors_in_rainbow' (180 as of now) rainbow colors
      rainbowColors[i] = CHSV(hue, 255, 255);
    }
  }
private:
  const uint8_t colorShift = 10; // how many pixels diagonal should we go per sextant?
  const uint16_t cycleTime = 2800;
  const int colors_in_rainbow = 256;

  uint16_t pixel_index_in_sextant;
  uint8_t sextant_index;
  uint8_t color_offset;

  uint16_t pixel_address;
  uint16_t color_index_with_offset;
  
  uint16_t color_index = 0;
  CHSV rainbowColors[256]; // use length equal to colors_in_rainbow
};



class SecondDiagonalRainbow {
public:
  SecondDiagonalRainbow() {}
  void go() {
    color_index++;
    if(color_index >= colors_in_rainbow) {
      color_index = 0; // reset if we've gone through the colors
    }

    for (sextant_index=0; sextant_index < 6; sextant_index++) {
      color_offset = sextant_index * colorShift;
      for (pixel_index_in_sextant=0; pixel_index_in_sextant < SEXTANT_LED_COUNT; pixel_index_in_sextant++) {

          pixel_address = pixel_index_in_sextant + (sextant_index*SEXTANT_LED_COUNT);
          color_index_with_offset = (color_index + color_offset) % colors_in_rainbow;

          leds[pixel_address] = rainbowColors[color_index_with_offset];
      }
    }
    // FastLED.show();
  }

  void setup() {
    for (int i=0; i<colors_in_rainbow; i++) {
      int hue = i * 2;
      // pre-compute the 'colors_in_rainbow' (180 as of now) rainbow colors
      rainbowColors[i] = CHSV(hue, 255, 255);
    }
  }
private:
  const uint8_t colorShift = 60; // how many pixels diagonal should we go per sextant?
  const uint16_t cycleTime = 2800;
  const int colors_in_rainbow = 256;

  uint16_t pixel_index_in_sextant;
  uint8_t sextant_index;
  uint8_t color_offset;

  uint16_t pixel_address;
  uint16_t color_index_with_offset;
  
  uint16_t color_index = 0;
  CHSV rainbowColors[256]; // use length equal to colors_in_rainbow
};












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
DiagonalRainbow diagonal_rainbow;
SecondDiagonalRainbow second_diagonal_rainbow;

void setup() {
	LEDS.addLeds<OCTOWS2811>(ledsraw, SEXTANT_LED_COUNT);
	mapInit();

  // set up the button.
  pinMode(buttonPin, INPUT);
  button_debounced.attach(buttonPin);
  button_debounced.interval(5); // interval in ms

/*
  // set up the rotary encoder...
  pinMode(rotaryEncoderPinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(rotaryEncoderPinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,RotaryEncoderPinAInterrupt,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,RotaryEncoderPinBInterrupt,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
*/

  if(DEBUG) {
    // set up the serial connection to log out to the console.  
  	Serial.begin(9600);
  }
  
	movement_ticker.begin(mvmntTick, (1000000 / MVMNT_TICK_HZ));
	for (int i = 0; i < NUM_ANIMS; i++) {
		animtimer[i]= anim_reset[i];
	}

  diagonal_rainbow.setup();
  second_diagonal_rainbow.setup();
	
	// Initialize All Leds
	for (int i = 0; i < SEXTANT_LED_COUNT * 6; i++) {
		leds[i] = CHSV(80, 255, 255);
	}
  
}

uint8_t bright = 255;

void get_controller_input() {
  // Update the Bounce instance :
   button_debounced.update();
   
   // Call code if Bounce fell (transition from HIGH to LOW) :
   if ( button_debounced.rose() ) {
     log(DEBUG_CONTROLLER_INPUT, "the button is pressed"); 
     injector.inject();
   }

  /*
  if(oldEncPos != encoderPos) {
    Serial.println(encoderPos);
    oldEncPos = encoderPos;
  }
  */
  
}

void loop() {
	
  // get_controller_input();
  
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
			if (curr_anim == 3)
				breathe.start();
		}

		switch (curr_anim)
		{
		case 0: 
			in_out_rainbow();
			break;
		case 1: 
			radar.go();
			break;
    case 2:
      second_diagonal_rainbow.go();
      break;
		case 3:
			breathe.go();
			break;
    case 4:
      diagonal_rainbow.go();
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

