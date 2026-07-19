#include "sobel_manual_opt.hpp"
#include <cmath>
#ifdef _OPENMP
#include <omp.h>
#endif

cv::Mat sobel_manual_opt(const cv::Mat& gray) {
    cv::Mat out(gray.rows, gray.cols, CV_8U, cv::Scalar(0));

    #pragma omp parallel for schedule(static)
    for (int y = 1; y < gray.rows - 1; ++y) {
        const uchar* r0 = gray.ptr<uchar>(y - 1);
        const uchar* r1 = gray.ptr<uchar>(y);
        const uchar* r2 = gray.ptr<uchar>(y + 1);
        uchar* dst = out.ptr<uchar>(y);

        for (int x = 1; x < gray.cols - 1; ++x) {
            const int a00 = r0[x - 1];
            const int a01 = r0[x];
            const int a02 = r0[x + 1];
            const int a10 = r1[x - 1];
            const int a12 = r1[x + 1];
            const int a20 = r2[x - 1];
            const int a21 = r2[x];
            const int a22 = r2[x + 1];

            const int gx = -a00 + a02 - 2 * a10 + 2 * a12 - a20 + a22;
            const int gy = -a00 - 2 * a01 - a02 + a20 + 2 * a21 + a22;

            int mag = static_cast<int>(std::sqrt(static_cast<float>(gx * gx + gy * gy)));
            if (mag > 255) mag = 255;
            dst[x] = static_cast<uchar>(mag);
        }
    }

    return out;
}
