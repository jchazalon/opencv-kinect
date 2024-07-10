#include "calibration-utils.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

cv::Mat unwrap_estimate(int coordinates[], int width, int height)
{
    std::vector<cv::Point2f> input_points = {
        cv::Point2f(coordinates[0], coordinates[1]),
        cv::Point2f(coordinates[2], coordinates[3]),
        cv::Point2f(coordinates[4], coordinates[5]),
        cv::Point2f(coordinates[6], coordinates[7])
    };

    std::vector<cv::Point2f> output_points = {
        cv::Point2f(0, 0),
        cv::Point2f(width, 0),
        cv::Point2f(width, height),
        cv::Point2f(0, height)
    };

    return cv::findHomography(input_points, output_points);
}

// Return a new image unwraped from the wrapped image
cv::Mat unwrap(const cv::Mat& wrapped, const cv::Mat& H)
{
    cv::Mat im_out;
    // Warp source image to destination based on homography
    cv::warpPerspective(wrapped, im_out, H, wrapped.size());
    return im_out;
}