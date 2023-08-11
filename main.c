/*
 * MIT License
 *
 * Copyright (c) 2023 Davidson Francis <davidsondfgl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <SDL3/SDL.h>

#include "font.h"
#include "weather.h"
#include "image.h"
#include "common.h"

/* Window size. */
#define SCREEN_WIDTH  341
#define SCREEN_HEIGHT 270

SDL_Renderer *renderer;
static SDL_Window *window;

/* Background image. */
static SDL_Texture *bg_tex;

/* Icon background (sun, moon...). */
static SDL_Texture *bg_icon_tex;

/* Loaded fonts and rendered texts. */
static TTF_Font *font_16pt;
static TTF_Font *font_18pt;
static TTF_Font *font_40pt;
static struct rendered_text txt_footer;
static struct rendered_text txt_day1, txt_day2, txt_day3;
static struct rendered_text txt_day1_min, txt_day1_max;
static struct rendered_text txt_day2_min, txt_day2_max;
static struct rendered_text txt_day3_min, txt_day3_max;
static struct rendered_text txt_location;
static struct rendered_text txt_curr_minmax;
static struct rendered_text txt_curr_cond;
static struct rendered_text txt_curr_temp;

/* Forecast icons textures. */
static SDL_Texture *fc_day1_tex;
static SDL_Texture *fc_day2_tex;
static SDL_Texture *fc_day3_tex;

/* Text colors. */
static SDL_Color color_blue  = {148,199,228,SDL_ALPHA_OPAQUE};
static SDL_Color color_white = {255,255,255,SDL_ALPHA_OPAQUE};
static SDL_Color color_gray  = {146,148,149,SDL_ALPHA_OPAQUE};
static SDL_Color color_cloudy_gray = {162,179,189,SDL_ALPHA_OPAQUE};
static SDL_Color color_black = {0,0,0,SDL_ALPHA_OPAQUE};

/* Footer text, i.e., where the weather data
 * were obtained. */
#define FOOTER_X  21
#define FOOTER_Y 222

/* Forecast days. */
#define DAY_Y    151 /* Y-axis for the days text.   */
#define DAY1_X    16 /* Day 1 text forecast X-axis. */
#define DAY2_X   116 /* Day 2 text forecast X-axis. */
#define DAY3_X   230 /* Day 3 text forecast X-axis. */
#define DAYMAX_Y 171 /* Day max 1-2-3 text Y-axis.  */
#define DAYMIN_Y 189 /* Day min 1-2-3 text Y-axis.  */

#define DAY_IMG_Y  165 /* Forecast images Y-axis.   */
#define DAY1_IMG_X  45 /* Forecast day1 img X-axis. */
#define DAY2_IMG_X 146 /* Forecast day1 img X-axis. */
#define DAY3_IMG_X 257 /* Forecast day1 img X-axis. */

/*
 * Header values
 *
 * Max X value to the header text,
 * which includes:
 * - Current temperature
 * - Weather condition
 * - Min/max temperature
 * - Location
 *
 * X value is dynamically calculated
 */
#define HDR_MAX_X    310

/* Header Y-values. */
#define HDR_TEMP_Y    15
#define HDR_COND_Y    60
#define HDR_MINMAX_Y  83
#define HDR_LOC_Y    120

/* Current weather info. */
static struct weather_info wi = {0};

/* Command-line arguments. */
static struct args {
	const char *execute_command;
	Uint32 update_weather_time_ms;
	int x;
	int y;
} args = {
	.execute_command = NULL,
	.update_weather_time_ms = 600*1000,
	.x = -1,
	.y = -1
};

/* Forward definitions. */
static void create_texts(
	const SDL_Color *days_color,
	const SDL_Color *max_temp_color,
	const SDL_Color *hdr_color);
static Uint32 update_weather_cb(Uint32 interval,
	void *data);

/**
 * Update logic and drawing for each frame
 */
