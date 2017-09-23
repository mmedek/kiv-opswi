#include "ROISelector.h"
#include <dirent.h>

std::vector<std::string> get_all_files_names_within_folder(std::string folder);
void prepareNegativeROIs();
void preparePostiveROIs();

ROISelector* selector;

int main(int argc, char** argv) {


    preparePostiveROIs();
    prepareNegativeROIs();

    return 0;

}

void preparePostiveROIs() {

    std::string folder = "data/ABoard_p/";
    selector = new ROISelector();
    bool run = false;

    for (std::string& image : get_all_files_names_within_folder(folder)) {

        selector->set_new_image(image);
        std::cout << "Processing image '" << image.c_str() << "' started" << std::endl;
        // success result = 1
        //std::cout << "Result of parsing: "  << std::endl;
        selector->runParser();
        std::cout << "Result of selecting: " << selector->findTags() << std::endl;
        //selector->findTags();
        //method for segmentation lines (in future, next shapes too) cut 128 x 128 ROIs
        std::cout << "Result of segmentations ROIs: " << selector->cutROIs() << std::endl;
        //selector->cutROIs();
        std::cout << "Result of equalization: " << selector->preprocess() << std::endl;
        //selector->preprocess();
        std::cout << "Processing image '" << image.c_str() << "' ended" << std::endl;

        run = true;
    }
    // if was processing images running write image to groups
    if (run) {
        selector->writeGroups();

        // 1) Extract features (SURF or SIFT or others...)
        // 2) Match the features (FLANN or BruteForce...) and filter the matchings
        // 3) Find the geometrical transformation (RANSAC or LMeds...)
        selector->processSURF();


    }
}

void prepareNegativeROIs() {

    std::string folder = "data/ABoard_n/";
    bool run = false;

    std::cout << "Generating negative ROIs started" << std::endl;

    for (std::string& image : get_all_files_names_within_folder(folder)) {

        selector->set_new_image(image);
        //std::cout << "Processing image '" << image.c_str() << "' started" << std::endl;
        // success result = 1
        //std::cout << "Result of parsing: " << selector->runParser() << std::endl;
        //std::cout << "Result of selecting: " << selector->findTags() << std::endl;
        //method for segmentation lines (in future, next shapes too) cut 128 x 128 ROIs
        //std::cout << "Result of segmentations ROIs: " << selector->cutROIs() << std::endl;
        //std::cout << "Result of equalization: " <<  << std::endl;
        selector->preprocess();
        selector->segmentate_negative_ROIs();

        //std::cout << "Processing image '" << image.c_str() << "' ended" << std::endl;
    }

    std::cout << "Generating negative ROIs ended" << std::endl;


}

std::vector<std::string> get_all_files_names_within_folder(std::string folder) {

    std::vector<std::string> names;
    std::string search_path = folder + "/*.*";
    std::string full_path;

    DIR *dpdf;
    struct dirent *epdf;
    dpdf = opendir(folder.c_str());
    if (dpdf != NULL){
        while (epdf = readdir(dpdf)) {
            std::string fullname = folder;
            std::string path(epdf->d_name);
            fullname.append(path);

            // we want save only jpg files
            if (path.substr(path.find_last_of(".") + 1) == "svg") {
                // std::cout <<  path << std::endl;
                names.push_back(fullname);
            }
        }
    }

    closedir(dpdf);

    return names;
}
