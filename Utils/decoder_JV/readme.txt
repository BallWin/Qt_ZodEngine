
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

