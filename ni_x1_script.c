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
	*out_pid = 0x1220;
}

struct x1_t {
	uint8_t shift_pressed; /* 0 or 1 when "mode" button is pressed */
};

int script_init_func()
{
	return sizeof(struct x1_t);
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
	struct x1_t *x1 = userdata;

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		int idx;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			pr = e->button.pressed;
			printf("btn: %d %d\n", e->button.id, pr);
			switch(e->button.id) {

			/* FX 1 (left) enable buttons */
			case 0: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1]", "enabled"); break;
			case 1: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect1]", "enabled"); break;
			case 2: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect2]", "enabled"); break;
			case 3: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect3]", "enabled"); break;
			/* FX 2 (right) enable buttons */
			case 4: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2]", "enabled"); break;
			case 5: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect1]", "enabled"); break;
			case 6: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect2]", "enabled"); break;
			case 7: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect3]", "enabled"); break;

			/* left hotcue */
			case 16: case 17: case 18: case 19: {
				int i = e->button.id - 16;
				mixxx_config_key_toggle("[Channel2]", hotcue_activate[i]);
				} break;

			/* right cue/play */
			case 22: mixxx_config_key_toggle("[Channel2]", "cue_default"); break;
			case 23: if(pr) mixxx_config_key_toggle("[Channel2]", "play"); break;

			/* left hotcue */
			case 24: case 25: case 26: case 27: {
				int i = e->button.id - 24;
				mixxx_config_key_toggle("[Channel1]", hotcue_activate[i]);
				} break;

			/* left cue/play */
			case 30: mixxx_config_key_toggle("[Channel1]", "cue_default"); break;
			case 31: if(pr) mixxx_config_key_toggle("[Channel1]", "play"); break;

			case 33: /* right encoder touch */ break;
			case 33: /* center encoder touch */ break;
			case 33: /* left encoder touch */ break;
			case 35: if(pr) mixxx_config_key_set("[Library]", "ChooseItem", 1); break;
#if 0

			case 0: if (x1->shift_pressed)
					mixxx_config_key_toggle("[Channel1]", "hotcue_1_activate");
				else if(pr) {
					mixxx_config_key_toggle("[Channel1]", "pfl");
				}
				break;
			case 1: if (x1->shift_pressed)
					mixxx_config_key_toggle("[Channel2]", "hotcue_1_activate");
				else if(pr)
					mixxx_config_key_toggle("[Channel2]", "pfl"); break;
			/* mode button */
			case 2: x1->shift_pressed = e->button.pressed; break;
			case 3: if(pr)
					if(x1->shift_pressed)
						mixxx_config_key_toggle("[Channel1]", "play");
					else
						mixxx_config_key_toggle("[QuickEffectRack1_[Channel1]_Effect1]", "enabled");
				break;
			case 4: if(pr)
					if(x1->shift_pressed)
						mixxx_config_key_toggle("[Channel2]", "play");
					else
						mixxx_config_key_toggle("[QuickEffectRack1_[Channel2]_Effect1]", "enabled");
				break;
#endif
			default: break;
			} break;

		case CTLRA_EVENT_ENCODER:
			switch(e->encoder.id) {
			case 1: {
				int dir = ((e->encoder.delta > 0) * 2) - 1;
				mixxx_config_key_set("[Library]", "MoveVertical", dir);
				} break;
			default: printf("enc %d\n", e->encoder.id); break;
#if 0
			case 26: // "Load B": move focus forward
				if(pr) mixxx_config_key_set("[Library]", "MoveFocusForward", 1);
				break;
			case 27: // "Load A": move focus backward
				if(pr) mixxx_config_key_set("[Library]", "MoveFocusBackward", 1);
				break;
			case 48: if(pr) mixxx_config_key_set("[Library]", "ChooseItem", 1); break;
#endif
			}
			break;

		case CTLRA_EVENT_SLIDER:
			switch(e->slider.id) {
			/* FX 1 - top left */
			case 0: mixxx_config_key_set("[EffectRack1_EffectUnit1]", "mix", e->slider.value); break;
			case 1: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect1]", "meta", e->slider.value); break;
			case 2: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect2]", "meta", e->slider.value); break;
			case 3: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect3]", "meta", e->slider.value); break;

			/* FX 2 - top right */
			case 4: mixxx_config_key_set("[EffectRack1_EffectUnit2]", "mix", e->slider.value); break;
			case 5: mixxx_config_key_set("[EffectRack1_EffectUnit2_Effect1]", "meta", e->slider.value); break;
			case 6: mixxx_config_key_set("[EffectRack1_EffectUnit2_Effect2]", "meta", e->slider.value); break;
			case 7: mixxx_config_key_set("[EffectRack1_EffectUnit2_Effect3]", "meta", e->slider.value); break;
			case 8: /* touchstrip */ break;
			default:
				printf("unused slider %d\n", e->slider.id);
				break;
			}
		default: break;
		}
	}
}

void script_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	struct x1_t *x1 = userdata;

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

	if(x1->shift_pressed) {
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

	/* Left channel hotcues */
	float c1_hot1 = mixxx_config_key_get("[Channel1]", "hotcue_1_enabled");
	float c1_hot2 = mixxx_config_key_get("[Channel1]", "hotcue_2_enabled");
	float c1_hot3 = mixxx_config_key_get("[Channel1]", "hotcue_3_enabled");
	float c1_hot4 = mixxx_config_key_get("[Channel1]", "hotcue_4_enabled");
	ctlra_dev_light_set(dev, 19, c1_hot1 ? yellow : low );
	ctlra_dev_light_set(dev, 20, c1_hot2 ? purple : low );
	ctlra_dev_light_set(dev, 23, c1_hot3 ? lblue  : low );
	ctlra_dev_light_set(dev, 24, c1_hot4 ? green  : low );

	/* Left channel hotcues */
	float c2_hot1 = mixxx_config_key_get("[Channel2]", "hotcue_1_enabled");
	float c2_hot2 = mixxx_config_key_get("[Channel2]", "hotcue_2_enabled");
	float c2_hot3 = mixxx_config_key_get("[Channel2]", "hotcue_3_enabled");
	float c2_hot4 = mixxx_config_key_get("[Channel2]", "hotcue_4_enabled");
	ctlra_dev_light_set(dev, 21, c2_hot1 ? yellow : low );
	ctlra_dev_light_set(dev, 22, c2_hot2 ? purple : low );
	ctlra_dev_light_set(dev, 25, c2_hot3 ? lblue  : low );
	ctlra_dev_light_set(dev, 26, c2_hot4 ? green  : low );

	float c1_play = mixxx_config_key_get("[Channel1]","play_indicator");
	float c2_play = mixxx_config_key_get("[Channel2]","play_indicator");
	ctlra_dev_light_set(dev, 32, c1_play > 0.5 ? high : low);
	ctlra_dev_light_set(dev, 34, c2_play > 0.5 ? high : low);

	float c1_cue = mixxx_config_key_get("[Channel1]","cue_indicator");
	float c2_cue = mixxx_config_key_get("[Channel2]","cue_indicator");
	ctlra_dev_light_set(dev, 31, c1_cue > 0.5 ? high : low);
	ctlra_dev_light_set(dev, 33, c2_cue > 0.5 ? high : low);

	ctlra_dev_light_flush(dev, 1);
}
