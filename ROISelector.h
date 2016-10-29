#pragma once

#include <vector>
#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"

#include "XMLParser.h"
#include "Line.h"

class ROISelector {
	private:
		const std::string TEXT_STYLE = "font-family:Arial;font-size:9pt;font-style:normal;font-weight:normal;fill:#FF0000";
		const std::string RED = "#FF0000";
		const std::string STROKE_WIDTH = "1";

		std::string filenameSVG;
		std::string filenameJPG;
		cv::Mat equalizedImage;
		FILE* openFile();
		FILE* openFile(std::string);
		std::vector<XMLTag*> parsedRootScope;
		std::vector<Line*> parsedLines;

	public:
		ROISelector(std::string);
		int runParser();
		int findTags();
		int runEqualization();
		std::vector<XMLTag*> getParsedRootScope();
		std::vector<Line*> getParsedLines();
};