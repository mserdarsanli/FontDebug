#!/usr/bin/env python3

# Copyright 2021 Mustafa Serdar Sanli
#
# This file is part of FontDebug.
#
# FontDebug is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FontDebug is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FontDebug.  If not, see <https://www.gnu.org/licenses/>.


import subprocess

build_tmpl = '''
rule embed
    command = xxd -i $in $out

rule compile
    depfile = $out.d
    command = g++ -MD -MF $out.d -c -o $out -O3 $in $ccflags

rule link
    command = g++ -o $out $in $linkflags


build out/resources/icon_embed.cpp: embed resources/app_icon.png

build out/src/drawer.cpp.o: compile src/drawer.cpp
build out/src/fontdebug.cpp.o: compile src/fontdebug.cpp
build out/src/properties.cpp.o: compile src/properties.cpp
build out/resources/icon_embed.cpp.o: compile out/resources/icon_embed.cpp

build out/fontdebug: link out/src/drawer.cpp.o out/src/fontdebug.cpp.o out/src/properties.cpp.o out/resources/icon_embed.cpp.o
'''

def main():
    # TODO maybe use something like meson

    deps = ['gtkmm-3.0', 'gtk+-3.0', 'glibmm-2.4', 'glib-2.0', 'icu-uc', 'freetype2']
    ccflags   = str(subprocess.check_output(['pkg-config', '--cflags'] + deps), 'utf-8')
    linkflags = str(subprocess.check_output(['pkg-config', '--libs'] + deps), 'utf-8')

    build_rules = '\n'.join([
        f'ccflags = {ccflags}',
        f'linkflags = {linkflags}',
        build_tmpl
    ])

    open('build.ninja', 'w').write(build_rules)

if __name__ == '__main__':
    main()
