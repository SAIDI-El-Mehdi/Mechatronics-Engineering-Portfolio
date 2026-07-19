#include "sobel_opencv_opt.hpp"

void sobel_opencv_opt(
    const cv::Mat& gray,
    cv::Mat& gx,
    cv::Mat& gy,
    cv::Mat& mag,
    cv::Mat& out
) {
    cv::Sobel(gray, gx, CV_32F, 1, 0, 3);
    cv::Sobel(gray, gy, CV_32F, 0, 1, 3);
    cv::magnitude(gx, gy, mag);
    mag.convertTo(out, CV_8U);

    out.row(0).setTo(0);
    out.row(out.rows - 1).setTo(0);
    out.col(0).setTo(0);
    out.col(out.cols - 1).setTo(0);
}
