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
 * @param new_size New size (in UTF-8 characters) of the
 *                 truncated text.
 *
 * @return Returns a new string containing the truncated
 * text.
 *
 * @note The @p new_size should be less than the current
 * size (in UTF-8 characters) of the string, examples:
 *
 * Input (new_size) -> output:
 * abcdef (4) -> a...
 * abc    (3) -> abc
 * abc    (4) -> abc
 * abcdef (6) -> abcdef
 * abcdef (5) -> ab...
 * abcdef (3) -> NULL
 * abcdef (2) -> NULL
 * ...
 */
static char *utf8_truncate(const char *u8_txt, size_t new_size)
{
	char *s;
	size_t u8_len;
	size_t byte_count;
	size_t length_count;

	u8_len = utf8_strlen(u8_txt);

	/*
	 * If the required new size is already less
	 * than or equal the current character amount,
	 * there is no need to add ellipsis.
	 */
	if (u8_len <= new_size)
		return (strdup(u8_txt));

	/* Returns NULL if the required size is less than the
	 * ellipsis size (because doesn't make sense...) */
	else if (new_size <= 3)
		return (NULL);

	s            = (char*)u8_txt;
	u8_len       = utf8_strlen(u8_txt);
	byte_count   = 0;
	length_count = 0;

	/*
	 * Since our string needs to have at most new_size
	 * characters, our new size will be new_size-3,
	 * because we're adding ellipsis to the text.
	 */
	new_size -= 3;

	/* Count how may bytes have the desired size. */
	while (*s++ && length_count < new_size) {
		if ((*s & 0xC0) != 0x80)
			length_count++;
		byte_count++;
	}

	/* Allocate new string: utf8 + ... + \0. */
	s = calloc(1, byte_count + 3 + 1);
	if (!s)
		log_oom("Unable to allocate UTF8 string!\n");

	memcpy(s + 0,          u8_txt, byte_count);
	memcpy(s + byte_count, "...", 3);
	return (s);
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
	int extent, count;

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
		TTF_MeasureUTF8(font, text, mwidth, &extent, &count);
		if ((size_t)count < utf8_strlen(text)) {
			new_text = utf8_truncate(text, count);
			text     = new_text;
		}
	}

	/* Create a new one. */
	s = TTF_RenderUTF8_Blended(font, text, *color);
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
