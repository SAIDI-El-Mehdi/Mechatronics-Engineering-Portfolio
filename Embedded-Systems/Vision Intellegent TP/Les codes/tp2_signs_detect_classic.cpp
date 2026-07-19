#include <opencv2/opencv.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace cv;
using namespace std;

struct Hough4 {
    int h0 = 0;
    int h45 = 0;
    int h90 = 0;
    int h135 = 0;
    int hother = 0;
    int score = 0;
};

struct Detection {
    Rect bbox;
    int class_id = -1;
    float confidence = 1.0f;
    string color;
    int color_pct = 0;
    string shape;
    float circularity = 0.0f;
    int vertices = 0;
    string category;
    string source = "Classical";
    bool validated = false;
    Hough4 h4;
    int v_closest = 0;
};

static const vector<string> SUPPORTED_EXT = {".jpg", ".jpeg", ".png", ".bmp", ".tif", ".tiff"};

bool hasValidImageExtension(const fs::path& p) {
    string ext = p.extension().string();
    transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
    return find(SUPPORTED_EXT.begin(), SUPPORTED_EXT.end(), ext) != SUPPORTED_EXT.end();
}

vector<fs::path> listImages(const fs::path& input) {
    vector<fs::path> images;
    if (fs::is_regular_file(input) && hasValidImageExtension(input)) {
        images.push_back(input);
        return images;
    }
    if (!fs::exists(input) || !fs::is_directory(input)) return images;
    for (const auto& entry : fs::recursive_directory_iterator(input)) {
        if (entry.is_regular_file() && hasValidImageExtension(entry.path())) {
            images.push_back(entry.path());
        }
    }
    sort(images.begin(), images.end());
    return images;
}

void ensureDir(const fs::path& dir) {
    if (!fs::exists(dir)) fs::create_directories(dir);
}

template <typename F>
double measureMs(F&& func) {
    auto t0 = chrono::high_resolution_clock::now();
    func();
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration<double, milli>(t1 - t0).count();
}

string safeStem(const fs::path& p) {
    string s = p.stem().string();
    for (char& c : s) {
        if (!isalnum(static_cast<unsigned char>(c))) c = '_';
    }
    return s;
}

Scalar colorScalar(const string& color) {
    if (color == "red") return Scalar(0, 0, 255);
    if (color == "blue") return Scalar(255, 100, 0);
    if (color == "yellow") return Scalar(0, 220, 255);
    return Scalar(0, 255, 0);
}

double computeCircularity(const vector<Point>& contour) {
    double area = contourArea(contour);
    double peri = arcLength(contour, true);
    if (peri <= 1e-6) return 0.0;
    return 4.0 * CV_PI * area / (peri * peri);
}

int closestStandardVertex(int v) {
    static const int stdV[] = {3, 4, 5, 6, 8, 12};
    int best = 3;
    int bestDist = 999;
    for (int sv : stdV) {
        int d = abs(sv - v);
        if (d < bestDist) {
            bestDist = d;
            best = sv;
        }
    }
    return best;
}

Hough4 hough4Vote(const Mat& gray) {
    Hough4 h4;
    Mat blur, edges;
    GaussianBlur(gray, blur, Size(3, 3), 0);
    Canny(blur, edges, 30, 100);

    vector<Vec2f> lines;
    HoughLines(edges, lines, 1, CV_PI / 180, 30);

    for (const auto& l : lines) {
        float theta = l[1] * 180.0f / static_cast<float>(CV_PI);
        while (theta >= 180.0f) theta -= 180.0f;
        while (theta < 0.0f) theta += 180.0f;

        auto closeTo = [&](float a) {
            float diff = fabs(theta - a);
            diff = min(diff, 180.0f - diff);
            return diff <= 10.0f;
        };

        if (closeTo(0.0f)) h4.h0++;
        else if (closeTo(45.0f)) h4.h45++;
        else if (closeTo(90.0f)) h4.h90++;
        else if (closeTo(135.0f)) h4.h135++;
        else h4.hother++;
    }

    int threshold = 3;
    h4.score = 0;
    if (h4.h0 > threshold) h4.score++;
    if (h4.h45 > threshold) h4.score++;
    if (h4.h90 > threshold) h4.score++;
    if (h4.h135 > threshold) h4.score++;
    return h4;
}

