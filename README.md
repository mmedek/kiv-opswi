# kiv-opswi

There are two branches:
a) master for python 3.5,
b) branch python-27 for python 2.7

Project contains 3 main programs:
1) roi_selector - application written in c++ for the selection ROI according to errors highlighted in svg images,
2) cnns - one cnn for every highlighted error is computed and saved,
3) according to trained and saved cnns we classified new images and if we find out error we saved the image with highlighted error
