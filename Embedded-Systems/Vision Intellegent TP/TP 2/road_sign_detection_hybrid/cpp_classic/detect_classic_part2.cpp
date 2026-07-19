#include <opencv2/opencv.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace cv;
using namespace std;

struct Params {
    int sat = 80;              // HSV saturation threshold: test 40, 80, 120
    double area = 300.0;       // contour area threshold: test 100, 300, 800
    double circ = 0.82;        // circularity threshold: test 0.82, 0.75
    double rdp = 0.04;         // approxPolyDP coefficient: test 0.02, 0.08
    int hough = 3;             // Hough4 vote threshold: test 3, 5
    fs::path input = "images/input";
    fs::path output = "images/output/part2";
};

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
    string color;
    int color_pct = 0;
    string shape;
    double circularity = 0.0;
    int vertices = 0;
    int v_closest = 0;
    Hough4 h4;
    string category;
};

bool hasImageExtension(const fs::path& p) {
    string ext = p.extension().string();
    transform(ext.begin(), ext.end(), ext.begin(),
              [](unsigned char c){ return static_cast<char>(tolower(c)); });
    return ext == ".jpg" || ext == ".jpeg" || ext == ".png" ||
           ext == ".bmp" || ext == ".tif" || ext == ".tiff";
}

vector<fs::path> listImages(const fs::path& input) {
    vector<fs::path> images;
    if (fs::is_regular_file(input) && hasImageExtension(input)) {
        images.push_back(input);
        return images;
    }
    if (!fs::exists(input) || !fs::is_directory(input)) return images;

    for (const auto& entry : fs::recursive_directory_iterator(input)) {
        if (entry.is_regular_file() && hasImageExtension(entry.path())) {
            images.push_back(entry.path());
        }
    }
    sort(images.begin(), images.end());
    return images;
}

void ensureDir(const fs::path& dir) {
    if (!fs::exists(dir)) fs::create_directories(dir);
}

template <typename Func>
double measureMs(Func&& func) {
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

Scalar drawColor(const string& color) {
    if (color == "red") return Scalar(0, 0, 255);
    if (color == "blue") return Scalar(255, 80, 0);
    if (color == "yellow") return Scalar(0, 220, 255);
    return Scalar(0, 255, 0);
}

int closestStandardVertex(int v) {
    vector<int> stdV = {3, 4, 5, 6, 8, 12};
    int best = stdV[0];
    int bestDist = abs(v - best);
    for (int value : stdV) {
        int d = abs(v - value);
        if (d < bestDist) {
            bestDist = d;
            best = value;
        }
    }
    return best;
}

double circularity(const vector<Point>& contour) {
    double area = contourArea(contour);
    double peri = arcLength(contour, true);
    if (peri <= 1e-9) return 0.0;
    return 4.0 * CV_PI * area / (peri * peri);
}

Hough4 hough4Vote(const Mat& gray, int threshold) {
    Hough4 h;
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

        if (closeTo(0.0f)) h.h0++;
        else if (closeTo(45.0f)) h.h45++;
        else if (closeTo(90.0f)) h.h90++;
        else if (closeTo(135.0f)) h.h135++;
        else h.hother++;
    }

    h.score = 0;
    if (h.h0 > threshold) h.score++;
    if (h.h45 > threshold) h.score++;
    if (h.h90 > threshold) h.score++;
    if (h.h135 > threshold) h.score++;
    return h;
}

