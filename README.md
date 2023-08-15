# 🍃 windy
An SDL3 widget that attempts to looks like the good ol' Windows Vista/7 weather
widget

## Introduction
Even though I'm a Linux user 99% of the time, one of the few things I miss is
the weather widget from Windows Vista/7: it's simple, but I genuinely find it
attractive and useful.

While there are various weather widgets for Linux using Conky, I've never come
across one that resembled the Windows 7 version. So, I decided to create my own
with C and SDL3. Windy is open source and works on Linux, FreeBSD, MacOS, and
Windows.

Windy:
<p align="center">
<img align="center" src="https://github.com/Theldus/windy/assets/8294550/8b63edf6-2c21-4053-b8f0-ecd99e52332d" alt="Windy examples">
<br>
Windy examples
</p>
</a>

## How to use
### Quick Explanation:

Change the following lines of `request.py` file with the lat/long of your city
(exact location not needed!):
```python
#
# Example:
# LATITUDE="35.69"
# LONGITUDE="139.69"
# LOCATION="Tokyo, Japan"
#
LATITUDE="<change to your location>"
LONGITUDE="<change to your location>"
LOCATION="<City, State>"
```

and then run:
```c
$ ./windy -c "python request.py"
```

### Detailed Explanation:

Unlike most weather programs/widgets, Windy doesn't attempt to embed one or more
forecast APIs within itself. Instead, it delegates this responsibility to an
external program/script.

Windy accomplishes this by invoking an external script provided as a parameter by
the user. This script returns JSON output through stdout, which Windy reads and
displays on the screen. Essentially, Windy serves as a GUI for an external
program.

By default, Windy provides the `request.py` script, which uses the
[OpenMeteo's API](https://open-meteo.com/en/docs) to fetch weather information.
However, users can supply any script, program, etc., in their preferred language,
as long as it follows the JSON format below:
```json
{
    "temperature": 22,
    "condition": "clear",
    "max_temp": 20,
    "min_temp": 15,
    "location": "Tokyo, Japan",
    "provider": "OpenMeteo",
    "forecast": [
        {
            "max_temp": 34,
            "min_temp": 27,
            "condition": "rainfall"
        },
        {
            "max_temp": 34,
            "min_temp": 27,
            "condition": "clouds"
        },
        {
            "max_temp": 34,
            "min_temp": 27,
            "condition": "clouds"
        }
    ]
}

```
acceptable values for `condition` are: `clear`, `fog`, `clouds`, `showers`, `rainfall`, `thunder`, and `snow`.

### Command-line arguments:

Windy also supports changing the weather update interval (`-t`) and the screen
coordinates (`-x` and `-y`) where it should appear. Below are all the available
options:
```text
$ ./windy -h
Usage: ./windy [options] -c <command-to-run>
Options:
  -t           Interval time (in seconds) to check for weather
               updates (default = 10 minutes)
  -c <command> Command to execute when the update time reaches
  -x <pos>     Set the window X coordinate
  -y <pos>     Set the window Y coordinate
  -h           This help

Example:
 Update the weather info each 30 minutes, by running the command
 'python request.py'
    $ ./windy -t 1800 -c "python request.py"

Obs: Options -t,-x and -y are not required, -c is required!
```