static inline void update_frame(void)
{
	/* Draw background alpha image. */
	SDL_RenderClear(renderer);

	/* Backgruond and icon. */
	SDL_RenderTexture(renderer, bg_tex, NULL, NULL);
	image_render(bg_icon_tex, 0,0);

	/* Footer and forecast days text. */
	font_render_text(&txt_footer, FOOTER_X, FOOTER_Y);
	font_render_text(&txt_day1, DAY1_X, DAY_Y);
	font_render_text(&txt_day2, DAY2_X, DAY_Y);
	font_render_text(&txt_day3, DAY3_X, DAY_Y);

	/* Forecast days min and max temp. */
	font_render_text(&txt_day1_max, DAY1_X, DAYMAX_Y);
	font_render_text(&txt_day2_max, DAY2_X, DAYMAX_Y);
	font_render_text(&txt_day3_max, DAY3_X, DAYMAX_Y);
	font_render_text(&txt_day1_min, DAY1_X, DAYMIN_Y);
	font_render_text(&txt_day2_min, DAY2_X, DAYMIN_Y);
	font_render_text(&txt_day3_min, DAY3_X, DAYMIN_Y);

	/* Header: curr temp, condition, min/max and location. */
	font_render_text(&txt_curr_temp,   HDR_MAX_X - txt_curr_temp.width,   HDR_TEMP_Y);
	font_render_text(&txt_curr_cond,   HDR_MAX_X - txt_curr_cond.width,   HDR_COND_Y);
	font_render_text(&txt_curr_minmax, HDR_MAX_X - txt_curr_minmax.width, HDR_MINMAX_Y);
	font_render_text(&txt_location,    HDR_MAX_X - txt_location.width,    HDR_LOC_Y);

	/* Forecast icons based on weather condition. */
	image_render(fc_day1_tex, DAY1_IMG_X, DAY_IMG_Y);
	image_render(fc_day2_tex, DAY2_IMG_X, DAY_IMG_Y);
	image_render(fc_day3_tex, DAY3_IMG_X, DAY_IMG_Y);

	/* Render everything. */
	SDL_RenderPresent(renderer);
}

/**
 * @brief Creates the current SDL window and renderer
 * with given width @p w, height @P h and @p flags.
 *
 * @param w     Window width.
 * @param h     Window height.
 * @param flags Window flags.
 */
static int create_sdl_window(int w, int h, int flags)
{
	if (SDL_CreateWindowAndRenderer(
		w, h, flags, &window, &renderer) == -1)
		panic("Unable to create window and renderer!\n");

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	/* Set coordinates. */
	if (args.x >= 0 && args.y >= 0)
		SDL_SetWindowPosition(window, args.x, args.y);

	return (0);
}

/**
 * @brief 'Main' weather update routine
 *
 * Executes the command given, read its output in stdout,
 * parse its json and then choose which text/icons should
 * be loaded into the screen.
 */
static void update_weather_info(void)
{
	const SDL_Color *cd, *cmt, *chdr;
	const char *bg_icon_tex_path;
	const char *wbg;

	char buff1[32] = {0};
	char buff2[32] = {0};
	char buff3[32] = {0};

	if (weather_get(args.execute_command, &wi) < 0)
		errto(out, "Unable to get weather info!\n");

	/* Icon to be loaded if not 'clear'. */
	snprintf(buff1,
		sizeof buff1,
		"assets/bg_icon_%s.png",
		wi.condition);

	bg_icon_tex_path = buff1;

	/* Load background depending of time and weather condition. */
	if (!weather_is_day()) {
		wbg = "assets/bg_night.png";

		/*
		 * Load moon or other weather icons, accordingly
		 * to the weather condition.
		 */
		if (!strcmp(wi.condition, "clear"))
			bg_icon_tex_path = weather_get_moon_phase_icon();

		cd   = &color_gray;
		cmt  = &color_white;
		chdr = &color_white;
	}

	/* If day. */
	else {
		/* If clear. */
		if (!strcmp(wi.condition, "clear")) {
			bg_icon_tex_path = NULL;
			wbg  = "assets/bg_sunny_day.png";
			cd   = &color_blue;
			cmt  = &color_white;
			chdr = &color_black;
		}

		/* Anything else, should load bg and icon. */
		else {
			wbg  = "assets/bg_notclear_day.png";
			cd   = &color_cloudy_gray;
			cmt  = &color_white;
			chdr = &color_black;
		}
	}

	image_free(&bg_icon_tex);
	image_load(&bg_tex, wbg);
	if (bg_icon_tex_path)
		image_load(&bg_icon_tex, bg_icon_tex_path);

	create_texts(cd, cmt, chdr);

	/* Forecast days icons. */
	snprintf(buff1, sizeof buff1, "assets/%s.png",
		wi.forecast[0].condition);
	snprintf(buff2, sizeof buff2, "assets/%s.png",
		wi.forecast[1].condition);
	snprintf(buff3, sizeof buff3, "assets/%s.png",
		wi.forecast[2].condition);

	image_load(&fc_day1_tex, buff1);
	image_load(&fc_day2_tex, buff2);
	image_load(&fc_day3_tex, buff3);

out:
	SDL_AddTimer(args.update_weather_time_ms,
		update_weather_cb, NULL);
}