string detectShape(const Mat& gray, float& circ_out, int& v_out, Hough4& h4_out) {
    Mat blur, edges;
    GaussianBlur(gray, blur, Size(3, 3), 0);
    Canny(blur, edges, 30, 100);

    vector<vector<Point>> contours;
    findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        circ_out = 0.0f;
        v_out = 0;
        h4_out = hough4Vote(gray);
        return "unknown";
    }

    auto best = max_element(contours.begin(), contours.end(),
        [](const vector<Point>& a, const vector<Point>& b) {
            return contourArea(a) < contourArea(b);
        });

    double area = contourArea(*best);
    double peri = arcLength(*best, true);
    if (area < 10.0 || peri <= 1e-6) {
        circ_out = 0.0f;
        v_out = 0;
        h4_out = hough4Vote(gray);
        return "unknown";
    }

    vector<Point> approx;
    approxPolyDP(*best, approx, 0.04 * peri, true);

    circ_out = static_cast<float>(computeCircularity(*best));
    v_out = static_cast<int>(approx.size());
    h4_out = hough4Vote(gray);

    if (circ_out > 0.82f) return "circle";
    if (circ_out > 0.70f && v_out >= 6) return "octagon";
    if (v_out == 3) return "triangle";
    if (v_out == 4) return "rectangle";
    if (v_out > 6) return "octagon";
    return "polygon";
}

pair<string, int> dominantColorPct(const Mat& hsv_roi) {
    Mat r1, r2, r, b, y;
    inRange(hsv_roi, Scalar(0, 80, 60), Scalar(12, 255, 255), r1);
    inRange(hsv_roi, Scalar(168, 80, 60), Scalar(180, 255, 255), r2);
    bitwise_or(r1, r2, r);
    inRange(hsv_roi, Scalar(95, 80, 50), Scalar(145, 255, 255), b);
    inRange(hsv_roi, Scalar(18, 80, 80), Scalar(35, 255, 255), y);

    int total = hsv_roi.rows * hsv_roi.cols;
    if (total <= 0) return {"unknown", 0};

    int redPct = static_cast<int>(round(100.0 * countNonZero(r) / total));
    int bluePct = static_cast<int>(round(100.0 * countNonZero(b) / total));
    int yellowPct = static_cast<int>(round(100.0 * countNonZero(y) / total));

    if (redPct >= bluePct && redPct >= yellowPct) return {"red", redPct};
    if (bluePct >= redPct && bluePct >= yellowPct) return {"blue", bluePct};
    return {"yellow", yellowPct};
}

string inferCategory(const string& color, const string& shape) {
    if (color == "red" && shape == "octagon") return "STOP Sign";
    if (color == "yellow" && shape == "triangle") return "Warning Sign";
    if (color == "red" && shape == "triangle") return "Yield / Warning Sign";
    if (color == "blue" && shape == "rectangle") return "Information / Direction Sign";
    if (color == "red" && shape == "rectangle") return "Prohibition Sign";
    if (color == "yellow" && shape == "rectangle") return "Caution Sign";
    if (color == "red") return "Red Sign (Unclassified)";
    if (color == "blue") return "Blue Sign (Unclassified)";
    if (color == "yellow") return "Yellow Sign (Unclassified)";
    return "Unknown";
}

double intersectionOverUnion(const Rect& a, const Rect& b) {
    Rect inter = a & b;
    Rect uni = a | b;
    if (uni.area() <= 0) return 0.0;
    return static_cast<double>(inter.area()) / static_cast<double>(uni.area());
}

