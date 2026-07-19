#pragma once
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

std::vector<fs::path> list_images(const std::string& dir);
bool is_image_file(const fs::path& p);
void ensure_dir(const fs::path& p);
cv::Mat load_gray_resized(const std::string& path, int width, int height);
