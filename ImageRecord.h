#pragma once

#include "opencv2/imgproc/imgproc.hpp"

class ImageRecord {
	private:
		double x;
		double y;
		cv::Mat image;
		std::string filename;
	public:
		ImageRecord(double x, double y, cv::Mat image, std::string filename);
		double getX();
		double getY();
		cv::Mat getImage();
		std::string getFilename();
};