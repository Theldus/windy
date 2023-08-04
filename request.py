#!/usr/bin/env python

# MIT License
#
# Copyright (c) 2023 Davidson Francis <davidsondfgl@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import requests
import json

LATITUDE="<change to your location>"
LONGITUDE="<change to your location>"
LOCATION="<City, State>"

#
# OpenMeteo provider, more info at:
#   https://open-meteo.com/en/docs
#
PROVIDER="OpenMeteo"

# Function to convert weather code to a human-readable condition
def get_weather_condition(code):
	conditions = {
		0: "clear",		# Clear sky
		1: "clear",		# Mainly clear
		2: "clouds",	# Partly cloudy
		3: "clouds",	# Overcast
		45: "fog",		# Fog
		48: "fog",		# Depositing rime fog
		51: "showers",	# Drizzle: Light intensity
		53: "showers",	# Drizzle: Moderate intensity
		55: "showers",	# Drizzle: Dense intensity
		56: "showers",	# Freezing Drizzle: Light intensity
		57: "showers",	# Freezing Drizzle: Dense intensity
		61: "rainfall", # Rain: Slight intensity
		63: "rainfall", # Rain: Moderate intensity
		65: "rainfall", # Rain: Heavy intensity
		66: "rainfall", # Freezing Rain: Light intensity
		67: "rainfall", # Freezing Rain: Heavy intensity
		71: "snow",		# Snow fall: Slight intensity
		73: "snow",		# Snow fall: Moderate intensity
		75: "snow",		# Snow fall: Heavy intensity
		77: "snow",		# Snow grains
		80: "rainfall", # Rain showers: Slight intensity
		81: "rainfall", # Rain showers: Moderate intensity
		82: "rainfall", # Rain showers: Violent intensity
		85: "snow",		# Snow showers: Slight intensity
		86: "snow",		# Snow showers: Heavy intensity
		95: "thunder",	# Thunderstorm: Slight or moderate
		96: "thunder",	# Thunderstorm: With slight hail
		99: "thunder"	# Thunderstorm: With heavy hail
	}
	return conditions.get(code, "unknown")

# Function to format forecast data
def format_forecast(forecast_data):
	forecast = []
	# Skip the first element (current day) in the forecast array
	for i in range(1, 4):
		data = {
			"max_temp": forecast_data["temperature_2m_max"][i],
			"min_temp": forecast_data["temperature_2m_min"][i],
			"condition": get_weather_condition(forecast_data["weathercode"][i]),
		}
		forecast.append(data)
	return forecast

# HTTPS request to get JSON data
response = requests.get("https://api.open-meteo.com/v1/forecast?"
	+ "latitude="	+ LATITUDE
	+ "&longitude=" + LONGITUDE
	+ "&current_weather=true"
	+ "&daily=weathercode,temperature_2m_max,temperature_2m_min&timezone=auto&forecast_days=4")

data = response.json()

# Process data and create the output JSON
output = {
	"temperature": data["current_weather"]["temperature"],
	"condition": get_weather_condition(data["current_weather"]["weathercode"]),
	"max_temp": data["daily"]["temperature_2m_max"][0],
	"min_temp": data["daily"]["temperature_2m_min"][0],
	"location": LOCATION,
	"provider": PROVIDER,
	"forecast": format_forecast(data["daily"]),
}

# Encode the output JSON with proper formatting
output_json = json.dumps(output, indent=4)

# Print the output JSON to stdout
print(output_json)
