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
	*out_vid = 0x17cc;
	*out_pid = 0x1210;
}

struct z1_t {
	uint8_t shift_pressed; /* 0 or 1 when "mode" button is pressed */
};

int script_init_func()
{
	return sizeof(struct z1_t);
}

void script_event_func(struct ctlra_dev_t* dev,
                       unsigned int num_events,
                       struct ctlra_event_t** events,
                       void *userdata)
{
	struct z1_t *z1 = userdata;

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		int idx;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			pr = e->button.pressed;
			switch(e->button.id) {
			case 0: if (z1->shift_pressed)
					mixxx_config_key_toggle("[Channel1]", "hotcue_1_activate");
				else if(pr) {
					mixxx_config_key_toggle("[Channel1]", "pfl");
				}
				break;
			case 1: if (z1->shift_pressed)
					mixxx_config_key_toggle("[Channel2]", "hotcue_1_activate");
				else if(pr)
					mixxx_config_key_toggle("[Channel2]", "pfl"); break;
			/* mode button */
			case 2: z1->shift_pressed = e->button.pressed; break;
			case 3: if(pr)
					if(z1->shift_pressed)
						mixxx_config_key_toggle("[Channel1]", "play");
					else
						mixxx_config_key_toggle("[QuickEffectRack1_[Channel1]_Effect1]", "enabled");
				break;
			case 4: if(pr)
					if(z1->shift_pressed)
						mixxx_config_key_toggle("[Channel2]", "play");
					else
						mixxx_config_key_toggle("[QuickEffectRack1_[Channel2]_Effect1]", "enabled");
				break;
			default: break;
			} break;

		case CTLRA_EVENT_SLIDER:
			switch(e->slider.id) {
			case 0: mixxx_config_key_set("[Channel1]","pregain", e->slider.value * 2); break;
			case 1: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", e->slider.value * 2); break;
			case 2: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", e->slider.value * 2); break;
			case 3: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", e->slider.value * 2); break;
			case 4: mixxx_config_key_set("[QuickEffectRack1_[Channel1]]", "super1", e->slider.value); break;

			case 5: mixxx_config_key_set("[Channel2]","pregain", e->slider.value * 2); break;
			case 6: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", e->slider.value * 2); break;
			case 7: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", e->slider.value * 2); break;
			case 8: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", e->slider.value * 2); break;
			case 9: mixxx_config_key_set("[QuickEffectRack1_[Channel2]]", "super1", e->slider.value); break;

			case 10: mixxx_config_key_set("[Master]", "headMix", (e->slider.value * 2) - 1); break;
			case 11: mixxx_config_key_set("[Channel1]", "volume", e->slider.value); break;
			case 12: mixxx_config_key_set("[Channel2]", "volume", e->slider.value); break;
			case 13: mixxx_config_key_set("[Master]", "crossfader", (e->slider.value * 2) - 1); break;
			default: break;
			}
		default: break;
		}
	}
}

void script_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	struct z1_t *z1 = userdata;

	/* define the brightest / dimmest of toggling LEDs */
	const uint32_t high = 0xffffffff;
	const uint32_t low  = 0x07070707;
	const uint32_t off  = 0x0;

	const uint32_t blue     = 0x000000ef;
	const uint32_t blue_low = 0x0000000f;
	const uint32_t orange   = 0x00efef00;

	if(z1->shift_pressed) {
		/* A:B buttons, hotcue 1 available when lit */
		float c1_hot1 = mixxx_config_key_get("[Channel1]", "hotcue_1_enabled");
		float c2_hot1 = mixxx_config_key_get("[Channel2]", "hotcue_1_enabled");
		ctlra_dev_light_set(dev, 14, c1_hot1 > 0.5 ? high : off);
		ctlra_dev_light_set(dev, 15, c2_hot1 > 0.5 ? high : off);
		/* playing status to filter in orange */
		float c1_play = mixxx_config_key_get("[Channel1]","play_indicator");
		float c2_play = mixxx_config_key_get("[Channel2]","play_indicator");
		ctlra_dev_light_set(dev, 16, c1_play > 0.5 ? orange : off);
		ctlra_dev_light_set(dev, 19, c2_play > 0.5 ? orange : off);
		/* mode off */
		ctlra_dev_light_set(dev, 18, 0);
	}
	else {
		float pfl_1 = mixxx_config_key_get("[Channel1]","pfl");
		float pfl_2 = mixxx_config_key_get("[Channel2]","pfl");
		float filter_1 = mixxx_config_key_get(
				 "[QuickEffectRack1_[Channel1]_Effect1]", "enabled");
		float filter_2 = mixxx_config_key_get(
				 "[QuickEffectRack1_[Channel2]_Effect1]", "enabled");

		int c1pfl_value = pfl_1 > 0.5;
		int c2pfl_value = pfl_2 > 0.5;
		int c1fon_val = filter_1 > 0.5;
		int c2fon_val = filter_2 > 0.5;

		ctlra_dev_light_set(dev, 14, c1pfl_value ? high : low);
		ctlra_dev_light_set(dev, 15, c2pfl_value ? high : low);

		ctlra_dev_light_set(dev, 16, c1fon_val ? blue : blue_low);
		ctlra_dev_light_set(dev, 19, c2fon_val ? blue : blue_low);

		ctlra_dev_light_set(dev, 18, low);
	}

	/* vu meters */
	float vu_1 = mixxx_config_key_get("[Channel1]","VuMeter");
	float vu_2 = mixxx_config_key_get("[Channel2]","VuMeter");
	for(int i = 0; i < 7; i++) {
		float val = (i / 7.f) + 0.01;
		float v1 = vu_1 > 0.001 ? vu_1 : 0;
		float v2 = vu_2 > 0.001 ? vu_2 : 0;
		ctlra_dev_light_set(dev, i    , v1 < val ? 0x03030303 : high);
		ctlra_dev_light_set(dev, i + 7, v2 < val ? 0x03030303 : high);
	}

	ctlra_dev_light_flush(dev, 1);
}