vector<Detection> mergeCloseDetections(vector<Detection> detections) {
    vector<Detection> merged;
    vector<bool> used(detections.size(), false);

    for (size_t i = 0; i < detections.size(); ++i) {
        if (used[i]) continue;
        Detection best = detections[i];
        used[i] = true;

        for (size_t j = i + 1; j < detections.size(); ++j) {
            if (used[j]) continue;
            if (intersectionOverUnion(best.bbox, detections[j].bbox) > 0.35) {
                if (detections[j].color_pct > best.color_pct ||
                    detections[j].bbox.area() > best.bbox.area()) {
                    best = detections[j];
                }
                used[j] = true;
            }
        }
        merged.push_back(best);
    }
    return merged;
}

void saveImage(const fs::path& folder, const string& filename, const Mat& img) {
    ensureDir(folder);
    imwrite((folder / filename).string(), img);
}

class Detector {
private:
    Scalar red_lo1 = Scalar(0, 80, 60);
    Scalar red_hi1 = Scalar(12, 255, 255);
    Scalar red_lo2 = Scalar(168, 80, 60);
    Scalar red_hi2 = Scalar(180, 255, 255);
    Scalar blue_lo = Scalar(95, 80, 50);
    Scalar blue_hi = Scalar(145, 255, 255);
    Scalar yellow_lo = Scalar(18, 80, 80);
    Scalar yellow_hi = Scalar(35, 255, 255);

public:
    pair<Mat, vector<Detection>> detect(const Mat& image, const fs::path& debugDir, ostream& report) {
        Mat blur, hsv, r1, r2, r, b, y, mask, opening, closing;
        Mat contourDebug = Mat::zeros(image.size(), CV_8UC3);
        Mat output = image.clone();

        saveImage(debugDir, "win0_original.png", image);

        GaussianBlur(image, blur, Size(5, 5), 0);
        saveImage(debugDir, "win1_blur.png", blur);

        cvtColor(blur, hsv, COLOR_BGR2HSV);
        saveImage(debugDir, "win2_hsv.png", hsv);

        inRange(hsv, red_lo1, red_hi1, r1);
        inRange(hsv, red_lo2, red_hi2, r2);
        bitwise_or(r1, r2, r);
        inRange(hsv, blue_lo, blue_hi, b);
        inRange(hsv, yellow_lo, yellow_hi, y);

        saveImage(debugDir, "win3_red_mask.png", r);
        saveImage(debugDir, "win4_blue_mask.png", b);
        saveImage(debugDir, "win5_yellow_mask.png", y);

        bitwise_or(r, b, mask);
        bitwise_or(mask, y, mask);
        saveImage(debugDir, "win6_combined_mask.png", mask);

        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
        morphologyEx(mask, opening, MORPH_OPEN, kernel);
        morphologyEx(opening, closing, MORPH_CLOSE, kernel);
        saveImage(debugDir, "win7_opening.png", opening);
        saveImage(debugDir, "win8_closing.png", closing);

        vector<vector<Point>> contours;
        findContours(closing, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        report << "[INFO] Classical candidates: " << contours.size() << "\n";

        vector<Detection> detections;

        for (const auto& c : contours) {
            double area = contourArea(c);
            if (area < 300.0) continue;

            Rect bb = boundingRect(c);
            bb &= Rect(0, 0, image.cols, image.rows);
            if (bb.width <= 5 || bb.height <= 5) continue;

            Mat roi = image(bb).clone();
            Mat hsv_roi, gray_roi;
            cvtColor(roi, hsv_roi, COLOR_BGR2HSV);
            cvtColor(roi, gray_roi, COLOR_BGR2GRAY);

            float circ = 0.0f;
            int vertices = 0;
            Hough4 h4;
            string shape = detectShape(gray_roi, circ, vertices, h4);
            auto colorResult = dominantColorPct(hsv_roi);

            Detection d;
            d.bbox = bb;
            d.color = colorResult.first;
            d.color_pct = colorResult.second;
            d.shape = shape;
            d.circularity = circ;
            d.vertices = vertices;
            d.v_closest = closestStandardVertex(vertices);
            d.h4 = h4;
            d.category = inferCategory(d.color, d.shape);

            detections.push_back(d);

            drawContours(contourDebug, vector<vector<Point>>{c}, -1, Scalar(0, 255, 0), 2);
            saveImage(debugDir, "win10_roi.png", roi);

            Mat roiBlur, roiEdges, roiContours = Mat::zeros(roi.size(), CV_8UC3);
            GaussianBlur(gray_roi, roiBlur, Size(3, 3), 0);
            Canny(roiBlur, roiEdges, 30, 100);
            vector<vector<Point>> roiLocalContours;
            findContours(roiEdges, roiLocalContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            drawContours(roiContours, roiLocalContours, -1, Scalar(0, 255, 0), 2);
            saveImage(debugDir, "win12_canny_roi.png", roiEdges);
            saveImage(debugDir, "win13_contours_roi.png", roiContours);

            report << "  [ROI] bbox=(" << bb.x << "," << bb.y << "," << bb.width << "," << bb.height << ")"
                   << " color=" << d.color << " pct=" << d.color_pct
                   << "% shape=" << d.shape << " category=" << d.category
                   << " circ=" << fixed << setprecision(2) << d.circularity
                   << " vertices=" << d.vertices
                   << " v_closest=" << d.v_closest
                   << " Hough4[0=" << h4.h0 << ",45=" << h4.h45 << ",90=" << h4.h90
                   << ",135=" << h4.h135 << ",other=" << h4.hother
                   << ",score=" << h4.score << "]\n";
        }

        saveImage(debugDir, "win9_contours.png", contourDebug);

        detections = mergeCloseDetections(detections);

        Mat gray_full;
        cvtColor(blur, gray_full, COLOR_BGR2GRAY);
        vector<Vec3f> circles;
        HoughCircles(gray_full, circles, HOUGH_GRADIENT, 1, 30, 100, 30, 10, 200);

        Mat houghCirclesDebug = image.clone();
        for (const auto& detectedCircle : circles) {
            Point center(cvRound(detectedCircle[0]), cvRound(detectedCircle[1]));
            int radius = cvRound(detectedCircle[2]);
            cv::circle(houghCirclesDebug, center, radius, Scalar(0, 255, 255), 2);
            cv::circle(houghCirclesDebug, center, 2, Scalar(0, 0, 255), 3);
        }
        saveImage(debugDir, "win_hough_circles.png", houghCirclesDebug);
        report << "[INFO] Hough Circles found: " << circles.size() << "\n";

        for (auto& d : detections) {
            for (const auto& cir : circles) {
                Point c(cvRound(cir[0]), cvRound(cir[1]));
                if (d.bbox.contains(c)) {
                    d.validated = true;
                    break;
                }
            }

            Scalar boxColor = colorScalar(d.color);
            rectangle(output, d.bbox, boxColor, 2);

            string label = d.color + " | " + d.shape + " (" + to_string(d.color_pct) + "%)";
            int baseline = 0;
            Size ts = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            int y0 = max(0, d.bbox.y - ts.height - 6);
            rectangle(output, Point(d.bbox.x, y0),
                      Point(min(output.cols - 1, d.bbox.x + ts.width + 4), d.bbox.y),
                      boxColor, FILLED);
            putText(output, label, Point(d.bbox.x + 2, max(14, d.bbox.y - 4)),
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
        }

        saveImage(debugDir, "win_final_result.png", output);

        return {output, detections};
    }
};

void writeCSVHeader(ofstream& csv) {
    csv << "image,source,color,color_pct,shape,circularity,vertices,v_closest,category,"
        << "validated,x,y,w,h,h0,h45,h90,h135,hother,hscore\n";
}

void writeCSVRows(ofstream& csv, const string& imageName, const vector<Detection>& detections) {
    for (const auto& d : detections) {
        csv << imageName << ","
            << d.source << ","
            << d.color << ","
            << d.color_pct << ","
            << d.shape << ","
            << fixed << setprecision(3) << d.circularity << ","
            << d.vertices << ","
            << d.v_closest << ","
            << "\"" << d.category << "\"" << ","
            << (d.validated ? "true" : "false") << ","
            << d.bbox.x << "," << d.bbox.y << "," << d.bbox.width << "," << d.bbox.height << ","
            << d.h4.h0 << "," << d.h4.h45 << "," << d.h4.h90 << "," << d.h4.h135 << ","
            << d.h4.hother << "," << d.h4.score << "\n";
    }
}

int main(int argc, char** argv) {
    fs::path inputPath = "images/input";
    fs::path outputPath = "images/output/classic";

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "--input" || arg == "-i") && i + 1 < argc) inputPath = argv[++i];
        else if ((arg == "--output" || arg == "-o") && i + 1 < argc) outputPath = argv[++i];
        else if (arg == "--help" || arg == "-h") {
            cout << "Usage: ./detect_classic.exe --input images/input --output images/output/classic\n";
            return 0;
        }
    }

