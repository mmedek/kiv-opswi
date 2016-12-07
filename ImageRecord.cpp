#include "ImageRecord.h"

ImageRecord::ImageRecord(double x, double y, cv::Mat image, std::string filename) {
	this->x = x;
	this->y = y;
	this->image = image;
	this->filename = filename;
}

double ImageRecord::getX() {
	return this->x;
}

double ImageRecord::getY() {
	return this->y;
}

cv::Mat ImageRecord::getImage() {
	return this->image;
}

std::string ImageRecord::getFilename() {
	return this->filename;
}
