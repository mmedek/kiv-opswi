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
		const unsigned int MAX_DEVIATION = 20;
		// width is 164 - this is half width

		const float PI = 3.1415927;

		const int ROI_WIDTH = 64;
		// height is 164 - this is half height
		const int ROI_HEIGHT = 64;
		// prefix of filepatg segmented image
                const std::string SEGMENTED_IMAGE_PREFIX_PATH = "data/segmented_images/";
                const std::string POSITIVE_ROIS_IMAGE_PREFIX_PATH = "data/positive_rois/";
                const std::string NEGATIVE_ROIS_IMAGE_PREFIX_PATH = "data/negative_rois/";


		std::string filename;
		std::string filenameSVG;
		std::string filenameJPG;
		std::vector<std::vector<ImageRecord*>> surfGroups;
		cv::Mat equalizedImage;
		cv::Mat segment;
		std::string segmentedImageFilename;
		FILE* openFile();
		FILE* openFile(std::string);
		float get_angle(cv::Point2f A, cv::Point2f B);
		std::vector<XMLTag*> parsedRootScope;
		std::vector<Line*> parsedLines;
		void addImageToGroup(double middleX, double middleY);
                void init();
                cv::Mat shearMat(cv::Mat img, float shear);
	public:
		ROISelector();
		void set_new_image(std::string);
		void writeGroups();
		int runParser();
		int findTags();
		int preprocess();
		void segmentate_positive_ROIs();
		int cutROIs();
		int writeImage();
		void processSURF();
		std::vector<XMLTag*> getParsedRootScope();
		std::vector<Line*> getParsedLines();
};
