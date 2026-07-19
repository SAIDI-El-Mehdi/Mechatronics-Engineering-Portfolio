#pragma once
#include <opencv2/opencv.hpp>

void sobel_opencv_opt(
    const cv::Mat& gray,
    cv::Mat& gx,
    cv::Mat& gy,
    cv::Mat& mag,
    cv::Mat& out
);
