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
	*out_pid = 0x1320;
}

/* defines for shift/gain etc arrays */
#define DECK_A 0
#define DECK_B 1

struct s2_t {
	uint8_t shift_pressed[2]; /* 0 or 1 when "mode" button is pressed */
	uint8_t gain_enc_enabled[2]; /* 0 for filter, 1 for gain mode */
	uint8_t jog_wheel_pressed[2]; /* jog wheel top button press */
};

int script_init_func()
{
	return sizeof(struct s2_t);
}

/* reverse order because the button IDs are too :) */
static const char *hotcue_activate[] = {
	"hotcue_4_activate",
	"hotcue_3_activate",
	"hotcue_2_activate",
	"hotcue_1_activate",
};
static const char *hotcue_gotoplay[] = {
	"hotcue_4_gotoandplay",
	"hotcue_3_gotoandplay",
	"hotcue_2_gotoandplay",
	"hotcue_1_gotoandplay",
};

void script_event_func(struct ctlra_dev_t* dev,
                       unsigned int num_events,
                       struct ctlra_event_t** events,
                       void *userdata)
{
	struct s2_t *s2 = userdata;

	//mixxx_config_key_set("[Channel1]","quantize", 0);
	//mixxx_config_key_set("[Channel2]","quantize", 0);

	//mixxx_config_key_set("[Channel1]","quantize_beat", 0);
	//mixxx_config_key_set("[Channel2]","quantize_beat", 0);

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		int idx;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			pr = e->button.pressed;
			printf("%d\n", e->button.id);
			switch(e->button.id) {
			case 0: if(pr) mixxx_config_key_toggle("[Channel2]", "play"); break;
			case 1: mixxx_config_key_toggle("[Channel2]", "cue_default"); break;
			case 2: mixxx_config_key_toggle("[Channel2]", "sync_enabled"); break;
			case 3: s2->shift_pressed[DECK_B] = e->button.pressed; break;
			case 4: case 5: case 6: case 7:
				mixxx_config_key_toggle("[Channel2]", hotcue_activate[e->button.id-4]);
				break;
			case 8: mixxx_config_key_set("[Channel1]", "scratch2_enable", e->button.pressed);
				s2->jog_wheel_pressed[DECK_A] = e->button.pressed;
				break;
			case 9: mixxx_config_key_set("[Channel2]", "scratch2_enable", e->button.pressed);
				s2->jog_wheel_pressed[DECK_B] = e->button.pressed;
				break;
			
			case 28: if(pr) mixxx_config_key_toggle("[Channel1]", "pfl"); break;
			case 12: if(pr) mixxx_config_key_toggle("[Channel2]", "pfl"); break;

			/* loop toggles */
			case 14: 
				if(!s2->shift_pressed[DECK_B])
					if(pr) mixxx_config_key_toggle("[Channel2]", "loop_in"); break;
			case 15: if(pr) mixxx_config_key_toggle("[Channel2]", "loop_out"); break;

			case 16: if(pr) mixxx_config_key_toggle("[Channel1]", "play"); break;
			case 17: mixxx_config_key_toggle("[Channel1]", "cue_default"); break;
			case 18: mixxx_config_key_toggle("[Channel1]", "sync_enabled"); break;
			case 20: case 21: case 22: case 23:
				mixxx_config_key_toggle("[Channel1]", hotcue_activate[e->button.id-20]);
				break;

			case 26: // "Load B": move focus forward
				if(pr) mixxx_config_key_set("[Library]", "MoveFocusForward", 1);
				break;
			case 27: // "Load A": move focus backward
				if(pr) mixxx_config_key_set("[Library]", "MoveFocusBackward", 1);
				break;

			case 41: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1]",
							       "group_[Channel1]_enable"); break;
			case 40: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2]",
							       "group_[Channel1]_enable"); break;
			case 39: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1]",
							       "group_[Channel2]_enable"); break;
			case 38: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2]",
							       "group_[Channel2]_enable"); break;


			case 48: if(pr) mixxx_config_key_set("[Library]", "ChooseItem", 1); break;
			case 49: if(pr) mixxx_config_key_set("[Channel2]", "reloop_toggle", 1); break;
			//case 50: if(pr) mixxx_config_key_set("[Channel2]", "loop_move",  4); break;
				break;

			/* F1 on buttons */
			case 43: if(pr) mixxx_config_key_toggle("[QuickEffectRack1_[Channel1]_Effect1]", "enabled"); break;
			case 45: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect1]", "enabled"); break;
			case 44: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect2]", "enabled"); break;

			/* F2 on buttons */
			case 33: if(pr) mixxx_config_key_toggle("[QuickEffectRack1_[Channel2]_Effect1]", "enabled"); break;
			case 35: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect1]", "enabled"); break;
			case 34: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit2_Effect2]", "enabled"); break;
			default: printf("button %d\n", e->button.id); break;
			}
			break;
		case CTLRA_EVENT_ENCODER:
			switch(e->encoder.id) 
			{
			case 0: /* jog wheel left */ {
				float v = mixxx_config_key_get("[Channel1]", "scratch2");
				//mixxx_config_key_set("[Channel1]", "scratch2", v * 0.95 + (e->encoder.delta_float*50.f)-0.f);
				mixxx_config_key_set("[Channel1]", "scratch2", v * 0.98 + (e->encoder.delta_float*40.f)-0.f);
				} break;
			case 1: /* jog wheel right */
				mixxx_config_key_set("[Channel2]", "wheel", 0.f);
				break;
			case 4: {
				int dir = ((e->encoder.delta > 0) * 2) - 1;
				mixxx_config_key_set("[Library]", "MoveVertical", dir);
				} break;

			case 5: {
				if(e->encoder.delta > 0)
					mixxx_config_key_set("[Channel2]", "beatjump_forward", 1);
				else
					mixxx_config_key_set("[Channel2]", "beatjump_backward", 1);
				} break;
			case 6: {
				int now = mixxx_config_key_get("[Channel2]", "beatjump_size");
				int v = e->encoder.delta > 0 ? now * 2 : now / 2;
				if(v <  4) v =  4;
				if(v > 32) v = 32;
				mixxx_config_key_set("[Channel2]", "beatjump_size", v);
				printf("%d\n", v);
				} break;
			default:
				printf("encoder id %d\n", e->encoder.id);
				break;
			}
			break;
		case CTLRA_EVENT_SLIDER:
			printf("%d\n", e->slider.id);
			switch(e->slider.id) {
			case 0: mixxx_config_key_set("[Master]"  , "crossfader", (e->slider.value * 2) - 1); break;
			case 1: mixxx_config_key_set("[Channel1]", "rate", (e->slider.value*2.f)-1.f); break;
			case 2: mixxx_config_key_set("[Channel2]", "rate", (e->slider.value*2.f)-1.f); break;
			case 3: mixxx_config_key_set("[Master]"  , "headMix", (e->slider.value * 2) - 1); break;

			case 7: mixxx_config_key_set("[Channel1]", "volume", e->slider.value); break;
			case 8: mixxx_config_key_set("[Channel2]", "volume", e->slider.value); break;

			/* F1 FX - top left */
			case  9: mixxx_config_key_set("[EffectRack1_EffectUnit1]", "mix", e->slider.value); break;
			case 10: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect1]", "meta", e->slider.value); break;
			case 11: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect2]", "meta", e->slider.value); break;
			case 12: mixxx_config_key_set("[QuickEffectRack1_[Channel1]]", "super1", e->slider.value); break;

			case 13: mixxx_config_key_set("[EffectRack1_EffectUnit2]", "mix", e->slider.value); break;
			case 14: mixxx_config_key_set("[EffectRack1_EffectUnit2_Effect1]", "meta", e->slider.value); break;
			case 15: mixxx_config_key_set("[EffectRack1_EffectUnit2_Effect2]", "meta", e->slider.value); break;
			case 16: mixxx_config_key_set("[QuickEffectRack1_[Channel2]]", "super1", e->slider.value); break;

			case 17: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", e->slider.value * 2); break;
			case 18: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", e->slider.value * 2); break;
			case 19: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", e->slider.value * 2); break;

			//case 15: mixxx_config_key_set("[QuickEffectRack1_[Channel2]]", "super1", e->slider.value); break;
			case 20: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", e->slider.value * 2); break;
			case 21: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", e->slider.value * 2); break;
			case 22: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", e->slider.value * 2); break;

			//case 0: mixxx_config_key_set("[Channel1]","pregain", e->slider.value * 2); break;
			//case 1: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", e->slider.value * 2); break;
			//case 2: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", e->slider.value * 2); break;
			//case 3: mixxx_config_key_set("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", e->slider.value * 2); break;

			//case 5: mixxx_config_key_set("[Channel2]","pregain", e->slider.value * 2); break;
			//case 6: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", e->slider.value * 2); break;
			//case 7: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", e->slider.value * 2); break;
			//case 8: mixxx_config_key_set("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", e->slider.value * 2); break;
			//case 9: mixxx_config_key_set("[QuickEffectRack1_[Channel2]]", "super1", e->slider.value); break;

			default: printf("slider %d\n", e->slider.id); break;
			}

			break;
		default: break;
		}
	}
}

