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
 * @brief Initializes the SDL_ttf font lib.
 */
int font_init(void) {
	return TTF_Init();
}

/**
 * @brief De-initializes the SDL_ttf font lib.
 */
void font_quit(void) {
	TTF_Quit();
}

/**
 * @brief Open a given font for a given @p file and
 * size @p ptsize.
 *
 * @param file   Font path to be loaded.
 * @param ptsize Desired font size.
 *
 * @return Returns a pointer to the loaded font.
 */
TTF_Font *font_open(const char *file, int ptsize) {
	return TTF_OpenFont(file, ptsize);
}

/**
 * @brief Close a previously loaded font.
 *
 * @param font Loaded font to be closed.
 */
void font_close(TTF_Font *font) {
	TTF_CloseFont(font);
}

/**
 * @brief Creates a new SDL_Texture for a given @p text,
 * @p color and @p font, returning the result into @p rt.
 *
 * @param rt    Rendered text structure pointer.
 * @param font  Already opened TTF font.
 * @param text  Text to be created.
 * @param color SDL color
 *
 * @note If there is an previous allocated text, it will
 * be destroyed first, so multiples calls to this is
 * safe.
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
 * @brief Destroy a previously created text.
 *
 * @param rt Rendered text to be destroyed.
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
 * @brief Copy the text texture pointed by @p rt into the
 * renderer, at coordinates @p x and @p y.
 *
 * This is an small wrapper around 'SDL_RenderTexture',
 * with an additional check if the text/texture pointer
 * is valid or not, i.e., this function can be called
 * at any time, even if the texture does not exist
 * (NULL).
 *
 * @param rt Text to be rendered.
 * @param x  Screen X coordinate.
 * @param y  Screen Y coordinate.
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
