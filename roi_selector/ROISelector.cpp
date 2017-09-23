#include <stdlib.h>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"

#include <sys/stat.h>
#include "ROISelector.h"
#include "api.h"

FILE* ROISelector::openFile() {
	return openFile(this->filenameSVG);
}

ROISelector::ROISelector() {}

void ROISelector::set_new_image(std::string filenameSVG) {
	init();
	// e.g. ../data/ABoard_TX-55AS650B_%NH-4540419(35724).svg
	this->filenameSVG = filenameSVG;
	// e.g. ABoard_TX-55AS650B_%NH-4540419(35724)
	std::string extensionJPG = "jpg";
	std::string file = filenameSVG.substr(filenameSVG.find_last_of("/") + 1);
	this->filename = file.substr(0, file.find_last_of("."));
	// e.g. ../data/ABoard_TX-55AS650B_%NH-4540419(35724).jpg
	this->filenameJPG = (filenameSVG.substr(0, filenameSVG.find_last_of(".") + 1)).append(extensionJPG);
}

void ROISelector::init() {
	this->filenameSVG = "";
	this->filename = "";
	this->filenameJPG = "";
	this->segmentedImageFilename = "";

	while (this->parsedRootScope.size() != 0)
		this->parsedRootScope.pop_back(); // we need to empty this before processing next image

	while(this->parsedLines.size() != 0)
		this->parsedLines.pop_back();
}

FILE* ROISelector::openFile(std::string filename) {

        FILE* tmp_file = fopen(filename.c_str(), "r");
        if (tmp_file == NULL)
	{
		std::cerr << "Opening of file '" << filename.c_str() << "' failed" << std::endl;
		return nullptr;
	}

        return tmp_file;
}

int ROISelector::preprocess() {

	cv::Mat image = cv::imread(this->filenameJPG.c_str(), cv::IMREAD_GRAYSCALE);

	if (image.empty()) {
		std::cout << "Could not open or find the image" << std::endl;
		exit(0);
	}

	cv::equalizeHist(image, this->equalizedImage);

	// sharpening
	cv::Mat sharpenedImg;
	GaussianBlur(this->equalizedImage, sharpenedImg, cv::Size(0, 0), 3);
	addWeighted(this->equalizedImage, 1.5, sharpenedImg, -0.5, 0, sharpenedImg);

	this->equalizedImage = sharpenedImg;

	return 1;
}

//[x1, y1]	+---------------+
//			+				+
//			+				+
//			+		X		+
//			+				+
//			+				+		
//			+---------------+	[x2, y2]
int ROISelector::cutROIs() {
	// this should be there because it is possible to run equalization before this or otherwise
	
	this->equalizedImage = cv::imread(this->filenameJPG.c_str(), cv::IMREAD_GRAYSCALE);

	if (this->equalizedImage.empty()) {
		std::cout << "Could not open or find the image" << std::endl;
		exit(0);
	}

	double middleX = 0; // middle X - for vertical or horizontal line is computation same
	double middleY = 0;	// middle X - for vertical or horizontal line is computation same
	double x1 = 0;
	double x2 = 0;
	double y1 = 0;
	double y2 = 0;
	std::string segmentFilename = SEGMENTED_IMAGE_PREFIX_PATH;

    mkdir(segmentFilename.c_str(), S_IRWXU);
	// add name of processed file
	// separate name of file and index of line with '_'
	segmentFilename.append(this->filename);
	segmentFilename.append("_");
        int indexx = 0;

	for (int i = 0; i < this->getParsedLines().size(); i++) {

		middleX = (this->getParsedLines()[i]->getX1() + this->getParsedLines()[i]->getX2()) / 2;
		middleY = (this->getParsedLines()[i]->getY1() + this->getParsedLines()[i]->getY2()) / 2;

		x1 = middleX - ROI_WIDTH;
		x2 = middleX + ROI_WIDTH;
		y1 = middleY - ROI_HEIGHT;
		y2 = middleY + ROI_HEIGHT;

		// border checks
		if (x2 > this->equalizedImage.size().width) {
			x2 = this->equalizedImage.size().width;
			x1 = x2 - 2 * ROI_WIDTH;
		}
		
		if (x1 < 0) {
			x1 = 0;
			x2 = 2 * ROI_WIDTH;
		}

		if (y2 > this->equalizedImage.size().height) {
			y2 = this->equalizedImage.size().height;
			y1 = y2 - 2 * ROI_HEIGHT;
		}

		if (y1 < 0) {
			y1 = 0;
			y2 = 2 * ROI_WIDTH;
		}
		
		this->segment = cv::Mat(equalizedImage, cv::Rect(x1, y1, 2 * ROI_WIDTH, 2 * ROI_HEIGHT));
		std::string temp = segmentFilename;
                this->segmentedImageFilename = (temp.append(std::to_string(indexx++))).append(".jpg");
		this->addImageToGroup(middleX, middleY);		
	}

	return 1;

}

