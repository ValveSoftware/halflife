DEVTOOLS
========
This folder contains two Python scripts to create resources for the Half-Life 1 engine. 
Both may be run in a command-line. Both scripts require the pillow & image modules
which can be installed by running:
		pip install pillow
		pip install image

IMAGE TO BACKGROUND
-------------------
Takes a source image (PSD/PNG/TGA), and partitions & converts the image to Targa (TGA)
format. The end result creates an 800x600 menu background for Half-Life 1 engine-based
games, which is to be placed in resource/background.
		Usage: python image_to_background.py path_to_image (psd/png/tga)
		

IMAGE TO SPRITE
---------------
Takes a source image (PSD/PNG/) and converts it to the Half-Life 1 engine's sprite (SPR)
format.
		Usage: python image_to_spr.py path_to_image (psd/png)