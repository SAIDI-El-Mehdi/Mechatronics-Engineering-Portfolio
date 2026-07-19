#include "sobel_manual.hpp"
#include <cmath>

cv::Mat sobel_manual(const cv::Mat& gray) {
    cv::Mat out(gray.rows, gray.cols, CV_8U, cv::Scalar(0));

    for (int y = 1; y < gray.rows - 1; ++y) {
        const uchar* r0 = gray.ptr<uchar>(y - 1);
        const uchar* r1 = gray.ptr<uchar>(y);
        const uchar* r2 = gray.ptr<uchar>(y + 1);
        uchar* dst = out.ptr<uchar>(y);

        for (int x = 1; x < gray.cols - 1; ++x) {
            int gx =
                -r0[x - 1] + r0[x + 1]
                -2 * r1[x - 1] + 2 * r1[x + 1]
                -r2[x - 1] + r2[x + 1];

            int gy =
                -r0[x - 1] - 2 * r0[x] - r0[x + 1]
                +r2[x - 1] + 2 * r2[x] + r2[x + 1];

            int mag = static_cast<int>(std::sqrt(static_cast<float>(gx * gx + gy * gy)));
            if (mag > 255) mag = 255;
            dst[x] = static_cast<uchar>(mag);
        }
    }

    return out;
}