    ensureDir(outputPath);

    vector<fs::path> images = listImages(inputPath);
    if (images.empty()) {
        cerr << "[ERROR] No images found in: " << inputPath << "\n";
        cerr << "Supported formats: jpg, jpeg, png, bmp, tif, tiff\n";
        return 1;
    }

    ofstream terminalReport(outputPath / "terminal_report_classic.txt");
    ofstream csv(outputPath / "classic_detections.csv");
    if (!terminalReport || !csv) {
        cerr << "[ERROR] Cannot create report files in: " << outputPath << "\n";
        return 1;
    }
    writeCSVHeader(csv);

    Detector detector;
    double totalMs = 0.0;

    terminalReport << "CLASSICAL OPENCV ROAD-SIGN DETECTION REPORT\n";
    terminalReport << "===========================================\n\n";
    terminalReport << "Input: " << inputPath << "\n";
    terminalReport << "Output: " << outputPath << "\n\n";

    cout << "[INFO] Input : " << inputPath << "\n";
    cout << "[INFO] Output: " << outputPath << "\n";

    for (const auto& imagePath : images) {
        Mat image = imread(imagePath.string(), IMREAD_COLOR);
        if (image.empty()) {
            cerr << "[WARN] Cannot read image: " << imagePath << "\n";
            continue;
        }

        string name = safeStem(imagePath);
        fs::path debugDir = outputPath / name;
        ensureDir(debugDir);

        terminalReport << "Image: " << imagePath << "\n";
        cout << "[INFO] Processing: " << imagePath << "\n";

        pair<Mat, vector<Detection>> result;
        double ms = measureMs([&]() {
            result = detector.detect(image, debugDir, terminalReport);
        });
        totalMs += ms;

        string finalName = name + "_final_result.png";
        imwrite((outputPath / finalName).string(), result.first);
        writeCSVRows(csv, imagePath.filename().string(), result.second);

        terminalReport << "[INFO] Final detections: " << result.second.size() << "\n";
        terminalReport << "[INFO] Processing time: " << fixed << setprecision(3) << ms << " ms\n\n";

        cout << "       detections=" << result.second.size()
             << " time=" << fixed << setprecision(3) << ms
             << " ms saved=" << (outputPath / finalName) << "\n";
    }

    terminalReport << "TOTAL TIME: " << fixed << setprecision(3) << totalMs << " ms\n";
    cout << "[DONE] Classical branch completed.\n";
    cout << "[INFO] Report: " << (outputPath / "terminal_report_classic.txt") << "\n";
    cout << "[INFO] CSV   : " << (outputPath / "classic_detections.csv") << "\n";
    return 0;
}
