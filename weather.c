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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.h"
#include "weather.h"
#include "common.h"

#define LUNAR_CYCLE_CONSTANT 29.53058770576
#define BUF_CAPACITY 16

/* Append buffer. */
struct abuf {
	char *str;
	size_t len;
	size_t capacity;
};

/* Moon phases path. */
const char* moon_phases[] = {
	"assets/bg_icon_new_moon.png",
	"assets/bg_icon_first_quarter.png",
	"assets/bg_icon_full_moon.png",
	"assets/bg_icon_last_quarter.png",
};

/**
 * Rounds up to the next power of two.
 * @param target Target number to be rounded.
 * @returns The next power of two.
 */
static size_t round_power(size_t target)
{
	target--;
	target |= target >> 1;
	target |= target >> 2;
	target |= target >> 4;
	target |= target >> 8;
	target |= target >> 16;
	target++;
	return (target);
}

/**
 * @brief Initializes the append buffer.
 *
 * @param ab Append buffer pointer to be initialized.
 *
 * @return Returns 0 if success, -1 otherwise.
 */
static int abuf_alloc(struct abuf *ab)
{
	if (!ab)
		return (-1);

	ab->capacity = BUF_CAPACITY;
	ab->len = 0;
	ab->str = calloc(1, BUF_CAPACITY);
	if (!ab->str)
		return (-1);

	return (0);
}

/**
 * @brief Free the append buffer.
 *
 * @param ab Append buffer pointer.
 */
static void abuf_free(struct abuf *ab)
{
	if (!ab)
		return;

	free(ab->str);
	ab->capacity = 0;
	ab->len = 0;
}

/**
 * @brief Adds a new buffer @p buf of length @p len into
 * the append buffer @p ab.
 *
 * @param ab  Append buffer.
 * @param buf Buffer to be add.
 * @param len Buffer length.
 *
 * @return Returns 0 if success, -1 otherwise.
 */
static int abuf_append(struct abuf *ab, const char *buf, size_t len)
{
	size_t size;
	char *ptr;

	if (!ab)
		return (-1);

	if (!ab->str && abuf_alloc(ab) < 0)
		return (-1);

	size = ab->capacity;

	/*
	 * Check if there is enough space to fit the new
	 * buf + NULL terminator, if not, increase it.
	 *
	 * Although this is a generic append buffer, its
	 * always interesting to keep space for a NUL-
	 * terminator, just in case it is used for
	 * strings.
	 */
	if (len >= (ab->capacity - ab->len - 1))
	{
		size = round_power(ab->len + len + 1);
		ptr  = realloc(ab->str, size);
		if (!ptr)
			return (-1);

		ab->str = ptr;
	}

	memcpy(ab->str+ab->len, buf, len);
	ab->len         += len;
	ab->capacity     = size;
	ab->str[ab->len] = '\0';
	return (0);
}

/**
 * @brief Check if a given weather condition
 * is valid or not.
 *
 * @param condition Weather condition string.
 *
 * @return Returns 1 if valid, 0 otherwise.
 */
static int is_condition_valid(const char *condition)
{
	int ok = 0;

	if (!strcmp(condition, "clear"))
		ok = 1;
	else if (!strcmp(condition, "fog"))
		ok = 1;
	else if (!strcmp(condition, "clouds"))
		ok = 1;
	else if (!strcmp(condition, "showers"))
		ok = 1;
	else if (!strcmp(condition, "rainfall"))
		ok = 1;
	else if (!strcmp(condition, "thunder"))
		ok = 1;
	else if (!strcmp(condition, "snow"))
		ok = 1;

	if (!ok)
		info("Condition '%s' is invalid, acceptable values are:\n"
			 "  clear, fog, clouds, showers, rainfall, thunder, snow\n",
			 condition);

	return (ok);
}

/**
 * @brief Given a cSON object pointed by @p root, read
 * its @p item as a number and saves into @p dest.
 *
 * @param root JSON root node.
 * @param item Item name to be read.
 * @param dest Destination integer pointer.
 *
 * @return Returns 0 if success, -1 otherwise.
 */
static int json_get_number(
	const cJSON *root, const char *item, int *dest)
{
	cJSON *number;
	number = cJSON_GetObjectItemCaseSensitive(root, item);
	if (!cJSON_IsNumber(number))
		errto(out0, "'%s' value not found and/or is invalid!\n", item);
	*dest = number->valueint;
	return (0);
out0:
	return (-1);
}

/**
 * @brief Given a cSON object pointed by @p root, read
 * its @p item as a string and saves into @p dest.
 *
 * @param root JSON root node.
 * @param item Item name to be read.
 * @param dest Destination string pointer.
 *
 * @return Returns 0 if success, -1 otherwise.
 */
static int json_get_string(
	const cJSON *root, const char *item, char **dest)
{
	cJSON *str;
	str = cJSON_GetObjectItemCaseSensitive(root, item);
	if (!cJSON_IsString(str) || !str->valuestring)
		errto(out0, "'%s' value not found and/or is invalid!\n", item);
	*dest = strdup(str->valuestring);
	if (!*dest)
		panic("Unable to allocate string\n");
	return (0);
out0:
	return (-1);
}

/**
 * @brief Parses the received json in @p json_str into the
 * structure weather_info pointed by @p wi.
 *
 * This json (and structure) contains all elements to
 * show into the screen.
 *
 * @return Returns 0 if the parsing was succeeded, -1 if not.
 */
