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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>

#include "font.h"
#include "weather.h"
#include "common.h"

#define SCREEN_WIDTH  341
#define SCREEN_HEIGHT 270

SDL_Renderer *renderer;
static SDL_Window *window;

/* Background image. */
static SDL_Texture *bg_tex;

/* Icon background (sun, moon...). */
static SDL_Texture *bg_icon_tex;

static int screen_width  = SCREEN_WIDTH;
static int screen_height = SCREEN_HEIGHT;

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
static SDL_Color color_blue  = {148,199,228};
static SDL_Color color_white = {255,255,255};
static SDL_Color color_gray  = {146,148,149};
static SDL_Color color_cloudy_gray = {162,179,189};
static SDL_Color color_black = {0,0,0};

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
struct weather_info wi = {0};

/* Forward definitions. */
static void render_image(SDL_Texture *tex, int x, int y);
static void create_texts(
	const SDL_Color *days_color,
	const SDL_Color *max_temp_color,
	const SDL_Color *hdr_color);


/**
 * Update logic and drawing for each frame
 */
static inline void update_frame(void)
{
	/* ----------------------------------------------------------------- */
	/* Update drawing                                                    */
	/* ----------------------------------------------------------------- */

	/* Draw background alpha image. */
	SDL_RenderClear(renderer);

	/* Backgruond and icon. */
	SDL_RenderTexture(renderer, bg_tex, NULL, NULL);
	render_image(bg_icon_tex, 0,0);

	font_render_text(&txt_footer, FOOTER_X, FOOTER_Y);
	font_render_text(&txt_day1, DAY1_X, DAY_Y);
	font_render_text(&txt_day2, DAY2_X, DAY_Y);
	font_render_text(&txt_day3, DAY3_X, DAY_Y);

	font_render_text(&txt_day1_max, DAY1_X, DAYMAX_Y);
	font_render_text(&txt_day2_max, DAY2_X, DAYMAX_Y);
	font_render_text(&txt_day3_max, DAY3_X, DAYMAX_Y);

	font_render_text(&txt_day1_min, DAY1_X, DAYMIN_Y);
	font_render_text(&txt_day2_min, DAY2_X, DAYMIN_Y);
	font_render_text(&txt_day3_min, DAY3_X, DAYMIN_Y);

	font_render_text(&txt_curr_temp,   HDR_MAX_X - txt_curr_temp.width,   HDR_TEMP_Y);
	font_render_text(&txt_curr_cond,   HDR_MAX_X - txt_curr_cond.width,   HDR_COND_Y);
	font_render_text(&txt_curr_minmax, HDR_MAX_X - txt_curr_minmax.width, HDR_MINMAX_Y);
	font_render_text(&txt_location,    HDR_MAX_X - txt_location.width,    HDR_LOC_Y);

	render_image(fc_day1_tex, DAY1_IMG_X, DAY_IMG_Y);
	render_image(fc_day2_tex, DAY2_IMG_X, DAY_IMG_Y);
	render_image(fc_day3_tex, DAY3_IMG_X, DAY_IMG_Y);

	/* Render everything. */
	SDL_RenderPresent(renderer);

}

static int create_sdl_window(int w, int h, int flags)
{
	if (SDL_CreateWindowAndRenderer(
		w, h, flags, &window, &renderer) == -1)
		panic("Unable to create window and renderer!\n");

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return (0);
}

/**
 *
 */
static int load_window_bg(const char *file)
{
	int w, h;

	if (bg_tex)
		SDL_DestroyTexture(bg_tex);

	bg_tex = IMG_LoadTexture(renderer, file);
	if (!bg_tex)
		panic("Unable to load image! (%s)\n");

	SDL_QueryTexture(bg_tex, NULL, NULL, &w, &h);
	SDL_SetWindowSize(window, w, h);
	return (0);
}

/**
 *
 */
static void free_image(SDL_Texture **tex)
{
	if (!*tex)
		return;
	SDL_DestroyTexture(*tex);
	tex = NULL;
}

/**
 *
 */
static void load_image(SDL_Texture **tex, const char *img)
{
	free_image(tex);
	*tex = IMG_LoadTexture(renderer, img);
	if (!*tex)
		panic("Unable to load texture (%s)\n", img);
}

/**
 *
 */
static void render_image(SDL_Texture *tex, int x, int y)
{
	SDL_FRect rect;
	int w, h;

	if (!tex)
		return;

	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_RenderTexture(renderer, tex, NULL, &rect);
}

/**
 *
 */
static void update_weather_info(void)
{
	const SDL_Color *cd, *cmt, *chdr;
	const char *bg_icon_tex_path;
	const char *wbg;

	char buff1[32] = {0};
	char buff2[32] = {0};
	char buff3[32] = {0};

	if (weather_get("cat inp", &wi) < 0)
		info("Unable to get weather info!\n");

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

	free_image(&bg_icon_tex);
	load_window_bg(wbg);
	if (bg_icon_tex_path)
		load_image(&bg_icon_tex, bg_icon_tex_path);

	create_texts(cd, cmt, chdr);

	/* Forecast days icons. */
	snprintf(buff1, sizeof buff1, "assets/%s.png",
		wi.forecast[0].condition);
	snprintf(buff2, sizeof buff2, "assets/%s.png",
		wi.forecast[1].condition);
	snprintf(buff3, sizeof buff3, "assets/%s.png",
		wi.forecast[2].condition);

	load_image(&fc_day1_tex, buff1);
	load_image(&fc_day2_tex, buff2);
	load_image(&fc_day3_tex, buff3);
}

/**
 *
 */
static int load_fonts(void)
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
 *
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
		"wonday",
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
 *
 */
void free_resources(void)
{
	free_image(&bg_icon_tex);
	free_image(&fc_day1_tex);
	free_image(&fc_day2_tex);
	free_image(&fc_day3_tex);
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
}

/**
 * Main weather loop.
 */
int main(void)
{
	SDL_Event event;
	uint32_t start_time;
	uint32_t end_time;
	uint32_t delta_time;

	/* Initialize. */
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		panic("SDL could not initialize!: %s\n", SDL_GetError());
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
		panic("Unable to initialize SDL_image!\n");
	if (font_init() < 0)
		panic("Unable to initialize SDL_ttf!\n");


	create_sdl_window(
		screen_width, screen_height,
		SDL_WINDOW_TRANSPARENT|
		SDL_WINDOW_BORDERLESS/*|
		SDL_WINDOW_UTILITY*/);

	load_window_bg("assets/bg_sunny_day.png");
	load_fonts();

	update_weather_info();

	while (1)
	{
		while (SDL_PollEvent(&event) != 0)
			if (event.type == SDL_EVENT_QUIT)
				goto quit;

		update_frame();
		SDL_Delay(500);
    }

quit:
	free_resources();

	if (bg_tex)
		SDL_DestroyTexture(bg_tex);
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	font_quit();
	IMG_Quit();
	SDL_Quit();
}
