#include "ROISelector.h"

int main(int argc, char** argv) {

	std::string filename = "image.svg";
	ROISelector* selector = new ROISelector(filename);
	std::cout << "Result of parsing: " << selector->startParser() << std::endl;	
	std::cout << "Result of selecting: " << selector->findTags() << std::endl;
	// open image.jpg -> equalized it 
	// method in selector getLines (in future, next shapes too) cut 128 x 128 ROIs
	
	return 0;

}