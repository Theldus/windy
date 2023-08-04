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

#include <SDL3/SDL.h>
#include "font.h"
#include "common.h"

/**
 *
 */
int font_init(void) {
	return TTF_Init();
}

/**
 *
 */
void font_quit(void) {
	TTF_Quit();
}

/**
 *
 */
TTF_Font *font_open(const char *file, int ptsize) {
	return TTF_OpenFont(file, ptsize);
}

/**
 *
 */
void font_close(TTF_Font *font) {
	TTF_CloseFont(font);
}

/**
 *
 */
void font_create_text(struct rendered_text *rt, TTF_Font *font,
	const char *text, const SDL_Color *color)
{
	int ret;
	SDL_Surface *s;

	ret = -1;

	if (!rt)
		return;

	/* Clear previous text, if any. */
	font_destroy_text(rt);

	/* Create a new one. */
	s = TTF_RenderUTF8_Blended(font, text, *color);
	if (!s)
		panic("Unable to create font surface!\n");

	rt->text_texture = SDL_CreateTextureFromSurface(renderer, s);
	if (!rt->text_texture)
		panic("Unable to create font texture!\n");

	rt->width  = s->w;
	rt->height = s->h;
	SDL_DestroySurface(s);
}

/**
 *
 */
void font_destroy_text(struct rendered_text *rt)
{
	if (!rt || !rt->text_texture)
		return;

	SDL_DestroyTexture(rt->text_texture);
	rt->text_texture = NULL;
	rt->width  = 0;
	rt->height = 0;
}

/**
 *
 */
void font_render_text(struct rendered_text *rt, int x, int y)
{
	if (!rt || !rt->text_texture)
		return;

	SDL_FRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = rt->width;
	rect.h = rt->height;
	SDL_RenderTexture(renderer, rt->text_texture, NULL, &rect);
}
