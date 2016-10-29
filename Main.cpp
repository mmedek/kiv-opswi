#include "ROISelector.h"

int main(int argc, char** argv) {

	std::string filename = "image.svg";
	ROISelector* selector = new ROISelector(filename);
	// good result = 1
	std::cout << "Result of parsing: " << selector->runParser() << std::endl;	
	std::cout << "Result of selecting: " << selector->findTags() << std::endl;
	std::cout << "Result of equalization: " << selector->runEqualization() << std::endl;
	// add method in selector getLines (in future, next shapes too) cut 128 x 128 ROIs
	
	return 0;

}