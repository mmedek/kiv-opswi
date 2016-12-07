#pragma once

#include <vector>
#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"

#include "XMLParser.h"
#include "Line.h"
#include "ImageRecord.h"

class ROISelector {
	private:
		const std::string TEXT_STYLE = "font-family:Arial;font-size:9pt;font-style:normal;font-weight:normal;fill:#FF0000";
		const std::string RED = "#FF0000";
		const std::string STROKE_WIDTH = "1";
		const unsigned int MAX_DEVIATION = 10;
		// width is 164 - this is half width

		const int ROI_WIDTH = 86;
		// height is 164 - this is half height
		const int ROI_HEIGHT = 86;
		// prefix of filepatg segmented image
		const std::string SEGMENTED_IMAGE_PREFIX_PATH = "../segmented_images/";

		std::string filename;
		std::string filenameSVG;
		std::string filenameJPG;
		std::vector<std::vector<ImageRecord*>> surfGroups;
		cv::Mat equalizedImage;
		cv::Mat segment;
		std::string segmentedImageFilename;
		FILE* openFile();
		FILE* openFile(std::string);
		std::vector<XMLTag*> parsedRootScope;
		std::vector<Line*> parsedLines;
		void addImageToGroup(double middleX, double middleY);
		void ROISelector::init();

	public:
		ROISelector();
		void set_new_image(std::string);
		void printGroup();
		int runParser();
		int findTags();
		int preprocess();
		int cutROIs();
		int ROISelector::writeImage();
		std::vector<XMLTag*> getParsedRootScope();
		std::vector<Line*> getParsedLines();
};