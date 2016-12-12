#include <stdlib.h>
#include <windows.h>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"

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

	FILE* file = fopen(filename.c_str(), "r");
	if (!file)
	{
		std::cerr << "Opening of file '" << filename.c_str() << "' failed" << std::endl;
		return nullptr;
	}

	return file;
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

//	cv::imshow("sharpened", sharpenedImg);
//	cv::imshow("equalized", equalizedImage);
//	cv::waitKey(0);

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
	// add name of processed file
	// separate name of file and index of line with '_'
	segmentFilename.append(this->filename);
	segmentFilename.append("_");
	index = 0;

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
		this->segmentedImageFilename = (temp.append(std::to_string(index++))).append(".jpg");
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
			CreateDirectory(temp.c_str(), NULL);
			temp.append(this->surfGroups[i][j]->getFilename().substr(this->surfGroups[i][j]->getFilename().find_last_of("/\\") + 1));
			cv::imwrite(temp, this->surfGroups[i][j]->getImage());
		}
	}
}

void ROISelector::processSURF() {

	int minHessian = 400;
	cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create(minHessian);

	std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
	cv::Mat descriptors_object, descriptors_scene;
	cv::Mat img_scene;
	cv::Mat img_object;
	cv::Mat result;

	for (int j = 0; j < this->surfGroups.at(0).size(); j++) {
			
		if (j == 0) {
			img_object = this->surfGroups[0][0]->getImage();
			detector->detectAndCompute(img_object, cv::Mat(), keypoints_object, descriptors_object);
			continue;
		}

		cv::Mat  img_scene_temp = cv::imread(this->surfGroups[0][j]->getOrigFilename().c_str(), cv::IMREAD_GRAYSCALE);
		cv::equalizeHist(img_scene_temp, img_scene);

		////////////////// pattern matching
		int match_method = cv::TM_SQDIFF_NORMED;

		cv::Mat img_display;
		img_scene.copyTo(img_display);
		int result_cols = img_scene.cols - img_object.cols + 1;
		int result_rows = img_scene.rows - img_object.rows + 1;
		result.create(result_rows, result_cols, CV_32FC1);
		cv::matchTemplate(img_scene, img_object, result, match_method);
		cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
		double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
		cv::Point matchLoc;
		cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
		if (match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
		{
			matchLoc = minLoc;
		}
		else
		{
			matchLoc = maxLoc;
		}
		cv::Point second_point = cv::Point(matchLoc.x + img_object.cols, matchLoc.y + img_object.rows);

		cv::rectangle(img_display, matchLoc, second_point, cv::Scalar::all(0), 2, 8, 0);
		cv::rectangle(result, matchLoc, second_point, cv::Scalar::all(0), 2, 8, 0);
		//cv::imshow("img_display", img_display);
		//cv::imshow("result", result);

		cv::Rect myROI(matchLoc.x, matchLoc.y, second_point.x - matchLoc.x, second_point.y - matchLoc.y);;
		cv::Mat cropped(img_scene, myROI);
		//imshow("cropped", cropped);
		cv::imwrite(this->surfGroups[0][j]->getFilename(), cropped);
		
		///////////////////

		//-- Step 1: Detect the keypoints and extract descriptors using SURF
		/*detector->detectAndCompute(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

		//-- Step 2: Matching descriptor vectors using FLANN matcher
		cv::FlannBasedMatcher matcher;
		std::vector< cv::DMatch > matches;
		matcher.match(descriptors_object, descriptors_scene, matches);
		double max_dist = 0; double min_dist = 100;
		//-- Quick calculation of max and min distances between keypoints
		for (int i = 0; i < descriptors_object.rows; i++) {
			double dist = matches[i].distance;
			if (dist < min_dist) min_dist = dist;
			if (dist > max_dist) max_dist = dist;
		}
		//printf("-- Max dist : %f \n", max_dist);
		//printf("-- Min dist : %f \n", min_dist);

		//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
		std::vector< cv::DMatch > good_matches;
		for (int i = 0; i < descriptors_object.rows; i++) {
			if (matches[i].distance < 3 * min_dist)	{
				good_matches.push_back(matches[i]);
			}
		}
		cv::Mat img_matches;
		cv::drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
			good_matches, img_matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
			std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		//-- Localize the object
		std::vector<cv::Point2f> obj;
		std::vector<cv::Point2f> scene;
		for (size_t i = 0; i < good_matches.size(); i++) {
			//-- Get the keypoints from the good matches
			obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
			scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
		}
		cv::Mat H = cv::findHomography(obj, scene, cv::RANSAC);
		//-- Get the corners from the image_1 ( the object to be "detected" )
		std::vector<cv::Point2f> obj_corners(4);
		obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(img_object.cols, 0);
		obj_corners[2] = cvPoint(img_object.cols, img_object.rows); obj_corners[3] = cvPoint(0, img_object.rows);
		std::vector<cv::Point2f> scene_corners(4);
		cv::perspectiveTransform(obj_corners, scene_corners, H);
		//-- Draw lines between the corners (the mapped object in the scene - image_2 )
		line(img_matches, scene_corners[0] + cv::Point2f(img_object.cols, 0), scene_corners[1] + cv::Point2f(img_object.cols, 0), cv::Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[1] + cv::Point2f(img_object.cols, 0), scene_corners[2] + cv::Point2f(img_object.cols, 0), cv::Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[2] + cv::Point2f(img_object.cols, 0), scene_corners[3] + cv::Point2f(img_object.cols, 0), cv::Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[3] + cv::Point2f(img_object.cols, 0), scene_corners[0] + cv::Point2f(img_object.cols, 0), cv::Scalar(0, 255, 0), 4);

		// align to rectangle +----------+
		/*cv::Point2f dst_vertices[3];
		dst_vertices[0] = cv::Point(0, 0);
		dst_vertices[1] = cv::Point(img_object.rows - 1, 0);
		dst_vertices[2] = cv::Point(0, img_object.cols - 1);

		cv::Point2f src_vertices[3];
		dst_vertices[0] = scene_corners[0];
		dst_vertices[1] = scene_corners[1];
		dst_vertices[2] = scene_corners[2];

		cv::Mat warpAffineMatrix = cv::getAffineTransform(src_vertices, dst_vertices);

		cv::Mat rotated;
		cv::Size size(img_object.rows, img_object.cols);
		cv::warpAffine(img_scene, rotated, warpAffineMatrix, size, cv::INTER_LINEAR, cv::BORDER_CONSTANT);*/

		


		//cv::Mat result;
		// create image with size of pattern and align image according to this pattern
		//cv::warpPerspective(img_scene, result, H, img_scene.size());
		//imshow("warpPerspective", result);


		
		/*cv::Mat rot_mat = getRotationMatrix2D(scene_corners[0], get_angle(scene_corners[0], scene_corners[1]), 1.0);
		std::cout << "angle is " << get_angle(scene_corners[0], scene_corners[1]) << std::endl;

		float angle = get_angle(scene_corners[0], scene_corners[1]);
		int length = 150;
		cv::Point P1(50, 50);
		cv::Point P2;

		P2.x = (int)round(P1.x + length * cos(angle * CV_PI / 180.0));
		P2.y = (int)round(P1.y + length * sin(angle * CV_PI / 180.0));
		

		cv::Mat dst;
		warpAffine(img_scene, dst, rot_mat, img_scene.size());
		//imshow("Good ANGLE", dst);

		cv::Mat croppedImage;
		// setup a rectangle to define your region of interest - before warp transformation 
		cv::Rect myROI(std::abs(std::min(scene_corners[0].x, scene_corners[3].x)), std::abs(std::min(scene_corners[0].y, scene_corners[1].y)),
			img_object.rows,
			img_object.cols);
		cv::line(img_matches, scene_corners[0], P2,	cv::Scalar(0, 0, 0), 1,	1);

		// Crop the full image to that image contained by the rectangle myROI
		// Note that this doesn't copy the data
		cv::Mat croppedRef(dst, myROI);
		cv::Mat cropped;
		// Copy the data into new matrix
		croppedRef.copyTo(cropped);
		//imshow("cropped", cropped);

		//-- Show detected matches
		//imshow("cropped", cropped);
		imshow("Good Matches & Object detection", img_matches);

		cv::imwrite(this->surfGroups[0][j]->getFilename(), cropped);
		cv::waitKey(0);*/


	}

}

// This function calculates the angle of the line from A to B with respect to the positive X-axis in degrees
float ROISelector::get_angle(cv::Point2f A, cv::Point2f B) {
	// height
	float v = std::abs(B.y - A.y);
	// adjacent
	float p = std::abs(B.x - A.x);

	float angle = std::atan(float(v / p)) * 180 / PI;

	return angle;
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