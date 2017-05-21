
#define OVERLAY_WAIT_TIME 4
#define OVERLAY_FADE_TIME 1
#define NUM_BOXES 61

#define FADE_START_TIMER toSecs(OVERLAY_FADE_TIME)


#include "FastLED.h"

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
					CHSV oldcolor = shape_start_addr[sextant][box];
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