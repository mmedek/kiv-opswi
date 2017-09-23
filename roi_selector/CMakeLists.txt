cmake_minimum_required(VERSION 3.5.1)
project(roi_selector)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ImageRecord.cpp ImageRecord.h api.h Line.cpp Line.h ROISelector.cpp ROISelector.h XMLParser.cpp XMLParser.h)
add_executable(roi_selector ${SOURCE_FILES})


set(OpenCV_DIR "/usr/include/opencv")

# find required package - OpenCV
FIND_PACKAGE(OpenCV REQUIRED)

# link OpenCV
TARGET_LINK_LIBRARIES(roi_selector ${OpenCV_LIBS})

# add OpenCV include directory path
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${OpenCV_DIR}/include)