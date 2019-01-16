
ffmpeg
----------------------------
-lavutil    \
-lavformat  \
-lavcodec   \
-lswresample \
-lswscale   \
----------------------------

----- SDL ------------------
-lSDL           \
-lSDL_image     \
----------------------------


c++ -Wall main.cpp -o main -lavutil -lavformat -lavcodec -lswresample -lswscale -lSDL -lSDL_image 
