#include <Windows.h>

#include "ROISelector.h"

std::vector<std::string> get_all_files_names_within_folder(std::string folder);

int main(int argc, char** argv) {

	std::string folder = "C:/Users/mmedek.MMEDEK-NB/Documents/Visual Studio 2015/Projects/ROISelector/data/ABoard/";
	ROISelector* selector = new ROISelector();
	bool run = false;

	for (std::string& image : get_all_files_names_within_folder(folder)) {
		
		selector->set_new_image(image);
		std::cout << "Processing image '" << image.c_str() << "' started" << std::endl;
		// success result = 1
		std::cout << "Result of parsing: " << selector->runParser() << std::endl;
		std::cout << "Result of selecting: " << selector->findTags() << std::endl;
		//method for segmentation lines (in future, next shapes too) cut 128 x 128 ROIs
		std::cout << "Result of segmentations ROIs: " << selector->cutROIs() << std::endl;
		std::cout << "Result of equalization: " << selector->preprocess() << std::endl;
		std::cout << "Processing image '" << image.c_str() << "' ended" << std::endl;
		run = true;
	}
	// if was processing images running write image to groups
	if (run) {
		//selector->writeGroups();

		// 1) Extract features (SURF or SIFT or others...)
		// 2) Match the features (FLANN or BruteForce...) and filter the matchings
		// 3) Find the geometrical transformation (RANSAC or LMeds...)
		selector->processSURF();
		

	}

	return 0;

}

std::vector<std::string> get_all_files_names_within_folder(std::string folder) {

	std::vector<std::string> names;
	std::string search_path = folder + "/*.*";
	std::string fullFilename;
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		int index = 0;
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				fullFilename = fd.cFileName;
				// we not save jpg files
				if (fullFilename.substr(fullFilename.find_last_of(".") + 1).find("jpg") == std::string::npos) {
					std::string fullname = folder;
					fullname.append(fd.cFileName);
					names.push_back(fullname);
				}

			}
		} while (::FindNextFile(hFind, &fd));

		::FindClose(hFind);
		const size_t last_slash_idx = folder.find_last_of("\\/");
		
		if (std::string::npos != last_slash_idx)
		{
			folder.erase(0, last_slash_idx + 1);
		}

		return names;

	}
}