static int json_parse_weather(const char *json_str,
	struct weather_info *wi)
{
	int i;
	cJSON *day;
	cJSON *weather;
	cJSON *forecast;
	const char *error_ptr;

	if (!(weather = cJSON_Parse(json_str))) {
		if ((error_ptr = cJSON_GetErrorPtr()))
			errto(out0, "Error: %s\n", error_ptr);
		else
			errto(out0, "Error while parsing json!\n");
	}

	if (json_get_number(weather, "temperature", &wi->temperature) < 0)
		goto out0;
	if (json_get_number(weather, "max_temp",  &wi->max_temp)  < 0)
		goto out0;
	if (json_get_number(weather, "min_temp",  &wi->min_temp)  < 0)
		goto out0;
	if (json_get_string(weather, "condition", &wi->condition) < 0)
		goto out0;
	if (json_get_string(weather, "provider",  &wi->provider)  < 0)
		goto out0;
	if (json_get_string(weather, "location",  &wi->location)  < 0)
		goto out0;

	forecast = cJSON_GetObjectItemCaseSensitive(weather, "forecast");
	if (!forecast || !cJSON_IsArray(forecast))
		errto(out0, "'forecast' array not found!\n");

	i = 0;
	cJSON_ArrayForEach(day, forecast) {
		if (i >= 3)
			break;
		if (json_get_number(day, "max_temp",  &wi->forecast[i].max_temp)  < 0)
			goto out0;
		if (json_get_number(day, "min_temp",  &wi->forecast[i].min_temp)  < 0)
			goto out0;
		if (json_get_string(day, "condition", &wi->forecast[i].condition) < 0)
			goto out0;
		i++;
	}

	/* Check if all fields were filled. */
	if (i != 3)
		errto(out0, "'forecast' array have missing items (%d/3)!\n", i);

	/* Validate all weather conditions. */
	if (!is_condition_valid(wi->condition))
		goto out0;

	for (i = 0; i < 3; i++)
		if (!is_condition_valid(wi->forecast[i].condition))
			goto out0;

	cJSON_Delete(weather);
	return (0);
out0:
	weather_free(wi);
	cJSON_Delete(weather);
	return (-1);
}

/**
 * @brief Deallocates the current data saved into the
 * weather_info structure.
 *
 * @param wi Weather info structure.
 */
void weather_free(struct weather_info *wi)
{
	if (!wi)
		return;
	free(wi->location);
	free(wi->provider);
	free(wi->condition);
	for (int i = 0; i < 3; i++)
		free(wi->forecast[i].condition);
}

/**
 * @brief Issues the command provided by the user, reads its
 * stdout and parses its json.
 *
 * @param command Command to be issued.
 * @param wi      Weather info structure to be filled.
 *
 * @return Returns 0 if success, -1 otherwise.
 */
int weather_get(const char *command, struct weather_info *wi)
{
	FILE *f;
	struct abuf ab;
	char tmp[256] = {0};

	if (abuf_alloc(&ab) < 0)
		return (-1);

	f = popen(command, "r");
	if (!f)
		return (-1);

	while (fgets(tmp, sizeof(tmp), f))
		abuf_append(&ab, tmp, strlen(tmp));

	weather_free(wi);
	if (json_parse_weather(ab.str, wi) < 0)
		return (-1);

	abuf_free(&ab);
	pclose(f);
	return (0);
}

/**
 * @brief Checks if the current hour is day or not.
 *
 * @return Returns 1 if day, 0 if night.
 */
int weather_is_day(void)
{
	time_t now;
	struct tm *now_tm;
	now    = time(NULL);
	now_tm = localtime(&now);
	return (now_tm->tm_hour >= 06 && now_tm->tm_hour <= 17);
}

/**
 * @brief Returns the days number for the next
 * three days.
 *
 * @param d1 Next day 1.
 * @param d2 Next day 2.
 * @param d3 Next day 3.
 */
void weather_get_forecast_days(int *d1, int *d2, int *d3)
{
	time_t now;
	struct tm *now_tm;
	now    = time(NULL);
	now_tm = localtime(&now);
	*d1    = (now_tm->tm_wday + 1) % 7;
	*d2    = (now_tm->tm_wday + 2) % 7;
	*d3    = (now_tm->tm_wday + 3) % 7;
}

/**
 * @brief Roughly calculates the current moon phase
 * and returns an string pointing the path of the
 * corresponding moon asset.
 *
 * Note: Algorithm based on this one:
 *   https://minkukel.com/en/various/calculating-moon-phase/
 * but adapted to my needs, i.e., using only for 4
 * main phases, instead of the 8.
 *
 * @return Returns an string pointing the current
 * moon phase icon.
 */
const char *weather_get_moon_phase_icon(void)
{
	time_t now, first_new_moon;
	time_t diffsecs;
	int currentphase;
	double lunarsecs;
	double totalsecs;
	double currentsecs;
	double currentfrac;

	now = time(NULL);

	/*
	 * Epoch of the first new moon on 2000s:
	 * 2000-01-06 18:14 UTC.
	 */
	first_new_moon = 947182440;

	/* Calculate moon phase. */
	lunarsecs    = LUNAR_CYCLE_CONSTANT * 86400.0;
	totalsecs    = difftime(now, first_new_moon);
	currentsecs  = fmod(totalsecs, lunarsecs);
	currentfrac  = currentsecs / lunarsecs;
	currentphase = (int)round(currentfrac * 4) % 4;
	return (moon_phases[currentphase]);
}
