#include "io.hpp"
#include <algorithm>
#include <cctype>

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool is_image_file(const fs::path& p) {
    static const std::vector<std::string> exts = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff"
    };
    const std::string ext = to_lower(p.extension().string());
    return std::find(exts.begin(), exts.end(), ext) != exts.end();
}

std::vector<fs::path> list_images(const std::string& dir) {
    std::vector<fs::path> files;
    if (!fs::exists(dir)) return files;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file() && is_image_file(entry.path())) {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

void ensure_dir(const fs::path& p) {
    if (!fs::exists(p)) fs::create_directories(p);
}

cv::Mat load_gray_resized(const std::string& path, int width, int height) {
    cv::Mat color = cv::imread(path, cv::IMREAD_COLOR);
    if (color.empty()) return {};

    cv::Mat resized, gray;
    cv::resize(color, resized, cv::Size(width, height));
    cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
