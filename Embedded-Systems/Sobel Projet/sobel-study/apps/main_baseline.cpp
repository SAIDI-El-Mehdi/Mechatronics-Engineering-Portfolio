#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>

#include "io.hpp"
#include "utils.hpp"
#include "sobel_manual.hpp"
#include "sobel_opencv.hpp"

namespace fs = std::filesystem;

struct Config {
    std::string input_dir = "../images/input";
    std::string output_dir = "../output/baseline";
    std::string results_dir = "../results/baseline";
    int runs = 30;
    int warmup = 5;
    int width = 640;
    int height = 480;
};

int main() {
    Config cfg;

    const fs::path output_root(cfg.output_dir);
    const fs::path results_root(cfg.results_dir);

    ensure_dir(output_root);
    ensure_dir(results_root);
    ensure_dir(output_root / "v1_manual");
    ensure_dir(output_root / "v2_opencv");

    std::ofstream raw_csv(results_root / "raw_results.csv");
    std::ofstream summary_csv(results_root / "summary.csv");

    raw_csv << "image,resolution,version,run_id,time_ms\n";
    summary_csv << "image,resolution,version,mean_ms,median_ms,min_ms,max_ms,std_ms,speedup_vs_manual,fps\n";

    auto images = list_images(cfg.input_dir);
    if (images.empty()) {
        std::cerr << "No images found in: " << cfg.input_dir << "\n";
        return 1;
    }

    std::map<std::string, double> manual_mean_by_image;

    for (const auto& image_path : images) {
        const std::string image_name = image_path.filename().string();
        cv::Mat gray = load_gray_resized(image_path.string(), cfg.width, cfg.height);
        if (gray.empty()) {
            std::cerr << "Failed to load: " << image_name << "\n";
            continue;
        }

        const std::string resolution = std::to_string(cfg.width) + "x" + std::to_string(cfg.height);

        for (const std::string& version : {"manual", "opencv"}) {
            for (int i = 0; i < cfg.warmup; ++i) {
                cv::Mat dummy = (version == "manual") ? sobel_manual(gray) : sobel_opencv(gray);
            }

            std::vector<double> times;
            cv::Mat result;

            for (int run = 1; run <= cfg.runs; ++run) {
                double ms = 0.0;
                if (version == "manual") {
                    ms = time_ms([&]() { return sobel_manual(gray); }, result);
                } else {
                    ms = time_ms([&]() { return sobel_opencv(gray); }, result);
                }

                times.push_back(ms);
                raw_csv << image_name << "," << resolution << "," << version << "," << run << "," << ms << "\n";
            }

            const double mean_ms = mean_of(times);
            const double median_ms = median_of(times);
            const double min_ms = *std::min_element(times.begin(), times.end());
            const double max_ms = *std::max_element(times.begin(), times.end());
            const double std_ms = stddev_of(times, mean_ms);
            const double fps = (mean_ms > 0.0) ? (1000.0 / mean_ms) : 0.0;

            double speedup_vs_manual = 0.0;
            if (version == "manual") {
                manual_mean_by_image[image_name] = mean_ms;
                speedup_vs_manual = 1.0;
            } else {
                speedup_vs_manual = manual_mean_by_image[image_name] / mean_ms;
            }

            summary_csv
                << image_name << ","
                << resolution << ","
                << version << ","
                << mean_ms << ","
                << median_ms << ","
                << min_ms << ","
                << max_ms << ","
                << std_ms << ","
                << speedup_vs_manual << ","
                << fps << "\n";

            fs::path save_path = (version == "manual")
                ? (output_root / "v1_manual" / image_name)
                : (output_root / "v2_opencv" / image_name);

            cv::imwrite(save_path.string(), result);

            std::cout
                << "[DONE] "
                << image_name
                << " | version=" << version
                << " | mean=" << mean_ms << " ms"
                << " | fps=" << fps
                << "\n";
        }
    }

    std::cout << "Baseline benchmark finished.\n";
    return 0;
}
