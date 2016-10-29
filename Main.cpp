#include "ROISelector.h"

int main(int argc, char** argv) {

	std::string filename = "image.svg";
	ROISelector* selector = new ROISelector(filename);

	std::cout << "Processing image '" << filename.c_str() << "' started" << std::endl;
	// success result = 1
	std::cout << "Result of parsing: " << selector->runParser() << std::endl;	
	std::cout << "Result of selecting: " << selector->findTags() << std::endl;
	std::cout << "Result of equalization: " << selector->preprocess() << std::endl;
	//method for segmentation lines (in future, next shapes too) cut 128 x 128 ROIs
	std::cout << "Result of segmentations ROIs: " << selector->cutROIs() << std::endl;
	std::cout << "Processing image '" << filename.c_str() << "' ended" << std::endl;
	
	return 0;

}