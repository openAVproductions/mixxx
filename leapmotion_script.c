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

struct leapmotion_t {
};

int script_init_func()
{
	return sizeof(struct leapmotion_t);
}

void script_event_func(struct ctlra_dev_t* dev,
                       unsigned int num_events,
                       struct ctlra_event_t** events,
                       void *userdata)
{
	struct leapmotion_t *leapmotion = userdata;

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		int idx;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			break;

		case CTLRA_EVENT_SLIDER:
			switch(e->slider.id) {
			case 0: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect1]", "meta", e->slider.value); break;
			case 1: mixxx_config_key_set("[EffectRack1_EffectUnit1_Effect2]", "meta", e->slider.value); break;
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
	struct leapmotion_t *leapmotion = userdata;
}
