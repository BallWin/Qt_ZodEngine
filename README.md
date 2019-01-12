# Qt_ZodEngine
Old game Z, Z95, Z-Expansion Kit - new engine

This is an ZodEngine revision.
The original is located here:
http://zod.sourceforge.net  http://www.nighsoft.com

" Welcome to the Zod Engine project. The Zod Engine is an open source remake of the 1996 game Z by the Bitmap Brothers written in C++ using the SDL library for Linux / Windows / Etc."

A forum for the discussion here http://zzone.lewe.com. 
A branch for open source here http://zzone.lewe.com/forum/viewforum.php?f=5

This option is divided into dynamic libraries. Project files for IDE QtCreator (qmake build system).
Building with Ubuntu LTS 18.04.1.

Before running the binaries, remember to add the path to the libraries to the environment variable:
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/your/custom/path_to_folder_bin_ZodEngine/lib


=== for linux systems ======
" Installing the required libraries -
The required libraries for this game are as follows...
* SDL
* SDL_ttf
* SDL_mixer
* SDL_image
* libmysqlclient (or sometimes called libmysql)
* wx (sometimes called libwx, or libwxgtk)
Notes: It is possible to compile the game without mysql support. 
Also wx is only needed for the zod_launcher.

Installing the required libraries on Ubuntu:
sudo apt-get install libsdl-dev libsdl-ttf2.0-dev libsdl-mixer1.2-dev 
libsdl-image1.2-dev libmysqlclient-dev libwxgtk2.8-dev "



