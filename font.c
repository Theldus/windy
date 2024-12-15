/*
 * MIT License
 *
 * Copyright (c) 2023-2024 Davidson Francis <davidsondfgl@gmail.com>
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

#include <string.h>
#include <SDL3/SDL.h>
#include "font.h"
#include "log.h"

extern SDL_Renderer *renderer;

/**
 * @brief Calculate the length of an UTF-8 encoded string
 *
 * @param s UTF-8 string.
 *
 * @return Returns the amount of characters in the
 * string.
 */
static inline size_t utf8_strlen(const char *s)
{
	size_t len;
	len = 0;
	while (*s) {
		if ((*s & 0xc0) != 0x80)
			len++;
		s++;
	}
	return (len);
}

/**
 * @brief For a given UTF-8 encoded text in @p u8_txt, returns
 * a new string of @p new_size characters, adding ellipsis
 * at the end of the text.
 *
 * @param u8_txt   UTF-8 text to be truncated.
 * @param max_size Max size (in bytes) of the truncated text.
 *
 * @return Returns a new string containing the truncated
 * text.
 */
static char *utf8_truncate(const unsigned char *u8_txt, size_t max_size)
{
	const unsigned char *s;
	size_t codep_cnt;
	char *new;

	codep_cnt = 0;

	/*
	 * Walk 3 codepoints right before the target byte, then
	 * add our ellipsis.
	 *
	 * Since characters might be bigger than others, 3 codepoints
	 * is not equal to 3 '...', i.e., 3 arbitrary UTF8 chars might
	 * not have the same width as 3 dots, but most likely will fit
	 * 3 dots.
	 */
	s = u8_txt + max_size - 1;
	while (s > u8_txt && codep_cnt < 3) {
		/* If new codepoint or a single ascii char. */
		if ( ((*s & 0xC0) != 0x80) || (*s <= 127) )
			codep_cnt++;
		s--;
		max_size--;
	}

	/* Allocate new string: utf8 + ... + \0. */
	new = calloc(1, max_size + 3 + 1);
	if (!new)
		log_oom("Unable to allocate UTF8 string!\n");

	memcpy(new + 0,        u8_txt, max_size);
	memcpy(new + max_size, "...", 3);
	return (new);
}

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
 * @param rt     Rendered text structure pointer.
 * @param font   Already opened TTF font.
 * @param text   Text to be created.
 * @param color  SDL color
 * @param mwidth Maximum text width (in pixels, 0 to not
 *               check). If the text exceeds the maximum
 *               size, ellipsis will be added.
 *
 * @note If there is an previous allocated text, it will
 * be destroyed first, so multiples calls to this is
 * safe.
 */
void font_create_text(struct rendered_text *rt, TTF_Font *font,
	const char *text, const SDL_Color *color, unsigned mwidth)
{
	SDL_Surface *s;
	char *new_text;
	size_t count;

	new_text = NULL;

	if (!rt)
		return;

	/* Clear previous text, if any. */
	font_destroy_text(rt);

	/*
	 * Check maximum width was provided _and_ if the
	 * the text exceeds the maximum size.
	 */
	if (mwidth) {
		TTF_MeasureString(font, text, 0, mwidth, NULL, &count);

		/* If the reported count (in bytes) is less than
		 * our string length, truncate it. */
		if (count < strlen(text)) {
			new_text = utf8_truncate((const unsigned char*)text, count);
			text     = new_text;
		}
	}

	/* Create a new one. */
	s = TTF_RenderText_Blended(font, text, 0, *color);
	if (!s)
		log_panic("Unable to create font surface!\n");

	rt->text_texture = SDL_CreateTextureFromSurface(renderer, s);
	if (!rt->text_texture)
		log_panic("Unable to create font texture!\n");

	rt->width  = s->w;
	rt->height = s->h;
	SDL_DestroySurface(s);
	free(new_text);
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
