# Assistive Voices

This repository contains the code, design files, and documentation for our Electronic Communication Board, a tool designed to assist students who are cognitively capable of making decisions but lack the fine motor skills for speech. The device provides visual and auditory prompts to help students express their needs and communicate effectively.

The default captions are in Spanish, as is our web interface to facilitate the uploading of images and audio files. 

# Setup 
Our project was designed to be compatible with the Arduino Mega 2560 Rev3. Changing the model of the board may necessitate updating the macros in `assistive-voices/Config.h`.

A detailed outline of our project features and purpose are available in the teachers' guide, accessible in `assistive-voices/instructions/teacher-guide.pdf`. This file is in Spanish. 

Likewise, our hardware assembly guide is accessible via `assistive-voices/instructions/assembly-guide.pdf`. This file includes all of the schematics, technical specifications, and instructions for reproducing our project. All software, default images, captions, and audio files are available in this repository. 

To assemble our project, you will need the following components: 
- Arduino Mega Rev3 
- 480 x 320 TFT LCD screen 
- SD card
- 8 Ω speaker
- LM386 audio amplifier
- MT3608 boost converter 

And you will need to have the following Arduino libraries installed: 
- Adafruit GFX
- MCUFRIEND_kbv
- SD
- TMRpcm
- Fonts/FreeSans12pt7b

Additional details are available in our aforementioned guides. 

# License 
This project is licensed under the MIT License.

# Acknowledgements 
This project was developed in collaboration with the Adaptive Design Center (CDA) and the MIT D-Lab: Development class (EC.701). Special thanks to all of our collaborators for their input and assistance.