/**
 * @brief Load the same font in three diferent sizes
 * for the GUI texts.
 */
static void load_fonts(void)
{
	/* Load require fonts and sizes. */
	font_16pt = font_open("assets/fonts/NotoSans-Regular.ttf", 16);
	if (!font_16pt)
		panic("Unable to open font with size 16pt!\n");
	font_18pt = font_open("assets/fonts/NotoSans-Regular.ttf", 18);
	if (!font_18pt)
		panic("Unable to open font with size 18pt!\n");
	font_40pt = font_open("assets/fonts/NotoSans-Regular.ttf", 40);
	if (!font_40pt)
		panic("Unable to open font with size 40pt!\n");
}

/**
 * @brief Create all texts/textures to the GUI.
 *
 * @param days_color     Color of footer, days and min temp.
 * @param max_temp_color Color of the max temp color for the
 *                       forecast days.
 * @param hdr_color      Header color (curr temp, location...)
 */
static void create_texts(
	const SDL_Color *days_color,
	const SDL_Color *max_temp_color,
	const SDL_Color *hdr_color)
{
	int d1, d2, d3;
	char buff1[32] = {0};
	char buff2[32] = {0};
	char buff3[32] = {0};

	static const char *const days_of_week[] = {
		"sunday",
		"monday",
		"tuesday",
		"wednesday",
		"thursday",
		"friday",
		"saturday"
	};

	weather_get_forecast_days(&d1, &d2, &d3);

	/* Footer. */
	font_create_text(&txt_footer, font_16pt, wi.provider, days_color);

	/* Forecast days string. */
	font_create_text(&txt_day1, font_16pt, days_of_week[d1], days_color);
	font_create_text(&txt_day2, font_16pt, days_of_week[d2], days_color);
	font_create_text(&txt_day3, font_16pt, days_of_week[d3], days_color);

	/* Max temperature value. */
	snprintf(buff1, sizeof buff1, "%dº", wi.forecast[0].max_temp);
	snprintf(buff2, sizeof buff2, "%dº", wi.forecast[1].max_temp);
	snprintf(buff3, sizeof buff3, "%dº", wi.forecast[2].max_temp);

	font_create_text(&txt_day1_max, font_16pt, buff1, max_temp_color);
	font_create_text(&txt_day2_max, font_16pt, buff2, max_temp_color);
	font_create_text(&txt_day3_max, font_16pt, buff3, max_temp_color);

	/* Min temperature value. */
	snprintf(buff1, sizeof buff1, "%dº", wi.forecast[0].min_temp);
	snprintf(buff2, sizeof buff2, "%dº", wi.forecast[1].min_temp);
	snprintf(buff3, sizeof buff3, "%dº", wi.forecast[2].min_temp);

	font_create_text(&txt_day1_min, font_16pt, buff1, days_color);
	font_create_text(&txt_day2_min, font_16pt, buff2, days_color);
	font_create_text(&txt_day3_min, font_16pt, buff3, days_color);

	/* Header: location, max/min, current condition and temperature. */
	snprintf(buff1, sizeof buff1, "%dº - %dº", wi.max_temp, wi.min_temp);
	snprintf(buff2, sizeof buff2, "%c%s",
		toupper(wi.condition[0]), wi.condition+1);
	snprintf(buff3, sizeof buff3, "%dº", wi.temperature);

	font_create_text(&txt_location, font_18pt, wi.location, hdr_color);
	font_create_text(&txt_curr_minmax, font_18pt, buff1, hdr_color);
	font_create_text(&txt_curr_cond, font_18pt, buff2, hdr_color);
	font_create_text(&txt_curr_temp, font_40pt, buff3, hdr_color);
}

/**
 * @brief Free all fonts and textures used.
 */
