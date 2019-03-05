
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
Decoder for video files with extension .JV 
For Bitmap Brothers JV video cuts from the game Z (Commander Zod).
http://zzone.lewe.com/index.php?page=game
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

C++ language.

headers:
- general_head.h
- jv_format.h	// interfaces for working with JVC files 
- bitstream.h   // interface bit stream
- to_bmp.h      // working with BMP format
- video_proc.h  // decoder for video
- audio_proc.h  // to work with audio data

The folder bin/Cuts contains two  of the movie to use in the tests for example.
- example_decode_jv.cpp   //An example of decoding data of the file JV.
Example presents:
	Saving frames as BMP. 
	Save sound as WAV. 
	Decoding and output to the screen using SDL 1.2.



folders:
-C++			- only this files
-QtCreator proj 	- project for qmake (ide QtCreator)


Note:
Audio and video handlers are designed to work in streams. They have the appropriate 
support files. However, JV files are very small. It makes no sense to use separate 
streams to decode and load data. It is better to process the whole file. To do this, 
 can use the methods "total". See example code.
--------------------------------------------------
					BallWin
					
Help
//======================================================
Command line arguments:
 --help  This help
 --file  Path to an existing video file JV
 --fps   Frame-per-second what you want to get (5..100)
--save  Path to an existing folder for saving frames
 --wav   To save the sound in WAV file
--show  Plays the video in the window (true) or no (false)

	 Example comand line params for app:
./this_app --file=/home/my_Cuts/video.JV --fps=25 --save=/home/my_frames --wav=my.wav

 *** Notes ****************************"
 If there are no options --save or --wav, the corresponding files will not be saved.
 If the --file parameter is missing, the default path will be used.
 If there is --show=false, the parameters --save or --wav must be set
 If the --show parameter is missing, use by default --show=true
 If the --fps parameter is missing, the value will be calculated by the frameRate in file.
 Regarding FPS-the real value depends on your hardware.
 If the command line is empty, the default settings will be used:
--file	path_to_file
--fps	calculated by the frameRate in file
--save	empty
--wav	empty
---------------------------------------
	 Control keys, press with playback:
 Esc or \"q\" - quit
 Space        - save shapshot
					
					

Compile commands for example (copy-paste-run commands to terminal)
- you do not need to enter comments %)
==================================================
// -- linced (make objects files .o)
c++ -Wall -c jv_format.cpp
c++ -Wall -c bitstream.cpp
c++ -Wall -c video_proc.cpp
c++ -Wall -c audio_proc.cpp
c++ -Wall -c example_decode_jv.cpp
// -- show files
ls
// -- build app
c++ -o ExampleDecode example_decode_jv.o jv_format.o bitstream.o video_proc.o audio_proc.o  -lSDL -lSDL_image -lSDL_mixer