void ROISelector::writeGroups() {
	
	for (unsigned int i = 0; i < this->surfGroups.size(); i++) {
		for (int j = 0; j < this->surfGroups.at(i).size(); j++) {
			std::string temp = SEGMENTED_IMAGE_PREFIX_PATH;
			temp += (char)('a' + i);
			temp.append("/"); // groups a-z
            mkdir(temp.c_str(), S_IRWXU);
			temp.append(this->surfGroups[i][j]->getFilename().substr(this->surfGroups[i][j]->getFilename().find_last_of("/\\") + 1));
			cv::imwrite(temp, this->surfGroups[i][j]->getImage());
		}
	}
}

void ROISelector::processSURF() {

	std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
	cv::Mat descriptors_object, descriptors_scene;
	cv::Mat img_scene;
	cv::Mat img_object;
	cv::Mat result;
    std::string temp = POSITIVE_ROIS_IMAGE_PREFIX_PATH;
    mkdir(temp.c_str(), S_IRWXU);


    for (int i = 0; i < this->surfGroups.size(); i++) {

		for (int j = 0; j < this->surfGroups.at(i).size(); j++) {

			if (j == 0) {
				img_object = this->surfGroups[i][0]->getImage();
                                // create new folder for saving ROIs
                                temp = POSITIVE_ROIS_IMAGE_PREFIX_PATH;
				temp += "roi_";
				temp += (char)('a' + i);
				temp.append("/"); // groups a-z
                mkdir(temp.c_str(), S_IRWXU);

				continue;
			}

			cv::Mat  img_scene_temp = cv::imread(this->surfGroups[i][j]->getOrigFilename().c_str(), cv::IMREAD_GRAYSCALE);
			cv::equalizeHist(img_scene_temp, img_scene);

			int match_method = cv::TM_CCOEFF_NORMED;

			cv::Mat img_display;
			img_scene.copyTo(img_display);

			int result_cols = img_scene.cols - img_object.cols + 1;
			int result_rows = img_scene.rows - img_object.rows + 1;

			result.create(result_rows, result_cols, CV_32FC1);

			cv::matchTemplate(img_scene, img_object, result, match_method);
			cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

			double minVal;
			double maxVal;
			cv::Point minLoc;
			cv::Point maxLoc;

			cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
			cv::Point matchLoc = maxLoc;

			cv::Point second_point = cv::Point(matchLoc.x + img_object.cols, matchLoc.y + img_object.rows);

			
			cv::rectangle(img_display, matchLoc, second_point, cv::Scalar::all(0), 2, 8, 0);
			cv::rectangle(result, matchLoc, second_point, cv::Scalar::all(0), 2, 8, 0);
			
			cv::Rect myROI(matchLoc.x, matchLoc.y, second_point.x - matchLoc.x, second_point.y - matchLoc.y);
			
			float start_rotation = -1;
			float start_shear = -0.1;
			cv::Point2f src_center(matchLoc.x + (second_point.x - matchLoc.x)/2, matchLoc.y + (second_point.y - matchLoc.y)/2);
			for (int k = 0; k < 5; k++) {
				
				cv::Mat rot_mat = getRotationMatrix2D(src_center, start_rotation, 1.0);
				cv::Mat dst;
				warpAffine(img_scene, dst, rot_mat, img_scene.size());

                                temp = POSITIVE_ROIS_IMAGE_PREFIX_PATH;
				temp += "roi_";
				temp += (char)('a' + i);
				temp.append("/"); // groups a-z
				temp.append(this->surfGroups[i][j]->getFilename().substr(this->surfGroups[i][j]->getFilename().find_last_of("/\\") + 1));
				size_t lastindex = temp.find_last_of(".");
				std::string rawname = temp.substr(0, lastindex);
			
				start_shear = -0.1;
				for (int l = 0; l < 5; l++) {
					
					cv::Mat cropped(dst, myROI);
					cv::Mat sheared = shearMat(cropped, start_shear);

                    std::ostringstream ss;
					if (start_rotation == 0 && start_shear == 0) {
                        ss << rawname;
                        ss << "_default";
                    }
                    else {
                        ss << rawname;
                        ss << "_r_";
                        ss << k;
                        ss << "s_";
                        ss << l;
                        ss << ".jpg";
                    }
					//std::cout << "temp " << ss.str().c_str() << std::endl;
					cv::imwrite(ss.str().c_str(), sheared);

					start_shear += 0.05;
				}
				
				start_rotation += 0.5;
			}
		}
	}
}