void free_resources(void)
{
	/* Free textures. */
	image_free(&bg_tex);
	image_free(&bg_icon_tex);
	image_free(&fc_day1_tex);
	image_free(&fc_day2_tex);
	image_free(&fc_day3_tex);
	font_destroy_text(&txt_footer);
	font_destroy_text(&txt_day1);
	font_destroy_text(&txt_day2);
	font_destroy_text(&txt_day3);
	font_destroy_text(&txt_day1_max);
	font_destroy_text(&txt_day2_max);
	font_destroy_text(&txt_day3_max);
	font_destroy_text(&txt_day1_min);
	font_destroy_text(&txt_day2_min);
	font_destroy_text(&txt_day3_min);
	font_destroy_text(&txt_curr_temp);
	font_destroy_text(&txt_curr_cond);
	font_destroy_text(&txt_curr_minmax);
	font_destroy_text(&txt_location);
	font_destroy_text(&txt_curr_temp);

	/* Close loaded fonts. */
	font_close(font_16pt);
	font_close(font_18pt);
	font_close(font_40pt);
}

/**
 * @brief SDL timer callback to update the weather
 *
 * @param interval Current timer interval
 * @param data Custom data
 * @return Returns the time of the next callback, 0
 * to disable.
 */
static Uint32 update_weather_cb(Uint32 interval, void *data)
{
	((void)interval);
	((void)data);
	SDL_Event event;
	event.type       = SDL_EVENT_USER;
	event.user.data1 = NULL;
	SDL_PushEvent(&event);
	return (0);
}

/**
 * @brief Show program usage.
 * @param prgname Program name.
 */
void usage(const char *prgname)
{
	fprintf(stderr, "Usage: %s [options] -c <command-to-run>\n",
		prgname);
	fprintf(stderr,
		"Options:\n"
		"  -t           Interval time (in seconds) to check for weather\n"
		"               updates (default = 10 minutes)\n"
		"  -c <command> Command to execute when the update time reaches\n"
		"  -x <pos>     Set the window X coordinate"
		"  -y <pos>     Set the window Y coordinate"
		"  -h           This help\n\n"
		"Example:\n"
		" Update the weather info each 30 minutes, by running the command\n"
		" 'python request.py'\n"
		"    $ %s -t 1800 -c \"python request.py\"\n\n"
		"Obs: Options -t,-x and -y are not required, -c is required!\n",
		prgname);
	exit(EXIT_FAILURE);
}

/**
 * @brief Parse command-line arguments.
 *
 * @param argc Argument count.
 * @param argv Argument list.
 */
void parse_args(int argc, char **argv)
{
	int c; /* Current arg. */
	while ((c = getopt(argc, argv, "t:c:x:y:h")) != -1)
	{
		switch (c) {
		case 'h':
			usage(argv[0]);
			break;
		case 't':
			args.update_weather_time_ms = atoi(optarg)*1000;
			if (!args.update_weather_time_ms) {
				info("Invalid -t value, please choose a valid interval!\n");
				usage(argv[0]);
			}
			break;
		case 'c':
			args.execute_command = optarg;
			break;
		case 'x':
			args.x = atoi(optarg);
			break;
		case 'y':
			args.y = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if (!args.execute_command) {
		info("Option -c is required!\n");
		usage(argv[0]);
	}
}

/**
 * Main weather loop.
 */
int main(int argc, char **argv)
{
	SDL_Event event;
	char *base_path;

	parse_args(argc, argv);

	/* Initialize. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
		panic("SDL could not initialize!: %s\n", SDL_GetError());
	if (font_init() < 0)
		panic("Unable to initialize SDL_ttf!\n");

	base_path = SDL_GetBasePath();
	if (!base_path)
		panic("Unable to get program base path!\n");
	chdir(base_path);

	create_sdl_window(
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_TRANSPARENT|
		SDL_WINDOW_BORDERLESS|
		SDL_WINDOW_UTILITY);

	image_load(&bg_tex, "assets/bg_sunny_day.png");
	load_fonts();

	update_weather_info();

	SDL_AddTimer(args.update_weather_time_ms,
		update_weather_cb, NULL);

	while (1)
	{
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_EVENT_QUIT)
				goto quit;
			else if (event.type == SDL_EVENT_USER)
				update_weather_info();
		}

		update_frame();
		SDL_Delay(500);
	}

quit:
	free_resources();

	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	SDL_free(base_path);
	font_quit();
	SDL_Quit();
}
