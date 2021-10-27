# FontDebug

A utility for exploring FreeType, example screenshot below:

![screenshot](doc/screenshot.png)

## Building

Install requirement dependencies (example for Debian/Ubuntu):

```shell
apt-get install cmake libglib2.0-dev libglibmm-2.4-dev libgtkmm-3.0-dev libgtk-3-dev libfreetype-dev libicu-dev  
```

Then run these commands:

```shell
cmake -B build
cmake --build build
```

Output will be at `build/fontdebug`.

## Copying

FontDebug is licensed under GNU General Public License Version 3, or any later version. See COPYING file for license text.
