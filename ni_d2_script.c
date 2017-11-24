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
#include "ni_kontrol_d2.h"

/* Tell the host application what USB device this script is for */
void script_get_vid_pid(int *out_vid, int *out_pid)
{
	*out_vid = 0x17cc;
	*out_pid = 0x1400;
}

static const char *chans[] = {
	"[Channel1]",
	"[Channel2]",
	"[Channel3]",
	"[Channel4]",
};

static const char *chans_nolin[] = {
	"[Channel3]",
	"[Channel1]",
	"[Channel2]",
	"[Channel4]",
};

static const char *chans_quickfx[] = {
	"[QuickEffectRack1_[Channel3]]",
	"[QuickEffectRack1_[Channel1]]",
	"[QuickEffectRack1_[Channel2]]",
	"[QuickEffectRack1_[Channel4]]",
};

static const char *chans_quickfx_effect1[] = {
	"[QuickEffectRack1_[Channel3]_Effect1]",
	"[QuickEffectRack1_[Channel1]_Effect1]",
	"[QuickEffectRack1_[Channel2]_Effect1]",
	"[QuickEffectRack1_[Channel4]_Effect1]",
};

static const char *hotcue_activate[] = {
	"hotcue_1_activate",
	"hotcue_2_activate",
	"hotcue_3_activate",
	"hotcue_4_activate",
	"hotcue_5_activate",
	"hotcue_6_activate",
	"hotcue_7_activate",
	"hotcue_8_activate"
};

static const char *hotcue_clear[] = {
	"hotcue_1_clear",
	"hotcue_2_clear",
	"hotcue_3_clear",
	"hotcue_4_clear",
	"hotcue_5_clear",
	"hotcue_6_clear",
	"hotcue_7_clear",
	"hotcue_8_clear"
};

static const char *beatloop_toggle_dur[] = {
	"beatloop_0.03125_toggle",
	"beatloop_0.0625_toggle",
	"beatloop_0.125_toggle",
	"beatloop_0.25_toggle",
	"beatloop_0.5_toggle",
	"beatloop_1_toggle",
	"beatloop_2_toggle",
	"beatloop_4_toggle",
	"beatloop_8_toggle",
	"beatloop_16_toggle",
	"beatloop_32_toggle",
	"beatloop_64_toggle"
};

struct d2_t {
	uint8_t deck_id;		/* 0-3 of the deck: abcd */
	uint8_t shift_pressed;		/* binary if shift is pressed */
	uint8_t perf_mode;		/* pad mode, hotcue, loop, freeze, remix */

	uint32_t samples[4];		/* Total duration of track on deck */

	float touch_start_pos;
};

int script_init_func()
{
	return sizeof(struct d2_t);
}

