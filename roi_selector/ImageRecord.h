#pragma once

#include "opencv2/imgproc/imgproc.hpp"

class ImageRecord {
	private:
		double x;
		double y;
		cv::Mat image;
		std::string filename;
		std::string filename_orig;
	public:
		ImageRecord(double x, double y, cv::Mat image, std::string filename, std::string orig_filename);
		double getX();
		double getY();
		cv::Mat getImage();
		std::string getFilename();
		std::string getOrigFilename();
};