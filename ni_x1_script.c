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

	/* fx rack: manualy handle "mix" value as enable, since turning
	 * on/off the actual "enable" control creates a pop. The enable
	 * stores the "software" enable switch, and mix the actaul value */
	float fx_rack_enable[2];
	float fx_rack_mix[2];
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
			case 0: {
				if(pr)
					x1->fx_rack_enable[0] = !x1->fx_rack_enable[0];
				int enable = x1->fx_rack_enable[0];
				float val = enable ? x1->fx_rack_mix[0] : 0.f;
				mixxx_config_key_set("[EffectRack1_EffectUnit1]", "mix", val);
				}
				break;
			case 1: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect1]", "enabled"); break;
			case 2: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect2]", "enabled"); break;
			case 3: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect3]", "enabled"); break;
			/* FX 2 (right) enable buttons */
			case 4: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2]", "enabled"); break;
			case 5: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect1]", "enabled"); break;
			case 6: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect2]", "enabled"); break;
			case 7: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect3]", "enabled"); break;

			case 13: mixxx_config_key_toggle("[Library]", "MoveFocus"); break;
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
			default: break;
			} break;

		case CTLRA_EVENT_ENCODER:
			switch(e->encoder.id) {
			case 1: {
				int dir = ((e->encoder.delta > 0) * 2) - 1;
				mixxx_config_key_set("[Library]", "MoveVertical", dir);
				} break;
			case 0: if(e->encoder.delta > 0)
					mixxx_config_key_set("[Channel1]", "rate_perm_down_small", 1.f);
				else
					mixxx_config_key_set("[Channel1]", "rate_perm_up_small", 1.f);
				break;
			case 2: if(e->encoder.delta > 0)
					mixxx_config_key_set("[Channel2]", "rate_perm_down_small", 1.f);
				else
					mixxx_config_key_set("[Channel2]", "rate_perm_up_small", 1.f);
				break;
			default: printf("enc %d\n", e->encoder.id); break;
			}
			break;

		case CTLRA_EVENT_SLIDER:
			switch(e->slider.id) {
			/* FX 1 - top left */
			case 0: x1->fx_rack_mix[0] = e->slider.value;
				int enable = x1->fx_rack_enable[0];
				if(enable) {
					float val = enable ? x1->fx_rack_mix[0] : 0.f;
					mixxx_config_key_set("[EffectRack1_EffectUnit1]", "mix", val);
				}
				break;
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

	/* FX 1 leds */
	float fx1_enable = (x1->fx_rack_enable[0]);
	ctlra_dev_light_set(dev, 0, fx1_enable ? high : 0x03030303);
	float fx1_e1_enable = mixxx_config_key_get("[EffectRack1_EffectUnit1_Effect1]", "enabled");
	ctlra_dev_light_set(dev, 1, fx1_e1_enable ? high : 0x03030303);
	float fx1_e2_enable = mixxx_config_key_get("[EffectRack1_EffectUnit1_Effect2]", "enabled");
	ctlra_dev_light_set(dev, 2, fx1_e2_enable ? high : 0x03030303);
	float fx1_e3_enable = mixxx_config_key_get("[EffectRack1_EffectUnit1_Effect3]", "enabled");
	ctlra_dev_light_set(dev, 3, fx1_e3_enable ? high : 0x03030303);

	/* FX 2 leds */
	float fx2_enable = mixxx_config_key_get("[EffectRack1_EffectUnit2]", "enabled");
	ctlra_dev_light_set(dev, 4, fx2_enable ? high : 0x03030303);
	float fx2_e1_enable = mixxx_config_key_get("[EffectRack1_EffectUnit2_Effect1]", "enabled");
	ctlra_dev_light_set(dev, 5, fx2_e1_enable ? high : 0x03030303);
	float fx2_e2_enable = mixxx_config_key_get("[EffectRack1_EffectUnit2_Effect2]", "enabled");
	ctlra_dev_light_set(dev, 6, fx2_e2_enable ? high : 0x03030303);
	float fx2_e3_enable = mixxx_config_key_get("[EffectRack1_EffectUnit2_Effect3]", "enabled");
	ctlra_dev_light_set(dev, 7, fx2_e3_enable ? high : 0x03030303);

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

	/* play/cue indicators */
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
