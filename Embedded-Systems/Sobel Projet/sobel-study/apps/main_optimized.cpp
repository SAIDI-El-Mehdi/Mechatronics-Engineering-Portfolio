#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>

#include "io.hpp"
#include "utils.hpp"
#include "sobel_manual_opt.hpp"
#include "sobel_opencv_opt.hpp"

namespace fs = std::filesystem;

int main() {
    const std::string input_dir = "../images/input";
    const std::string output_manual_dir = "../output/optimized/v3_manual_opt";
    const std::string output_opencv_dir = "../output/optimized/v4_opencv_opt";
    const std::string results_dir = "../results/optimized";
    const int runs = 30;
    const int warmup = 5;
    const int width = 640;
    const int height = 480;

    ensure_dir(output_manual_dir);
    ensure_dir(output_opencv_dir);
    ensure_dir(results_dir);

    std::ofstream raw_csv(results_dir + "/raw_results.csv");
    std::ofstream summary_csv(results_dir + "/summary.csv");

    raw_csv << "image,resolution,version,run_id,time_ms\n";
    summary_csv << "image,resolution,version,mean_ms,median_ms,min_ms,max_ms,std_ms,speedup_vs_manual_opt,fps\n";

    auto images = list_images(input_dir);
    if (images.empty()) {
        std::cerr << "No images found in " << input_dir << "\n";
        return 1;
    }

    for (const auto& image_path : images) {
        const std::string image_name = image_path.filename().string();
        cv::Mat gray = load_gray_resized(image_path.string(), width, height);
        if (gray.empty()) {
            std::cerr << "Failed to load " << image_name << "\n";
            continue;
        }

        for (int i = 0; i < warmup; ++i) {
            cv::Mat dummy = sobel_manual_opt(gray);
        }

        std::vector<double> times_manual;
        cv::Mat result_manual;

        for (int run = 1; run <= runs; ++run) {
            double ms = time_ms([&]() { return sobel_manual_opt(gray); }, result_manual);
            times_manual.push_back(ms);
            raw_csv << image_name << ",640x480,manual_opt," << run << "," << ms << "\n";
        }

        const double mean_manual = mean_of(times_manual);
        const double median_manual = median_of(times_manual);
        const double min_manual = *std::min_element(times_manual.begin(), times_manual.end());
        const double max_manual = *std::max_element(times_manual.begin(), times_manual.end());
        const double std_manual = stddev_of(times_manual, mean_manual);
        const double fps_manual = (mean_manual > 0.0) ? (1000.0 / mean_manual) : 0.0;

        summary_csv
            << image_name << ",640x480,manual_opt,"
            << mean_manual << "," << median_manual << "," << min_manual << ","
            << max_manual << "," << std_manual << "," << 1.0 << "," << fps_manual << "\n";

        cv::imwrite(output_manual_dir + "/" + image_name, result_manual);

        std::cout
            << "[DONE] " << image_name
            << " | version=manual_opt"
            << " | mean=" << mean_manual << " ms"
            << " | fps=" << fps_manual
            << "\n";

        cv::Mat gx(gray.size(), CV_32F);
        cv::Mat gy(gray.size(), CV_32F);
        cv::Mat mag(gray.size(), CV_32F);
        cv::Mat out(gray.size(), CV_8U);

        for (int i = 0; i < warmup; ++i) {
            sobel_opencv_opt(gray, gx, gy, mag, out);
        }

        std::vector<double> times_opencv;
        cv::Mat result_opencv;

        for (int run = 1; run <= runs; ++run) {
            auto t0 = std::chrono::high_resolution_clock::now();
            sobel_opencv_opt(gray, gx, gy, mag, out);
            auto t1 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> dt = t1 - t0;

            result_opencv = out.clone();
            double ms = dt.count();
            times_opencv.push_back(ms);
            raw_csv << image_name << ",640x480,opencv_opt," << run << "," << ms << "\n";
        }

        const double mean_opencv = mean_of(times_opencv);
        const double median_opencv = median_of(times_opencv);
        const double min_opencv = *std::min_element(times_opencv.begin(), times_opencv.end());
        const double max_opencv = *std::max_element(times_opencv.begin(), times_opencv.end());
        const double std_opencv = stddev_of(times_opencv, mean_opencv);
        const double fps_opencv = (mean_opencv > 0.0) ? (1000.0 / mean_opencv) : 0.0;
        const double speedup_vs_manual_opt = (mean_opencv > 0.0) ? (mean_manual / mean_opencv) : 0.0;

        summary_csv
            << image_name << ",640x480,opencv_opt,"
            << mean_opencv << "," << median_opencv << "," << min_opencv << ","
            << max_opencv << "," << std_opencv << "," << speedup_vs_manual_opt << "," << fps_opencv << "\n";

        cv::imwrite(output_opencv_dir + "/" + image_name, result_opencv);

        std::cout
            << "[DONE] " << image_name
            << " | version=opencv_opt"
            << " | mean=" << mean_opencv << " ms"
            << " | fps=" << fps_opencv
            << " | speedup_vs_manual_opt=" << speedup_vs_manual_opt
            << "\n";
    }

    std::cout << "Optimized benchmark finished.\n";
    return 0;
}
