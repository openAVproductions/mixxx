#pragma once

/* Ctlra functions */
void ctlra_dev_light_set(struct ctlra_dev_t *dev,
			uint32_t light_id,
			uint32_t light_status);
void ctlra_dev_light_flush(struct ctlra_dev_t *dev, uint32_t force);

/* Returns a config key */
void mixxx_config_key_set(const char *group, const char *name, float v);
void mixxx_config_key_toggle(const char *group, const char *name);

/* Sets the value of the ConfigKey to float */
//void config_key_set(void *key, float value);
//void *config_key_get(const char *group, const char *name);


/* Functions that the script *MUST* implement */

/* Fill in the VID / PID pair this script is capable of servicing */
typedef void (*script_get_vid_pid)(int *out_vid, int *out_pid);

/* Initialize the instance tracking memeory */
typedef int (*script_init_func)();

/* The script function that handles events. In the script implementation
 * of this function is where your "users" will be writing code */
typedef void (*script_event_func)(struct ctlra_dev_t* dev,
                                  unsigned int num_events,
                                  struct ctlra_event_t** events,
                                  void *userdata);

/* The script function that writes feedback to the device. The script
 * implementation will retrieve the status of the Mixxx engine, and write
 * LED status based on that */
typedef void (*script_feedback_func)(struct ctlra_dev_t* dev,
				     void *userdata);
