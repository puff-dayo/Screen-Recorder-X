# Screen Recorder for X11 (SRX)

SRX is a lightweight screen recording tool built using the FLTK GUI and FFmpeg for Linux X11.

## Key features:

- Start/Stop with a button
- Always on top window option
- Lightweight and easy to use

The output files are saved in the ~/Videos/ directory, with filenames in the format `screen_recording_YYYY-MM-DD_HH-MM-SS.mp4`.

First install ffmpeg, and then run the program and click to record.

![Screenshot_20241213_003055](https://github.com/user-attachments/assets/58f6ed49-ed7f-4b7f-858d-15087fc95fab)

## Build and install:

```
# cmake version 3.25.1
# gcc version 12.2.0

sudo apt install ffmpeg
mkdir build && cd build
cmake ..
make
./screenrecorder
```