string inferShape(const Mat& gray, const Params& params,
                  double& circOut, int& verticesOut, int& vClosestOut, Hough4& h4Out) {
    Mat blur, edges;
    GaussianBlur(gray, blur, Size(3, 3), 0);
    Canny(blur, edges, 30, 100);

    vector<vector<Point>> contours;
    findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        circOut = 0.0;
        verticesOut = 0;
        vClosestOut = 0;
        h4Out = hough4Vote(gray, params.hough);
        return "unknown";
    }

    auto best = max_element(contours.begin(), contours.end(),
        [](const vector<Point>& a, const vector<Point>& b) {
            return contourArea(a) < contourArea(b);
        });

    double area = contourArea(*best);
    double peri = arcLength(*best, true);
    if (area < 10.0 || peri <= 1e-9) {
        circOut = 0.0;
        verticesOut = 0;
        vClosestOut = 0;
        h4Out = hough4Vote(gray, params.hough);
        return "unknown";
    }

    vector<Point> approx;
    approxPolyDP(*best, approx, params.rdp * peri, true);

    circOut = circularity(*best);
    verticesOut = static_cast<int>(approx.size());
    vClosestOut = closestStandardVertex(verticesOut);
    h4Out = hough4Vote(gray, params.hough);

    if (circOut >= params.circ) return "circle";
    if (circOut > 0.70 && verticesOut >= 6) return "octagon";
    if (verticesOut == 3) return "triangle";
    if (verticesOut == 4) return "rectangle";
    if (verticesOut > 6) return "polygon";
    return "unknown";
}