void script_event_func(struct ctlra_dev_t* dev,
                       unsigned int num_events,
                       struct ctlra_event_t** events,
                       void *userdata)
{
	struct d2_t *d2 = userdata;
	mixxx_config_key_set(chans[0], "quantize", 0);
	mixxx_config_key_set(chans[1], "quantize", 0);
	mixxx_config_key_set(chans[2], "quantize", 0);
	mixxx_config_key_set(chans[3], "quantize", 0);

	//printf("script func, num events %d\n", num_events);
	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		int pr = 0;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			pr = e->button.pressed;
			printf("button %d\n", e->button.id);
			switch(e->button.id) {
			case 0: d2->deck_id = 0; break;
			case 1: d2->deck_id = 1; break;
			case 2: d2->deck_id = 2; break;
			case 3: d2->deck_id = 3; break;

			case 4:
			case 5:
			case 6:
			case 7:
				/* FX enable / disable */
				if(pr) mixxx_config_key_toggle(chans_quickfx_effect1[e->button.id - 4], "enabled");
				break;
			case 13:
				break;
			case 25:
				mixxx_config_key_set(chans[d2->deck_id], "LoadSelectedTrack", e->slider.value);
				break;

			case 26:
				mixxx_config_key_set("[Master]","maximize_library", e->button.pressed);
				break;
			case 27:
				if(pr) mixxx_config_key_set("[Library]","MoveFocusForward", e->button.pressed);
				break;
			case 30:
				//mixxx_config_key_set(chans[d2->deck_id], beatloop_toggle_dur[7], e->slider.value);
				mixxx_config_key_set(chans[d2->deck_id], "reloop_exit", e->slider.value);
				break;


			case 32:  /* ON buttons toggle PFL */
			case 33:
			case 34:
			case 35:
				if(pr)
					mixxx_config_key_toggle(chans_nolin[e->button.id-32],"pfl");
				break;

			/* Pads */
			case 40: case 41: case 42: case 43:
			case 44: case 45: case 46: case 47:
				switch(d2->perf_mode) {
				case 0: /* HOTCUE */
					if(pr)
						mixxx_config_key_set(chans[d2->deck_id],
								     d2->shift_pressed ?
								     hotcue_clear   [e->button.id-40] :
								     hotcue_activate[e->button.id-40],
								     1.f);
					break;
				case 1: /* LOOP */
					if(e->button.id < 44)
						mixxx_config_key_toggle(chans_nolin[e->button.id - 40], "loop_in");
					else if( pr )
						mixxx_config_key_toggle(chans_nolin[e->button.id - 44], "loop_out");
					break;
				case 3: /* REMIX */
					if(e->button.id < 44)
						mixxx_config_key_toggle(chans_nolin[e->button.id - 40], "cue_default");
					else if( pr )
						mixxx_config_key_toggle(chans_nolin[e->button.id - 44], "play");
					break;
				default: break;
				}
				break;
			/* Perf mode */
			case 48:
			case 49:
			case 50:
			case 51:
				d2->perf_mode = e->button.id - 48;
				break;

			case 52: printf("Flux pressed, %d\n", e->button.id);
				 //mixxx_config_key_set("[Channel1]","volume", 1.f);
				 break;
			case 53: printf("Deck\n"); break;
			case 54: printf("Shift\n");
				 d2->shift_pressed = e->button.pressed;
				 printf("shift pressed %d\n",
					d2->shift_pressed);
				 break;
			case 55: /* sync */
				 break;
			case 56:
				 mixxx_config_key_toggle(chans[d2->deck_id],"cue_default");
				 break;
			case 57: if(pr)
					mixxx_config_key_toggle(chans[d2->deck_id],"play");
				 break;
			case 58:
				printf("TouchStrip Touch %d\n", e->button.pressed);
				break;
			default: printf("button %d\n", e->button.id); break;
			}
			break;
		case CTLRA_EVENT_ENCODER:
			switch(e->encoder.id) {
			case 0:
			case 1:
			case 2:
			case 3: {
				static const uint8_t lin_to_chan[] = { 2, 0, 1, 3 };
				int idx = e->encoder.id;
				if(e->encoder.delta < 0)
					mixxx_config_key_set(chans[lin_to_chan[idx]],
							     "rate_perm_down_small", 1);
				else
					mixxx_config_key_set(chans[lin_to_chan[idx]],
							     "rate_perm_up_small", 1);
				}
				break;
			case 4: mixxx_config_key_set("[Library]","MoveVertical", e->encoder.delta);
				break;
			case 5:
				if(e->encoder.delta > 0)
					mixxx_config_key_set(chans[d2->deck_id],"loop_double", 1);
				else
					mixxx_config_key_set(chans[d2->deck_id],"loop_halve", 1);
				break;
			default:
				printf("encoder %d, delta %d\n", e->encoder.id,
				       e->encoder.delta);
				break;
			}
			break;
		case CTLRA_EVENT_SLIDER:
			switch(e->slider.id) {
			case 0: mixxx_config_key_set("[Channel3]","volume", e->slider.value);
				 break;
			case 1: mixxx_config_key_set("[Channel1]","volume", e->slider.value);
				 break;
			case 2: mixxx_config_key_set("[Channel2]","volume", e->slider.value);
				 break;
			case 3: mixxx_config_key_set("[Channel4]","volume", e->slider.value);
				 break;
			case 4:
			case 5:
			case 6:
			case 7:
				 printf("%d : %f\n", e->slider.id, e->slider.value);
				 mixxx_config_key_set(chans_quickfx[e->slider.id-4],"super1", 0.15+(e->slider.value*0.7));
				 break;
			case 8:
				if(d2->shift_pressed)
					mixxx_config_key_set(chans[d2->deck_id],"playposition", e->slider.value);
				 break;
			default: printf("slider %d\n", e->slider.id); break;
			}

			break;
		default: break;
		}
	}
}

static uint32_t c;

void drawa_rect(uint8_t *pixels,
		int32_t x1, int32_t y1,
		int32_t width, int32_t height,
		int32_t fill, uint32_t fill_col);

