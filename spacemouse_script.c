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

struct space_t {
	uint8_t shift_pressed; /* 0 or 1 when "mode" button is pressed */

	/* fx rack: manualy handle "mix" value as enable, since turning
	 * on/off the actual "enable" control creates a pop. The enable
	 * stores the "software" enable switch, and mix the actaul value */
	float fx_rack_enable[2];
	float fx_rack_mix[2];
};

int script_init_func()
{
	return sizeof(struct space_t);
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
	struct space_t *space = userdata;

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		int idx;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			pr = e->button.pressed;
			//printf("btn: %d %d\n", e->button.id, pr);
			switch(e->button.id) {
			/* FX 1 (left) enable buttons */
			case 1: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect3]", "enabled"); break;
			case 2: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect2]", "enabled"); break;
			case 4: if(pr) mixxx_config_key_toggle("[EffectRack1_EffectUnit1_Effect1]", "enabled"); break;
			case 5: mixxx_config_key_set("[EffectRack1_EffectUnit1]", "mix", pr ? 1.0f : 0.f);
				break;
			default: break;
			} break;

		case CTLRA_EVENT_SLIDER:
			switch(e->slider.id) {
			/* FX 1 - top left */
			/*
			case 0: space->fx_rack_mix[0] = e->slider.value;
				int enable = space->fx_rack_enable[0];
				if(enable) {
					float val = enable ? space->fx_rack_mix[0] : 0.f;
					mixxx_config_key_set("[EffectRack1_EffectUnit1]", "mix", val);
				}
				break;
			*/
			case 1: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect2]", "meta", e->slider.value); break;
			case 2: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect1]", "meta", e->slider.value); break;
			case 3: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect3]", "meta", e->slider.value); break;

			/* FX 2 - top right */
			case 4: mixxx_config_key_set("[EffectRack1_EffectUnit2]", "mix", e->slider.value); break;
			case 5: mixxx_config_key_set("[EffectRack1_EffectUnit2_Effect1]", "meta", e->slider.value); break;
			case 6: mixxx_config_key_set("[EffectRack1_EffectUnit2_Effect2]", "meta", e->slider.value); break;
			default:
				//printf("unused slider %d\n", e->slider.id);
				break;
			}
		default: break;
		}
	}
}

void script_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	struct space_t *space = userdata;
}