pair<string, int> dominantColor(const Mat& hsvRoi, int sat) {
    Mat r1, r2, r, b, y;
    inRange(hsvRoi, Scalar(0, sat, 50), Scalar(12, 255, 255), r1);
    inRange(hsvRoi, Scalar(168, sat, 50), Scalar(180, 255, 255), r2);
    bitwise_or(r1, r2, r);
    inRange(hsvRoi, Scalar(95, sat, 50), Scalar(145, 255, 255), b);
    inRange(hsvRoi, Scalar(18, sat, 70), Scalar(35, 255, 255), y);

    int total = hsvRoi.rows * hsvRoi.cols;
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

double iou(const Rect& a, const Rect& b) {
    Rect inter = a & b;
    Rect uni = a | b;
    if (uni.area() <= 0) return 0.0;
    return static_cast<double>(inter.area()) / static_cast<double>(uni.area());
}

vector<Detection> mergeDetections(vector<Detection> dets) {
    vector<Detection> merged;
    vector<bool> used(dets.size(), false);

    for (size_t i = 0; i < dets.size(); ++i) {
        if (used[i]) continue;
        Detection best = dets[i];
        used[i] = true;

        for (size_t j = i + 1; j < dets.size(); ++j) {
            if (used[j]) continue;
            if (iou(best.bbox, dets[j].bbox) > 0.35) {
                if (dets[j].color_pct > best.color_pct || dets[j].bbox.area() > best.bbox.area()) {
                    best = dets[j];
                }
                used[j] = true;
            }
        }
        merged.push_back(best);
    }
    return merged;
}

void save(const fs::path& dir, const string& name, const Mat& img) {
    ensureDir(dir);
    imwrite((dir / name).string(), img);
}

struct RunResult {
    int rawContours = 0;
    int areaAccepted = 0;
    int finalDetections = 0;
    double timeMs = 0.0;
};

RunResult processImage(const fs::path& imagePath, const fs::path& outRoot,
                       const Params& params, ofstream& report, ofstream& csv) {
    RunResult rr;
    fs::path imgOut = outRoot / safeStem(imagePath);
    ensureDir(imgOut);

    Mat image = imread(imagePath.string(), IMREAD_COLOR);
    if (image.empty()) {
        report << "[ERROR] Cannot read image: " << imagePath << "\n";
        return rr;
    }

    rr.timeMs = measureMs([&]() {
        Mat blur, hsv, r1, r2, red, blue, yellow, mask, opening, closing;
        Mat contourDebug = Mat::zeros(image.size(), CV_8UC3);
        Mat finalImage = image.clone();

        save(imgOut, "win0_original.png", image);

        GaussianBlur(image, blur, Size(5, 5), 0);
        save(imgOut, "win1_blur.png", blur);

        cvtColor(blur, hsv, COLOR_BGR2HSV);
        save(imgOut, "win2_hsv.png", hsv);

        inRange(hsv, Scalar(0, params.sat, 50), Scalar(12, 255, 255), r1);
        inRange(hsv, Scalar(168, params.sat, 50), Scalar(180, 255, 255), r2);
        bitwise_or(r1, r2, red);
        inRange(hsv, Scalar(95, params.sat, 50), Scalar(145, 255, 255), blue);
        inRange(hsv, Scalar(18, params.sat, 70), Scalar(35, 255, 255), yellow);

        save(imgOut, "win3_red_mask.png", red);
        save(imgOut, "win4_blue_mask.png", blue);
        save(imgOut, "win5_yellow_mask.png", yellow);

        bitwise_or(red, blue, mask);
        bitwise_or(mask, yellow, mask);
        save(imgOut, "win6_combined_mask.png", mask);

        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
        morphologyEx(mask, opening, MORPH_OPEN, kernel);
        morphologyEx(opening, closing, MORPH_CLOSE, kernel);

        save(imgOut, "win7_opening.png", opening);
        save(imgOut, "win8_closing.png", closing);

        vector<vector<Point>> contours;
        findContours(closing, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        rr.rawContours = static_cast<int>(contours.size());

        vector<Detection> detections;

        for (const auto& c : contours) {
            double area = contourArea(c);
            if (area < params.area) continue;
            rr.areaAccepted++;

            Rect bb = boundingRect(c) & Rect(0, 0, image.cols, image.rows);
            if (bb.width <= 5 || bb.height <= 5) continue;

            Mat roi = image(bb).clone();
            Mat hsvRoi, grayRoi;
            cvtColor(roi, hsvRoi, COLOR_BGR2HSV);
            cvtColor(roi, grayRoi, COLOR_BGR2GRAY);

            Detection d;
            d.bbox = bb;
            auto colorResult = dominantColor(hsvRoi, params.sat);
            d.color = colorResult.first;
            d.color_pct = colorResult.second;
            d.shape = inferShape(grayRoi, params, d.circularity, d.vertices, d.v_closest, d.h4);
            d.category = inferCategory(d.color, d.shape);
            detections.push_back(d);

            drawContours(contourDebug, vector<vector<Point>>{c}, -1, Scalar(0, 255, 0), 2);
        }

        save(imgOut, "win9_contours.png", contourDebug);

        detections = mergeDetections(detections);
        rr.finalDetections = static_cast<int>(detections.size());

        for (const auto& d : detections) {
            Scalar c = drawColor(d.color);
            rectangle(finalImage, d.bbox, c, 2);

            string label = d.color + " " + d.shape + " " + to_string(d.color_pct) + "%";
            int baseline = 0;
            Size ts = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            int y0 = max(0, d.bbox.y - ts.height - 8);
            rectangle(finalImage, Point(d.bbox.x, y0),
                      Point(min(finalImage.cols - 1, d.bbox.x + ts.width + 8), d.bbox.y),
                      c, FILLED);
            putText(finalImage, label, Point(d.bbox.x + 4, max(14, d.bbox.y - 5)),
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);

            csv << imagePath.filename().string() << ","
                << params.sat << "," << params.area << "," << params.circ << ","
                << params.rdp << "," << params.hough << ","
                << d.color << "," << d.color_pct << ","
                << d.shape << "," << d.circularity << ","
                << d.vertices << "," << d.v_closest << ","
                << d.h4.h0 << "," << d.h4.h45 << "," << d.h4.h90 << "," << d.h4.h135 << ","
                << d.h4.hother << "," << d.h4.score << ","
                << "\"" << d.category << "\"" << ","
                << d.bbox.x << "," << d.bbox.y << "," << d.bbox.width << "," << d.bbox.height << "\n";
        }

        save(imgOut, "win_final_result.png", finalImage);
        imwrite((outRoot / (safeStem(imagePath) + "_final_result.png")).string(), finalImage);
    });

    report << "Image: " << imagePath << "\n";
    report << "  raw contours       : " << rr.rawContours << "\n";
    report << "  accepted by area   : " << rr.areaAccepted << "\n";
    report << "  final detections   : " << rr.finalDetections << "\n";
    report << "  time               : " << fixed << setprecision(3) << rr.timeMs << " ms\n\n";

    cout << imagePath.filename().string()
         << " | raw=" << rr.rawContours
         << " | area_ok=" << rr.areaAccepted
         << " | final=" << rr.finalDetections
         << " | time=" << fixed << setprecision(3) << rr.timeMs << " ms\n";

    return rr;
}

Params parseArgs(int argc, char** argv) {
    Params p;

    for (int i = 1; i < argc; ++i) {
        string a = argv[i];

        if ((a == "--input" || a == "-i") && i + 1 < argc) p.input = argv[++i];
        else if ((a == "--output" || a == "-o") && i + 1 < argc) p.output = argv[++i];
        else if (a == "--sat" && i + 1 < argc) p.sat = stoi(argv[++i]);
        else if (a == "--area" && i + 1 < argc) p.area = stod(argv[++i]);
        else if (a == "--circ" && i + 1 < argc) p.circ = stod(argv[++i]);
        else if (a == "--rdp" && i + 1 < argc) p.rdp = stod(argv[++i]);
        else if (a == "--hough" && i + 1 < argc) p.hough = stoi(argv[++i]);
        else if (a == "--help" || a == "-h") {
            cout << "Usage:\n";
            cout << "  ./detect_classic_part2.exe --input images/input --output images/output/part2/test "
                 << "--sat 80 --area 300 --circ 0.82 --rdp 0.04 --hough 3\n";
            exit(0);
        }
    }

    return p;
}

int main(int argc, char** argv) {
    Params params = parseArgs(argc, argv);
    ensureDir(params.output);

    vector<fs::path> images = listImages(params.input);
    if (images.empty()) {
        cerr << "[ERROR] No images found in: " << params.input << "\n";
        return 1;
    }

    ofstream report(params.output / "terminal_report_part2.txt");
    ofstream csv(params.output / "detections_part2.csv");

    if (!report || !csv) {
        cerr << "[ERROR] Cannot create report files in: " << params.output << "\n";
        return 1;
    }

    report << "TP3 PART 2 PARAMETER ANALYSIS REPORT\n";
    report << "====================================\n\n";
    report << "Input: " << params.input << "\n";
    report << "Output: " << params.output << "\n";
    report << "sat=" << params.sat << "\n";
    report << "area=" << params.area << "\n";
    report << "circ=" << params.circ << "\n";
    report << "rdp=" << params.rdp << "\n";
    report << "hough=" << params.hough << "\n\n";

    csv << "image,sat,area,circ,rdp,hough,color,color_pct,shape,circularity,"
        << "vertices,v_closest,h0,h45,h90,h135,hother,hscore,category,x,y,w,h\n";

    cout << "TP3 PART 2 PARAMETER ANALYSIS\n";
    cout << "Input : " << params.input << "\n";
    cout << "Output: " << params.output << "\n";
    cout << "sat=" << params.sat
         << " area=" << params.area
         << " circ=" << params.circ
         << " rdp=" << params.rdp
         << " hough=" << params.hough << "\n\n";

    int totalRaw = 0, totalArea = 0, totalFinal = 0;
    double totalMs = 0.0;

    for (const auto& img : images) {
        RunResult r = processImage(img, params.output, params, report, csv);
        totalRaw += r.rawContours;
        totalArea += r.areaAccepted;
        totalFinal += r.finalDetections;
        totalMs += r.timeMs;
    }

    report << "GLOBAL SUMMARY\n";
    report << "==============\n";
    report << "images=" << images.size() << "\n";
    report << "total_raw_contours=" << totalRaw << "\n";
    report << "total_area_accepted=" << totalArea << "\n";
    report << "total_final_detections=" << totalFinal << "\n";
    report << "total_time_ms=" << fixed << setprecision(3) << totalMs << "\n";

    cout << "\nDONE\n";
    cout << "total_raw_contours=" << totalRaw << "\n";
    cout << "total_area_accepted=" << totalArea << "\n";
    cout << "total_final_detections=" << totalFinal << "\n";
    cout << "Report: " << (params.output / "terminal_report_part2.txt") << "\n";
    cout << "CSV   : " << (params.output / "detections_part2.csv") << "\n";
    return 0;
}
