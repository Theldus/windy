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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "deps/stb_image.h"

/**
 * @brief If the texture pointed by @p tex exists,
 * free it, otherwise, do nothing.
 *
 * @param tex Texture pointer.
 */
void image_free(SDL_Texture **tex)
{
	if (!*tex)
		return;
	SDL_DestroyTexture(*tex);
	*tex = NULL;
}

/**
 * @brief Load a given image path pointed by @p img, and
 * save into the texture pointer pointed by @p tex.
 *
 * If the texture pointer already points to an existing
 * texture, the old texture is deallocated first.
 *
 * @param tex Texture pointer to be loaded.
 * @param img Image path.
 */
void image_load(SDL_Texture **tex, const char *img)
{
	int w, h, comp;
	SDL_Surface *s;
	unsigned char *buff;

	/* Silence 'defined but not used' stb_image warnings. */
	((void)stbi__addints_valid);
	((void)stbi__mul2shorts_valid);

	image_free(tex);

	comp = 4;
	buff = stbi_load(img, &w, &h, &comp, 0);
	if (!buff)
		panic("Unable to load image: %s!\n", img);

    s = SDL_CreateSurfaceFrom(buff, w, h, 4*w, SDL_PIXELFORMAT_RGBA32);
	if (!s)
		panic("Unable to create image surface!: %s\n", SDL_GetError());

	*tex = SDL_CreateTextureFromSurface(renderer, s);
	if (!*tex)
		panic("Unable to create image texture!\n");

	SDL_DestroySurface(s);
	stbi_image_free(buff);
}

/**
 * @brief Copy the texture pointed by @p tex into the
 * renderer, at coordinates @p x and @p y.
 *
 * This is an small wrapper around 'SDL_RenderTexture',
 * with an additional check if the texture pointer is
 * valid or not, i.e., this function can be called
 * at any time, even if the texture does not exist
 * (NULL).
 *
 * @param tex Texture to be rendered.
 * @param x   Screen X coordinate.
 * @param y   Screen Y coordinate.
 */
void image_render(SDL_Texture *tex, int x, int y)
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
