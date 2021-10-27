cmake_minimum_required(VERSION 3.11)
project(FontDebug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GLIBMM REQUIRED IMPORTED_TARGET glibmm-2.4)
pkg_check_modules(GTKMM3 REQUIRED IMPORTED_TARGET gtkmm-3.0)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

find_package(Freetype REQUIRED)
find_package(ICU COMPONENTS uc REQUIRED)

# TODO: This approach needs external xxd binary, not a portable way
add_custom_command(
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon_embed.cpp
	COMMAND xxd -i resources/app_icon.png ${CMAKE_CURRENT_BINARY_DIR}/icon_embed.cpp
)

set(FontDebugSrc
	src/drawer.cpp
	src/fontdebug.cpp
	src/properties.cpp
	${CMAKE_CURRENT_BINARY_DIR}/icon_embed.cpp
	)

add_executable(fontdebug ${FontDebugSrc})

target_link_libraries(fontdebug
	PkgConfig::GLIB
	PkgConfig::GLIBMM
	PkgConfig::GTKMM3
	PkgConfig::GTK3
	Freetype::Freetype
	ICU::uc
	)