void script_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	struct d2_t *d2 = userdata;
	const char *chan = chans[d2->deck_id];
	int play = mixxx_config_key_get(chan,"play_indicator");
	//printf("samples %d\n", samples);
	static const uint8_t chan_to_lin[] = { 1,2,0,3 };

	float playpos = mixxx_config_key_get(chans[0] ,"playposition");

	uint32_t deck_color     = (d2->deck_id < 2) ? 0xff : 0xff000000;
	uint32_t deck_color_low = (d2->deck_id < 2) ? 0x02 : 0x02000000;

	/* ABCD decks */
	for(int i = 0; i < 4; i++)
		ctlra_dev_light_set(dev,  38 + i, 0xffffffff * (d2->deck_id == i));

	for(int i = 0; i < 4; i++) {
		float vol = mixxx_config_key_get(chans[i],"volume");
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_ON_1+chan_to_lin[i],
				    (vol > 0.05f) ? 0xffffffff : 0x0);
	}

	for(int i = 0; i < 4; i++)
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_HOTCUE + i,
				    (d2->perf_mode == i) ?
				     deck_color : deck_color_low );

	/* filter light */
	//ctlra_dev_light_set(dev, 10, 0xffffffff * ((int)play) );
	const uint32_t high = 0xffffffff;
	const uint32_t low  = 0x02020202;

	/* hotcues */
	if(d2->perf_mode == 0) {
		int led;
		led = mixxx_config_key_get(chan,"hotcue_1_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_1, led);
		led = mixxx_config_key_get(chan,"hotcue_2_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_2, led);
		led = mixxx_config_key_get(chan,"hotcue_3_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_3, led);
		led = mixxx_config_key_get(chan,"hotcue_4_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_4, led);
		led = mixxx_config_key_get(chan,"hotcue_5_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_5, led);
		led = mixxx_config_key_get(chan,"hotcue_6_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_6, led);
		led = mixxx_config_key_get(chan,"hotcue_7_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_7, led);
		led = mixxx_config_key_get(chan,"hotcue_8_enabled") ? high : low;
		ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_8, led);
	} else if(d2->perf_mode == 1) {
		/* loop mode */
		for(int i = 0; i < 4; i++) {
			float start = mixxx_config_key_get(chans_nolin[i], "loop_start_position");
			float end   = mixxx_config_key_get(chans_nolin[i], "loop_end_position");

			int start_led = 0x00ef7000;
			int end_led   = 0x00ef00ff;

			if(start == -1)
				start_led = 0x0;

			if(end   == -1)
				end_led = 0x0;

			ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_1 + i, start_led);
			ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_5 + i, end_led);

		}
	} else if(d2->perf_mode == 3) {
		uint32_t led;
		int cue  = 0xffff0000;
		int nocue= 0xff020000;
		int play = 0xff00ff00;
		int stop = 0xff020202;

		for(int i = 0; i < 4; i++) {
			int play_ind  = mixxx_config_key_get(chans_nolin[i], "play_indicator");
			float playpos = mixxx_config_key_get(chans_nolin[i] ,"playposition");
			float track   = mixxx_config_key_get(chans_nolin[i] ,"track_loaded");

			led = play_ind ? play : stop;
			if(play_ind && playpos > 0.75) {
				/* Set greens, OR in the red bits */
				float v = (playpos - 0.75) * 8.;
				if(v > 1.f)
					v = 1.f;
				led  = ((uint32_t)(0xff * (1-v))) << 8; /* green */
				led |=  (uint32_t)(0xff * (  v));       /* red */
			}
			else if(!play_ind) {
				if(playpos >= 1.0f)
					led = 0x02; /* red */
				else
					led = 0x0200; /* green */
			}

			if(!track)
				led = 0x0;
			ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_5 + i, led);

			led = mixxx_config_key_get(chans_nolin[i], "cue_indicator") ? cue : nocue;
			if(!track)
				led = 0x0;
			ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_1 + i, led);
		}
	} else {
		for(int i = 0; i < 8; i++)
			ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_PAD_1 + i, 0);
	}


	float filter_1 = mixxx_config_key_get("[QuickEffectRack1_[Channel1]_Effect1]", "enabled");
	float filter_2 = mixxx_config_key_get("[QuickEffectRack1_[Channel2]_Effect1]", "enabled");
	float filter_3 = mixxx_config_key_get("[QuickEffectRack1_[Channel3]_Effect1]", "enabled");
	float filter_4 = mixxx_config_key_get("[QuickEffectRack1_[Channel4]_Effect1]", "enabled");
	ctlra_dev_light_set(dev,10, filter_1 > 0.5 ? 0xffffffff : 0);
	ctlra_dev_light_set(dev,11, filter_2 > 0.5 ? 0xffffffff : 0);
	ctlra_dev_light_set(dev, 9, filter_3 > 0.5 ? 0xffffffff : 0);
	ctlra_dev_light_set(dev,12, filter_4 > 0.5 ? 0xffffffff : 0);

	int cue_indicator = mixxx_config_key_get(chan,"cue_indicator");
	uint32_t cue_led_1 = cue_indicator ? 0xffffffff : 0x11111111;
	ctlra_dev_light_set(dev, NI_KONTROL_D2_LED_CUE , cue_led_1);

#if 0
// how to embed the helper functions?
	float playpos = mixxx_config_key_get(chan,"playposition");
	uint8_t blue[25];
	uint8_t oran[25];
	for(int i = 0; i < 25; i++) {
		blue[i] = 0;
		oran[i] = 0;
	}
	for(int i = 0; i < playpos * 25; i++)
	{
		blue[i] = 0xff;
	}
	oran[(int)(playpos * 24)] = 0xff;
	//ni_kontrol_d2_light_touchstrip(dev, oran, blue);
#endif

	/* cue/play */
	ctlra_dev_light_set(dev, 37, play ? 0xffffffff : 0x0101010101);

	//if(c++ % 20 == 0)
	ctlra_dev_light_flush(dev, 1);

	uint8_t *pixel_data;
	uint32_t bytes;
	int has_screen = ctlra_dev_screen_get_data(dev,
						   &pixel_data,
						   &bytes, 0);
	int beat = mixxx_config_key_get(chan,"beat_active");

#define WIDTH  480
#define HEIGHT 272

	static uint32_t usb_cnt;

	if(has_screen == 0) {
		int32_t pos_px = (int32_t)(WIDTH * playpos);
		//printf("%f, px %d\n", playpos, pos_px);

		memset(pixel_data, 0, WIDTH*HEIGHT*2);

		for(int j = 0; j < 20; j++) {
			const uint32_t off = j * WIDTH * 2;
			for(int i = 0; i < pos_px * 2; i += 2) {
				pixel_data[off+i  ] = 0xff;
				pixel_data[off+i+1] = 0xff;
			}
		}

		drawa_rect(pixel_data, 0, 45, pos_px, 20, 1, -1);

		float playpos2 = mixxx_config_key_get(chans[1],"playposition");
		int32_t pos_px2 = (int32_t)(WIDTH * playpos2);
		//drawa_rect(pixel_data, 0, 65, pos_px2, 20, 1, -1);

		/* outstanding writes will cause segfaults on free? */
		//if(usb_cnt++ % 10 == 0)
			ctlra_dev_screen_get_data(dev, &pixel_data, &bytes, 1);
	}
}

