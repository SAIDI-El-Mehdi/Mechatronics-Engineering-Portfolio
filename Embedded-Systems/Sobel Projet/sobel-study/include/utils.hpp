#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <chrono>

double mean_of(const std::vector<double>& v);
double median_of(std::vector<double> v);
double stddev_of(const std::vector<double>& v, double mean);

template <typename Func>
double time_ms(Func&& f, cv::Mat& result) {
    auto t0 = std::chrono::high_resolution_clock::now();
    result = f();
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> dt = t1 - t0;
    return dt.count();
}
