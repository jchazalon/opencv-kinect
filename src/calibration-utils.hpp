#pragma once


#include <cstdint>
#include <opencv2/core.hpp>


/// @brief \brief Estimate the homography parameters from the coordinates of the corners of the wrapped image
/// @param coordinates 
/// @param width 
/// @param height 
/// @return  
cv::Mat unwrap_estimate(int coordinates[], int width, int height);


/// \brief Return a new image unwraped from the wrapped image
/// \param wrapped The image to unwrap
/// \param H The homography matrix 
cv::Mat unwrap(const cv::Mat& wrapped, const cv::Mat& H);