static uint32_t c;

void script_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	struct s2_t *s2 = userdata;

	/* define the brightest / dimmest of toggling LEDs */
	const uint32_t high = 0xffffffff;
	const uint32_t low  = 0x07070707;
	const uint32_t off  = 0x0;

	const uint32_t blue     = 0x000000ef;
	const uint32_t lblue    = 0x00004cef;
	const uint32_t blue_low = 0x0000000f;
	const uint32_t yellow   = 0x00efef00;
	const uint32_t green    = 0x0000ef00;
	const uint32_t red      = 0x00ef0000;
	const uint32_t purple   = 0x00ef00ef;

	for(int i = 0; i < 56; i++)
		ctlra_dev_light_set(dev, i, low );

	float c2_hot1 = mixxx_config_key_get("[Channel2]", "hotcue_1_enabled");
	float c2_hot2 = mixxx_config_key_get("[Channel2]", "hotcue_2_enabled");
	float c2_hot3 = mixxx_config_key_get("[Channel2]", "hotcue_3_enabled");
	float c2_hot4 = mixxx_config_key_get("[Channel2]", "hotcue_4_enabled");
	ctlra_dev_light_set(dev, 40, c2_hot1 ? yellow : low );
	ctlra_dev_light_set(dev, 41, c2_hot2 ? purple : low );
	ctlra_dev_light_set(dev, 42, c2_hot3 ? lblue  : low );
	ctlra_dev_light_set(dev, 43, c2_hot4 ? green  : low );

	float c1_hot1 = mixxx_config_key_get("[Channel1]", "hotcue_1_enabled");
	float c1_hot2 = mixxx_config_key_get("[Channel1]", "hotcue_2_enabled");
	float c1_hot3 = mixxx_config_key_get("[Channel1]", "hotcue_3_enabled");
	float c1_hot4 = mixxx_config_key_get("[Channel1]", "hotcue_4_enabled");
	ctlra_dev_light_set(dev, 36, c1_hot1 ? yellow : low );
	ctlra_dev_light_set(dev, 37, c1_hot2 ? purple : low );
	ctlra_dev_light_set(dev, 38, c1_hot3 ? lblue  : low );
	ctlra_dev_light_set(dev, 39, c1_hot4 ? green  : low );

	int c1_pfl = mixxx_config_key_get("[Channel1]", "pfl");
	ctlra_dev_light_set(dev, 26, c1_pfl ? high : low );
	int c2_pfl = mixxx_config_key_get("[Channel2]", "pfl");
	ctlra_dev_light_set(dev, 30, c2_pfl ? high : low );

	float c1_play = mixxx_config_key_get("[Channel1]","play_indicator");
	float c2_play = mixxx_config_key_get("[Channel2]","play_indicator");
	ctlra_dev_light_set(dev, 47, c1_play > 0.5 ? high : low);
	ctlra_dev_light_set(dev, 51, c2_play > 0.5 ? high : low);

