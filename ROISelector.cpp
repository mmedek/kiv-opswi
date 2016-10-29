#include <stdlib.h>

#include "opencv2/highgui/highgui.hpp"

#include "ROISelector.h"

FILE* ROISelector::openFile() {
	return openFile(this->filenameSVG);
}

ROISelector::ROISelector(std::string filenameSVG) {
	// e.g. ABoard_TX-55AS650B_%NH-4540419(35724).svg
	this->filenameSVG = filenameSVG;
	// e.g. ABoard_TX-55AS650B_%NH-4540419(35724).jpg
	std::string extensionJPG = "jpg";
	this->filenameJPG = (filenameSVG.substr(0, filenameSVG.find_last_of(".") + 1)).append(extensionJPG);
}

FILE* ROISelector::openFile(std::string filename) {

	FILE* file = fopen(filename.c_str(), "r");
	if (!file)
	{
		std::cerr << "Opening of file '" << filename.c_str() << "' failed" << std::endl;
		return nullptr;
	}

	return file;
}

int ROISelector::runEqualization() {

	cv::Mat image = cv::imread(this->filenameJPG.c_str(), cv::IMREAD_GRAYSCALE);

	if (image.empty()) {
		std::cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	cv::equalizeHist(image, this->equalizedImage);

//	cv::imshow("Display window", equalizedImage);
//	cv::waitKey(0);

	return 1;
}

int ROISelector::runParser() {
	FILE* file;
	if ((file = openFile()) == nullptr) {
		return -1;
	}

	int c;
	while (!feof(file))
	{
		c = fgetc(file);
		if (c == '<')
		{
			XMLTag* tag = XMLTag::parseBody(file);
			if (tag)
				parsedRootScope.push_back(tag);
		}
	}

	return 1;
}

int ROISelector::findTags() {
	try {
		// check if is format of xml file svg
		if (parsedRootScope.at(0)->tagName.find("svg") == std::string::npos) {
			std::cout << "Parsing XML failed. Invalid format of svg image!" << std::endl;
			return 2;
		}

		// 1st line body of XML is with image informations [0]
		// 2nd - 5th - informations about red borders around image [1 - 4]
		for (int i = 5; i < parsedRootScope.at(0)->children.size(); i++) { //start [5th line]

			XMLTag* rootScopeTag = parsedRootScope.at(0);
			//std::cout << rootScopeTag->children.at(i)->tagName.c_str() << std::endl;

			if ((rootScopeTag->children.at(i)->tagName.find("text") != std::string::npos)
				&& (rootScopeTag->children.at(i)->attributes.at("style").find(TEXT_STYLE) != std::string::npos)
				// line stroke-width="1" stroke="#FF0000" 
				&& ((rootScopeTag->children.at(i + 1)->tagName.find("line") != std::string::npos) && (rootScopeTag->children.at(i)->tagName.find("polyline") == std::string::npos))
				&& (rootScopeTag->children.at(i + 1)->attributes.at("stroke").find(RED) != std::string::npos)
				&& (rootScopeTag->children.at(i + 1)->attributes.at("stroke-width").find(STROKE_WIDTH) != std::string::npos)
				// polyline fill = "#FF0000" stroke = "#FF0000"
				&& (rootScopeTag->children.at(i + 2)->tagName.find("polyline") != std::string::npos)
				&& (rootScopeTag->children.at(i + 2)->attributes.at("fill").find(RED) != std::string::npos)
				&& (rootScopeTag->children.at(i + 2)->attributes.at("stroke").find(RED) != std::string::npos)
				// polyline fill = "#FF0000" stroke = "#FF0000"
				&& (rootScopeTag->children.at(i + 3)->tagName.find("polyline") != std::string::npos)
				&& (rootScopeTag->children.at(i + 3)->attributes.at("fill").find(RED) != std::string::npos)
				&& (rootScopeTag->children.at(i + 3)->attributes.at("stroke").find(RED) != std::string::npos))
			{
				parsedLines.push_back(new Line(atof(rootScopeTag->children.at(i + 1)->attributes.at("x1").c_str()), atof(rootScopeTag->children.at(i + 1)->attributes.at("x2").c_str()),
					atof(rootScopeTag->children.at(i + 1)->attributes.at("y1").c_str()), atof(rootScopeTag->children.at(i + 1)->attributes.at("y2").c_str())));
				std::cout << "LINE, x1 = " << rootScopeTag->children.at(i + 1)->attributes.at("x1").c_str() << ", x2 = " << rootScopeTag->children.at(i + 1)->attributes.at("x2").c_str() << ", y1 = " << rootScopeTag->children.at(i + 1)->attributes.at("y1").c_str() << ", y2 = " << rootScopeTag->children.at(i + 1)->attributes.at("y2").c_str() << std::endl;
			}
			/*
			cross:
			<g>
			line stroke-width="1" stroke="#FF0000 followed by
			polyline fill="#FF0000" stroke="#FF0000" followed by
			line stroke-width="1" stroke="#FF0000 followed by
			polyline fill="#FF0000" stroke="#FF0000"
			</g>
			*/
			else if (rootScopeTag->children.at(i)->tagName.find("g") != std::string::npos)
			{
				std::cout << "CROSS" << std::endl;
			}
			// line stroke - width = "1" stroke = "#FF0000
			else if (((rootScopeTag->children.at(i)->tagName.find("line") != std::string::npos) && (parsedRootScope.at(0)->children.at(i)->tagName.find("polyline") == std::string::npos))
				&& (rootScopeTag->children.at(i)->attributes.at("stroke-width").find(STROKE_WIDTH) != std::string::npos)
				&& (rootScopeTag->children.at(i)->attributes.at("stroke").find(RED) != std::string::npos)
				// line stroke - width = "1" stroke = "#FF0000
				&& ((rootScopeTag->children.at(i + 1)->tagName.find("line") != std::string::npos) && (parsedRootScope.at(0)->children.at(i + 1)->tagName.find("polyline") == std::string::npos))
				&& (rootScopeTag->children.at(i + 1)->attributes.at("stroke").find(RED) != std::string::npos)
				&& (rootScopeTag->children.at(i + 1)->attributes.at("stroke-width").find(STROKE_WIDTH) != std::string::npos)
				// line stroke - width = "1" stroke = "#FF0000
				&& ((rootScopeTag->children.at(i + 2)->tagName.find("line") != std::string::npos) && (parsedRootScope.at(0)->children.at(i + 2)->tagName.find("polyline") == std::string::npos))
				&& (rootScopeTag->children.at(i + 2)->attributes.at("stroke").find(RED) != std::string::npos)
				&& (rootScopeTag->children.at(i + 2)->attributes.at("stroke-width").find(STROKE_WIDTH) != std::string::npos)
				// line stroke - width = "1" stroke = "#FF0000
				&& ((rootScopeTag->children.at(i + 3)->tagName.find("line") != std::string::npos) && (parsedRootScope.at(0)->children.at(i + 3)->tagName.find("polyline") == std::string::npos))
				&& (rootScopeTag->children.at(i + 3)->attributes.at("stroke").find(RED) != std::string::npos)
				&& (rootScopeTag->children.at(i + 3)->attributes.at("stroke-width").find(STROKE_WIDTH) != std::string::npos)
				// circle stroke-width="1" stroke="#FF0000 fill="none"
				&& (rootScopeTag->children.at(i + 4)->tagName.find("circle") != std::string::npos)
				&& (rootScopeTag->children.at(i + 4)->attributes.at("stroke").find(RED) != std::string::npos)
				&& (rootScopeTag->children.at(i + 4)->attributes.at("fill").find("none") != std::string::npos))
			{
				std::cout << "CIRCLE" << std::endl;
			}
		}
	}
	catch (std::exception e)
	{
		std::cout << "Error during parsing XML file. With error message: " << e.what() << std::endl;
		return 2;
	}

	return 1;
}

std::vector<XMLTag*> ROISelector::getParsedRootScope() {
	return this->parsedRootScope;
}

std::vector<Line*> ROISelector::getParsedLines() {
	return this->parsedLines;
}