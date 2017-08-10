# kiv-opswi

There are two branches:

a) master for python 3.5 - run on Windows - development in this branch was stopped
b) branch python-27 for python 2.7 - run on Linux

Project contains 3 main programs (in every programme folder you can find README):
1) roi_selector - application written in c++ for the selection ROI according to errors highlighted in svg images,
2) cnns - one cnn for every highlighted error is computed and saved,
3) recognizer - according to trained and saved cnns we classified new images and if we find out error we saved the image with highlighted error