#if 0
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
#endif

	/* FX lights */
	float fx1_e1_enable = mixxx_config_key_get("[EffectRack1_EffectUnit1_Effect1]", "enabled");
	ctlra_dev_light_set(dev, 11, fx1_e1_enable ? high : 0x03030303);
	float fx1_e2_enable = mixxx_config_key_get("[EffectRack1_EffectUnit1_Effect2]", "enabled");
	ctlra_dev_light_set(dev, 12, fx1_e2_enable ? high : 0x03030303);
	float chan1_filter_enabled = mixxx_config_key_get("[QuickEffectRack1_[Channel1]_Effect1]", "enabled");
	ctlra_dev_light_set(dev, 13, chan1_filter_enabled? high : 0x03030303);
	/* FX 2 lights */
	float fx2_e1_enable = mixxx_config_key_get("[EffectRack1_EffectUnit2_Effect1]", "enabled");
	ctlra_dev_light_set(dev, 19, fx2_e1_enable ? high : 0x03030303);
	float fx2_e2_enable = mixxx_config_key_get("[EffectRack1_EffectUnit2_Effect2]", "enabled");
	ctlra_dev_light_set(dev, 20, fx2_e2_enable ? high : 0x03030303);
	float chan2_filter_enabled = mixxx_config_key_get("[QuickEffectRack1_[Channel2]_Effect1]", "enabled");
	ctlra_dev_light_set(dev, 21, chan2_filter_enabled? high : 0x03030303);

	/* Channel FX enable lights */
	float c1_fx1 = mixxx_config_key_get("[EffectRack1_EffectUnit1]",
					    "group_[Channel1]_enable");
	float c1_fx2 = mixxx_config_key_get("[EffectRack1_EffectUnit2]",
					    "group_[Channel1]_enable");
	ctlra_dev_light_set(dev, 14, c1_fx1 > 0.5 ? high : low);
	ctlra_dev_light_set(dev, 15, c1_fx2 > 0.5 ? high : low);
	float c2_fx1 = mixxx_config_key_get("[EffectRack1_EffectUnit1]",
					    "group_[Channel2]_enable");
	float c2_fx2 = mixxx_config_key_get("[EffectRack1_EffectUnit2]",
					    "group_[Channel2]_enable");
	ctlra_dev_light_set(dev, 16, c2_fx1 > 0.5 ? high : low);
	ctlra_dev_light_set(dev, 17, c2_fx2 > 0.5 ? high : low);


	/* vu meters */
	float vu_1 = mixxx_config_key_get("[Channel1]","VuMeter");
	float vu_2 = mixxx_config_key_get("[Channel2]","VuMeter");
	for(int i = 0; i < 5; i++) {
		float val = (i / 5.f) + 0.01;
		float v1 = vu_1 > 0.001 ? vu_1 : 0;
		float v2 = vu_2 > 0.001 ? vu_2 : 0;
		ctlra_dev_light_set(dev, i    , v1 < val ? 0x03030303 : high);
		ctlra_dev_light_set(dev, i + 5, v2 < val ? 0x03030303 : high);
	}

	static int count;
	if((count++ % 2) == 0)
		ctlra_dev_light_flush(dev, 1);
}
