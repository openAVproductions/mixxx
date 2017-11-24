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
	*out_vid = 0;
	*out_pid = 0;
}

struct mg_t {
	uint8_t shift_pressed; /* 0 or 1 when "mode" button is pressed */
	uint8_t update_fb;

	float scroll;
};

int script_init_func()
{
	return sizeof(struct mg_t);
}

void script_event_func(struct ctlra_dev_t* dev,
                       unsigned int num_events,
                       struct ctlra_event_t** events,
                       void *userdata)
{
	struct mg_t *mg = userdata;

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		int idx;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			pr = e->button.pressed;
			mg->update_fb = 1;
			printf("button %d, pr = %d\n", e->button.id, pr);
			switch(e->button.id) {
			/* cue / plays */
			case 36: mixxx_config_key_toggle("[Channel1]", "cue_default"); break;
			case 37: if(pr) mixxx_config_key_toggle("[Channel1]", "play"); break;
			case 38: mixxx_config_key_toggle("[Channel2]", "cue_default"); break;
			case 39: if(pr) mixxx_config_key_toggle("[Channel2]", "play"); break;
			/* hotcues */
			case 40: mixxx_config_key_toggle("[Channel1]", "hotcue_1_activate"); break;
			case 41: mixxx_config_key_toggle("[Channel1]", "hotcue_2_activate"); break;
			case 42: mixxx_config_key_toggle("[Channel2]", "hotcue_1_activate"); break;
			case 43: mixxx_config_key_toggle("[Channel2]", "hotcue_2_activate"); break;

			case 48: mixxx_config_key_toggle("[Channel1]", "hotcue_2_activate"); break;
			default:
				break;
			} break;

		case CTLRA_EVENT_SLIDER:
			switch(e->slider.id) {
			//case 0: mixxx_config_key_set("[Channel1]","pregain", e->slider.value * 2); break;
			case 1: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", e->slider.value * 2); break;
			case 2: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", e->slider.value * 2); break;
			case 3: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", e->slider.value * 2); break;
			case 4: mixxx_config_key_set("[QuickEffectRack1_[Channel1]]", "super1", e->slider.value); break;

			//case 5: mixxx_config_key_set("[Channel2]","pregain", e->slider.value * 2); break;
			case 5: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", e->slider.value * 2); break;
			case 6: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", e->slider.value * 2); break;
			case 7: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", e->slider.value * 2); break;
			case 8: mixxx_config_key_set("[QuickEffectRack1_[Channel2]]", "super1", e->slider.value); break;

			case 10: mixxx_config_key_set("[Master]", "headMix", (e->slider.value * 2) - 1); break;
			case  9: mixxx_config_key_set("[Channel1]", "volume", e->slider.value); break;
			case 12: mixxx_config_key_set("[Channel2]", "volume", e->slider.value); break;
			//case 22: mixxx_config_key_set("[Master]", "crossfader", (e->slider.value * 2) - 1); break;

			case 16:
				 printf("the encoder, %f\n", e->slider.value);
				 break;

			default:
				 printf("slider %d %f\n", e->slider.id, e->slider.value);
				 break;
			}
		default: break;
		}
	}
}

void script_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	struct mg_t *mg = userdata;

	if(!mg->update_fb) {
		//return;
	}

	mg->update_fb = 0;
	//printf("writing midi fb\n");
	
	uint8_t msg[3] = {
		0x90,
		0x0, /* MIDI note number for LED here */
		0x0, /* value to set LED to here */
	};

	/* define the brightest / dimmest of toggling LEDs */
	const uint32_t high = 0xffffffff;
	const uint32_t on = 707;
	const uint32_t off  = 0;

#if 0
	/* Demo of "advanced" usage of light_set API */
	float c1_hot1 = mixxx_config_key_get("[Channel1]", "hotcue_1_enabled");
	msg[1] = 40;
	msg[2] =  c1_hot1 > 0;
	uint32_t m = *(uint32_t*)msg;
	ctlra_dev_light_set(dev, -1, m);
#endif

	float c1_hot1 = mixxx_config_key_get("[Channel1]", "hotcue_1_enabled");
	float c1_hot2 = mixxx_config_key_get("[Channel1]", "hotcue_2_enabled");
	float c2_hot1 = mixxx_config_key_get("[Channel2]", "hotcue_1_enabled");
	float c2_hot2 = mixxx_config_key_get("[Channel2]", "hotcue_2_enabled");
	ctlra_dev_light_set(dev, 40, c1_hot1 > 0 ? on : off);
	ctlra_dev_light_set(dev, 41, c1_hot2 > 0 ? on : off);
	ctlra_dev_light_set(dev, 42, c2_hot1 > 0 ? on : off);
	ctlra_dev_light_set(dev, 43, c2_hot2 > 0 ? on : off);

	ctlra_dev_light_set(dev, 40, c1_hot1 > 0 ? on : off);
	float c1_play = mixxx_config_key_get("[Channel1]","play_indicator");
	float c2_play = mixxx_config_key_get("[Channel2]","play_indicator");
	ctlra_dev_light_set(dev, 37, c1_play > 0.5 ? on : off);
	ctlra_dev_light_set(dev, 39, c2_play > 0.5 ? on : off);

	float c1_cue = mixxx_config_key_get("[Channel1]","cue_indicator");
	float c2_cue = mixxx_config_key_get("[Channel2]","cue_indicator");
	ctlra_dev_light_set(dev, 36, c1_cue > 0.5 ? on : off);
	ctlra_dev_light_set(dev, 38, c2_cue > 0.5 ? on : off);

	//ctlra_dev_light_flush(dev, 1);
}
