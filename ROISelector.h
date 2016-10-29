#pragma once

#include <vector>
#include <iostream>

#include "XMLParser.h"
#include "Line.h"

class ROISelector {
	private:
		const std::string TEXT_STYLE = "font-family:Arial;font-size:9pt;font-style:normal;font-weight:normal;fill:#FF0000";
		const std::string RED = "#FF0000";
		const std::string STROKE_WIDTH = "1";

		std::string filename;
		FILE* openFile();
		std::vector<XMLTag*> parsedRootScope;
		std::vector<Line*> parsedLines;

	public:
		ROISelector(std::string);
		int startParser();
		int findTags();
		std::vector<XMLTag*> getParsedRootScope();
		std::vector<Line*> getParsedLines();
};