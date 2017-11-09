/* hack for TCC to not need stdint.h, which causes a segfault on compile
 * the second time the script is compiled */
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned int uint16_t;
typedef int int32_t;

void mixxx_config_key_set(const char *group, const char *name, float v);
void mixxx_config_key_toggle(const char *group, const char *name);
float mixxx_config_key_get(const char *group, const char *name);

#include "event.h"

void script_get_vid_pid(int *out_vid, int *out_pid)
{
	*out_vid = 0x27cc;
	*out_pid = 0x2220;
}

struct x2_t {
	uint8_t init;
	uint8_t shift_pressed; /* 0 or 1 when "mode" button is pressed */

	/* fx rack: manualy handle "mix" value as enable, since turning
	 * on/off the actual "enable" control creates a pop. The enable
	 * stores the "software" enable switch, and mix the actaul value */
	float fx_rack_enable[2];
};

int script_init_func()
{
	return sizeof(struct x2_t);
}

static const char *hotcue_activate[] = {
	"hotcue_1_activate",
	"hotcue_2_activate",
	"hotcue_3_activate",
	"hotcue_4_activate",
};

void script_event_func(struct ctlra_dev_t* dev,
                       unsigned int num_events,
                       struct ctlra_event_t** events,
                       void *userdata)
{
	struct x2_t *x2 = userdata;

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		int idx;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			pr = e->button.pressed;
			printf("btn: %d %d\n", e->button.id, pr);
			switch(e->button.id) {
			/* left cue/play */
			case 4: if(pr) mixxx_config_key_toggle("[Channel1]", "play"); break;
			//case 30: mixxx_config_key_toggle("[Channel1]", "cue_default"); break;
			/* left hotcue */
			case 24: case 25: case 26: case 27: {
				int i = e->button.id - 24;
				mixxx_config_key_toggle("[Channel1]", hotcue_activate[i]);
				} break;
			default: break;
			} break;
		case CTLRA_EVENT_GRID:
			printf("grid: %d %d %d\n", e->grid.id, e->grid.pos,
			       e->grid.pressed);
		default: break;
		}
	}
}

void script_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	struct x2_t *x2 = userdata;

	if(!x2->init) {
		for(int i = 0; i < 255; i++)
			ctlra_dev_light_set(dev, i, 0x0);
		x2->init = 1;
	}

	/* define the brightest / dimmest of toggling LEDs */
	const uint32_t high = 0xffffffff;
	const uint32_t low  = 0x07070707;
	const uint32_t off  = 0x0;

	/* define the brightest / dimmest of toggling LEDs */
	const uint32_t blue     = 0x000000ef;
	const uint32_t lblue    = 0x00004cef;
	const uint32_t blue_low = 0x0000000f;
	const uint32_t yellow   = 0x00efef00;
	const uint32_t green    = 0x0000ef00;
	const uint32_t red      = 0x00ef0000;
	const uint32_t purple   = 0x00ef00ef;

	/* Left channel hotcues */
	float c1_hot1 = mixxx_config_key_get("[Channel1]", "hotcue_1_enabled");
	float c1_hot2 = mixxx_config_key_get("[Channel1]", "hotcue_2_enabled");
	float c1_hot3 = mixxx_config_key_get("[Channel1]", "hotcue_3_enabled");
	float c1_hot4 = mixxx_config_key_get("[Channel1]", "hotcue_4_enabled");
	ctlra_dev_light_set(dev, 12, c1_hot1 ? high : low );
	ctlra_dev_light_set(dev, 13, c1_hot2 ? high : low );
	ctlra_dev_light_set(dev, 14, c1_hot3 ? high : low );
	ctlra_dev_light_set(dev, 15, c1_hot4 ? high : low );

	/* play/cue indicators */
	float c1_play = mixxx_config_key_get("[Channel1]","play_indicator");
	float c1_cue = mixxx_config_key_get("[Channel1]","cue_indicator");
	ctlra_dev_light_set(dev, 16, c1_play > 0.5 ? high : low);
	ctlra_dev_light_set(dev, 17, c1_cue  > 0.5 ? high : low);

	/*
	float c1_cue = mixxx_config_key_get("[Channel1]","cue_indicator");
	float c2_cue = mixxx_config_key_get("[Channel2]","cue_indicator");
	ctlra_dev_light_set(dev, 31, c1_cue > 0.5 ? high : low);
	ctlra_dev_light_set(dev, 33, c2_cue > 0.5 ? high : low);
	*/

	ctlra_dev_light_flush(dev, 1);

	uint8_t *pixel_data;
	uint32_t bytes;
	int has_screen = ctlra_dev_screen_get_data(dev,
						   &pixel_data,
						   &bytes, 0);


	float c1_vu = mixxx_config_key_get("[Channel1]","VuMeter");
	float c2_vu = mixxx_config_key_get("[Channel2]","VuMeter");

	/* TODO: fix the bytes parameter, once a decision on the
	 * general approach is made; see here for details:
	 * https://github.com/openAVproductions/openAV-Avtka/pull/5
	 */
	/* 128x64 pixels, RGB = 3 channels, Cairo Stride +1 */
#define SIZE (128 * 64)
	uint32_t *p = (uint32_t *)pixel_data;
	for(int i = 0; i < SIZE; i++) {
		p[i] = 0x0; //0ff0000;//rand();
	}

	for (int i = 0; i < 64; i++) {
		uint32_t v = (c1_vu * 64) > i ? 0xffffffff : 0;
		p[(63-i)*128]   = v;
		p[(63-i)*128+1] = v;
		p[(63-i)*128+2] = v;

		v = (c2_vu * 64) > i ? 0xffffffff : 0;
		p[(63-i)*128+125] = v;
		p[(63-i)*128+126] = v;
		p[(63-i)*128+127] = v;
	}

}