void drawa_rect(uint8_t *pixels,
		int32_t x1, int32_t y1,
		int32_t width, int32_t height,
		int32_t fill, uint32_t fill_col)
{
	uint16_t *px = (uint16_t *)pixels;
	const uint16_t uint16_max = -1;
	const int32_t hi_yidx = WIDTH * y1;
	const int32_t lo_yidx = WIDTH * (y1 + height);

	const int32_t le_xidx = x1;
	const int32_t ri_xidx = x1 + width;

	/*
	for(int i = x1; i < x1 + width; i++) {
		px[hi_yidx + i] = uint16_max;
		px[lo_yidx + i] = uint16_max;
	}

	for(int i = y1; i < y1 + height; i++) {
		px[le_xidx + (i * WIDTH)] = uint16_max;
		px[ri_xidx + (i * WIDTH)] = uint16_max;
	}
	*/
	
	if( 1 ) {
		uint16_t line[WIDTH];
		for(int i = 0; i < WIDTH; i++) {
			line[i] = 0b11100000;
		}

		for(int i = y1; i < y1 + height; i++) {
			for(int j = 0; j < width; j += 2) {
				px[le_xidx + (i * WIDTH /2) + j/2   ] = 0b1111100000000000;
				px[le_xidx + (i * WIDTH /4) + j/2 +1] = 0b1111100000000000;
				//px[le_xidx + (i * WIDTH) + j + 1] = 0xffff;
			}
		}
	}
}