cv::Mat ROISelector::shearMat(cv::Mat img, float shear) {

	cv::Mat dest;
	img.copyTo(dest);
	
	cv::Mat M(2, 3, CV_32F);

	M.at<float>(0, 0) = 1;
	M.at<float>(0, 1) = shear;
	M.at<float>(0, 2) = 0;

	M.at<float>(1, 0) = 0;
	M.at<float>(1, 1) = 1;
	M.at<float>(1, 2) = 0;
	cv::Mat new_dest;
	warpAffine(dest, new_dest, M, cv::Size(dest.cols, dest.rows));
	return new_dest;

}

void ROISelector::segmentate_negative_ROIs() {

	cv::Mat img_scene;
	cv::Mat img_object;
	cv::Mat result;
    std::string temp = NEGATIVE_ROIS_IMAGE_PREFIX_PATH;
    mkdir(temp.c_str(), S_IRWXU);

    for (int i = 0; i < this->surfGroups.size(); i++) {

		// get template of negative ROI and select same image as positive ROI
		img_object = this->surfGroups[i][0]->getImage();
		// create new folder for saving ROIs
                temp = NEGATIVE_ROIS_IMAGE_PREFIX_PATH;
		temp += "roi_";
		temp += (char)('a' + i);
		temp.append("/"); // groups a-z
                mkdir(temp.c_str(), S_IRWXU);

		img_scene = this->equalizedImage;

		int match_method = cv::TM_CCOEFF_NORMED;

		cv::Mat img_display;
		img_scene.copyTo(img_display);

		int result_cols = img_scene.cols - img_object.cols + 1;
		int result_rows = img_scene.rows - img_object.rows + 1;

		result.create(result_rows, result_cols, CV_32FC1);

		cv::matchTemplate(img_scene, img_object, result, match_method);
		cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

		double minVal;
		double maxVal;
		cv::Point minLoc;
		cv::Point maxLoc;

		cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
		cv::Point matchLoc = maxLoc;

		cv::Point second_point = cv::Point(matchLoc.x + img_object.cols, matchLoc.y + img_object.rows);

		cv::rectangle(img_display, matchLoc, second_point, cv::Scalar::all(0), 2, 8, 0);
		cv::rectangle(result, matchLoc, second_point, cv::Scalar::all(0), 2, 8, 0);

		cv::Rect myROI(matchLoc.x, matchLoc.y, second_point.x - matchLoc.x, second_point.y - matchLoc.y);
		cv::Mat cropped(img_scene, myROI);

		//cv::imwrite(this->surfGroups[i][j]->getFilename(), cropped);
        temp = NEGATIVE_ROIS_IMAGE_PREFIX_PATH;
		temp += "roi_";
		temp += (char)('a' + i);
		temp.append("/"); // groups a-z
		temp.append(this->filename.substr(this->filename.find_last_of("/\\") + 1));
		temp.append(".jpg");

		cv::imwrite(temp, cropped);

	}
}

void ROISelector::addImageToGroup(double middleX, double middleY) {

	for (unsigned int i = 0; i < this->surfGroups.size(); i++) {
		if ((std::abs(middleX - this->surfGroups[i][0]->getX()) < MAX_DEVIATION) && (std::abs(middleY - this->surfGroups[i][0]->getY()) < MAX_DEVIATION)) {
			this->surfGroups.at(i).push_back(new ImageRecord(middleX, middleY, this->segment, this->segmentedImageFilename, this->filenameJPG));
			return;
		}
	}

	std::vector<ImageRecord*> new_group;
	this->surfGroups.push_back(new_group);
	this->surfGroups.at(this->surfGroups.size() - 1).push_back(new ImageRecord(middleX, middleY, this->segment, this->segmentedImageFilename, this->filenameJPG));
}

int ROISelector::writeImage() {
	// add index and extension '.jpg' to segmented image
	cv::imwrite(this->segmentedImageFilename, this->segment);

	return 1;
}

int ROISelector::runParser() {
	FILE* file;
	if ((file = openFile()) == nullptr) {
		exit(0);
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
			exit(0);
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
				//std::cout << "LINE, x1 = " << rootScopeTag->children.at(i + 1)->attributes.at("x1").c_str() << ", x2 = " << rootScopeTag->children.at(i + 1)->attributes.at("x2").c_str() << ", y1 = " << rootScopeTag->children.at(i + 1)->attributes.at("y1").c_str() << ", y2 = " << rootScopeTag->children.at(i + 1)->attributes.at("y2").c_str() << std::endl;
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
				//std::cout << "CIRCLE" << std::endl;
			}
		}
	}
	catch (std::exception e)
	{
		std::cout << "Error during parsing XML file. With error message: " << e.what() << std::endl;
		exit(0);
	}

	return 1;
}

std::vector<XMLTag*> ROISelector::getParsedRootScope() {
	return this->parsedRootScope;
}

std::vector<Line*> ROISelector::getParsedLines() {
	return this->parsedLines;
}
