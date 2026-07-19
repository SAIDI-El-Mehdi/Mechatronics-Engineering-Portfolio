#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
using namespace cv;
using namespace cv::dnn;
using namespace std;

// ============================================================
// CONFIG
// ============================================================
const float CONF_THRESHOLD = 0.25f;
const float NMS_THRESHOLD  = 0.35f;
const int   INPUT_SIZE     = 416;

const string MODEL_CFG     = "yolov4-tiny.cfg";
const string MODEL_WEIGHTS = "yolov4-tiny.weights";
const string CLASS_FILE    = "coco.names";

// ============================================================
struct Detection {
    Rect   bbox;
    int    class_id;
    float  confidence;
    string color;
    string color_pct;   // e.g. "68%"
    string shape;
    float  circularity;
    int    vertices;
    string category;
    string source;      // "YOLO" or "Classical"
    bool   validated;

    // Hough4 debug fields
    int h0, h45, h90, h135, hother, hscore, v_closest;
};

// ============================================================
// Helper: compute circularity  4*pi*area / perimeter^2
// ============================================================
static float computeCircularity(const vector<Point>& contour) {
    double area = contourArea(contour);
    double peri = arcLength(contour, true);
    if (peri < 1e-5) return 0.0f;
    return (float)(4.0 * CV_PI * area / (peri * peri));
}

// ============================================================
// Helper: dominant color percentage inside ROI
// ============================================================
static pair<string,int> dominantColorPct(const Mat& hsv) {
    Mat r1,r2,r,b,y;
    inRange(hsv, Scalar(0,  80, 60),  Scalar(12, 255,255), r1);
    inRange(hsv, Scalar(168,80, 60),  Scalar(180,255,255), r2);
    bitwise_or(r1,r2,r);
    inRange(hsv, Scalar(95, 80, 50),  Scalar(145,255,255), b);
    inRange(hsv, Scalar(18, 80, 80),  Scalar(35, 255,255), y);

    int total = hsv.rows * hsv.cols;
    int rp = countNonZero(r);
    int bp = countNonZero(b);
    int yp = countNonZero(y);
    int best = max({rp,bp,yp});

    if (total == 0) return {"unknown", 0};
    int pct = (int)round(100.0 * best / total);

    if (best == rp && rp > bp && rp > yp) return {"red",    pct};
    if (best == bp && bp > rp && bp > yp) return {"blue",   pct};
    if (best == yp && yp > rp && yp > bp) return {"yellow", pct};
    return {"unknown", pct};
}

// ============================================================
// Helper: category from color + shape
// ============================================================
static string inferCategory(const string& color, const string& shape) {
    if (shape == "octagon" && color == "red")    return "STOP Sign";
    if (shape == "triangle" && color == "yellow") return "Warning Sign";
    if (shape == "triangle" && color == "red")    return "Yield / Warning Sign";
    if (shape == "rectangle" && color == "blue")  return "Information / Direction Sign";
    if (shape == "rectangle" && color == "red")   return "Prohibition Sign";
    if (shape == "rectangle" && color == "yellow") return "Caution Sign";
    if (color == "red")   return "Red Sign (Unclassified)";
    if (color == "blue")  return "Blue Sign (Unclassified)";
    if (color == "yellow") return "Yellow Sign (Unclassified)";
    return "Unknown Category";
}

// ============================================================
class Detector {

private:
    Net  net;
    bool yolo_loaded = false;

    // HSV ranges
    Scalar red_lo1 = Scalar(0,  80, 60);
    Scalar red_hi1 = Scalar(12, 255,255);
    Scalar red_lo2 = Scalar(168,80, 60);
    Scalar red_hi2 = Scalar(180,255,255);
    Scalar blue_lo = Scalar(95, 80, 50);
    Scalar blue_hi = Scalar(145,255,255);
    Scalar yel_lo  = Scalar(18, 80, 80);
    Scalar yel_hi  = Scalar(35, 255,255);

    // ======================
    bool loadYOLO() {
        ifstream f(MODEL_CFG);
        if (!f.good()) return false;
        net = readNetFromDarknet(MODEL_CFG, MODEL_WEIGHTS);
        return true;
    }

    // ======================
    string detectColor(const Mat& hsv) {
        return dominantColorPct(hsv).first;
    }

    // ======================
    // Hough4 voting: count lines near 0deg, 45deg, 90deg, 135deg
    // Returns {h0, h45, h90, h135, hother, score}
    struct Hough4 { int h0,h45,h90,h135,hother,score; };

    Hough4 hough4Vote(const Mat& gray) {
        Mat blur, edges;
        GaussianBlur(gray, blur, Size(3,3), 0);
        Canny(blur, edges, 30, 100);

        vector<Vec2f> lines;
        HoughLines(edges, lines, 1, CV_PI/180, 20);

        Hough4 h = {0,0,0,0,0,0};
        for (auto& l : lines) {
            float theta_deg = l[1] * 180.0f / (float)CV_PI;
            if      (theta_deg <  10 || theta_deg > 170) h.h0++;
            else if (theta_deg >= 40 && theta_deg <=  50) h.h45++;
            else if (theta_deg >= 80 && theta_deg <= 100) h.h90++;
            else if (theta_deg >= 130 && theta_deg <= 140) h.h135++;
            else                                           h.hother++;
        }
        // score = number of dominant axes (count > 3)
        h.score = (h.h0>3?1:0)+(h.h45>3?1:0)+(h.h90>3?1:0)+(h.h135>3?1:0);
        return h;
    }

    // ======================
    string detectShape(const Mat& gray, float& circ_out, int& v_out,
                       Hough4& hough_out) {
        Mat blur, edges;
        GaussianBlur(gray, blur, Size(3,3), 0);
        Canny(blur, edges, 30, 100);

        imshow("12. Canny ROI", edges);

        vector<vector<Point>> contours;
        findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        Mat dbg = Mat::zeros(gray.size(), CV_8UC3);
        drawContours(dbg, contours, -1, Scalar(0,255,0), 2);
        imshow("13. Contours ROI", dbg);

        hough_out = hough4Vote(gray);

        if (contours.empty()) { circ_out = 0; v_out = 0; return "unknown"; }

        // Pick largest contour
        auto best = max_element(contours.begin(), contours.end(),
            [](const vector<Point>& a, const vector<Point>& b){
                return contourArea(a) < contourArea(b);
            });

        circ_out = computeCircularity(*best);

        double peri = arcLength(*best, true);
        vector<Point> approx;
        approxPolyDP(*best, approx, 0.04 * peri, true);
        v_out = (int)approx.size();

        // Store v_closest for display (nearest standard polygon)
        hough_out.score   = hough_out.score;   // already set
        // v_closest: nearest polygon count
        // (stored externally in Detection struct)

        if (circ_out > 0.82f)          return "circle";
        if (circ_out > 0.70f && v_out >= 6) return "octagon";
        if (v_out == 3)                return "triangle";
        if (v_out == 4)                return "rectangle";
        if (v_out > 6)                 return "octagon";
        return "polygon";
    }

    // ======================
    vector<Detection> classicalDetect(const Mat& img) {

        vector<Detection> detections;

        Mat blur;
        GaussianBlur(img, blur, Size(5,5), 0);
        imshow("1. Blur", blur);

        Mat hsv;
        cvtColor(blur, hsv, COLOR_BGR2HSV);
        imshow("2. HSV", hsv);

        Mat r1,r2,r,b,y,mask;
        inRange(hsv, red_lo1, red_hi1, r1);
        inRange(hsv, red_lo2, red_hi2, r2);
        bitwise_or(r1,r2,r);
        imshow("3. Red mask", r);

        inRange(hsv, blue_lo, blue_hi, b);
        imshow("4. Blue mask", b);

        inRange(hsv, yel_lo, yel_hi, y);
        imshow("5. Yellow mask", y);

        bitwise_or(r,b,mask);
        bitwise_or(mask,y,mask);
        imshow("6. Combined mask", mask);

        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5,5));
        Mat open, close;
        morphologyEx(mask, open,  MORPH_OPEN,  kernel);
        imshow("7. Opening", open);
        morphologyEx(open,  close, MORPH_CLOSE, kernel);
        imshow("8. Closing", close);

        vector<vector<Point>> contours;
        findContours(close, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        Mat contour_img = Mat::zeros(img.size(), CV_8UC3);
        drawContours(contour_img, contours, -1, Scalar(0,255,0), 2);
        imshow("9. Contours", contour_img);

        cout << "[INFO] Classical candidates: " << contours.size() << "\n";

        int idx = 0;
        for (auto& c : contours) {
            double area = contourArea(c);
            if (area < 300) continue;

            Rect bb = boundingRect(c);
            Mat roi = img(bb);
            imshow("10. ROI", roi);

            Mat hsv_roi;
            cvtColor(roi, hsv_roi, COLOR_BGR2HSV);
            Mat gray;
            cvtColor(roi, gray, COLOR_BGR2GRAY);

            float circ   = 0;
            int   verts  = 0;
            Hough4 h4    = {};
            string shape = detectShape(gray, circ, verts, h4);

            pair<string,int> colorResult = dominantColorPct(hsv_roi);
            string colorName = colorResult.first;
            int    pct       = colorResult.second;

            // v_closest: nearest "standard" polygon vertex count
            static const int stdV[] = {3,4,5,6,8,12};
            int v_closest = 3;
            int best_dist = 999;
            for (int sv : stdV) {
                if (abs(sv - verts) < best_dist) {
                    best_dist = abs(sv - verts);
                    v_closest = sv;
                }
            }

            // Print Hough4 line
            cout << "    [Hough4] "
                 << "0deg=" << h4.h0
                 << " 45deg=" << h4.h45
                 << " 90deg=" << h4.h90
                 << " 135deg=" << h4.h135
                 << " other=" << h4.hother
                 << "  score=" << h4.score
                 << "  v_closest=" << v_closest
                 << "  circ=" << fixed << setprecision(2) << circ
                 << "  --> " << shape << "\n";

            Detection d;
            d.bbox        = bb;
            d.class_id    = -1;
            d.confidence  = 0.8f;
            d.color       = colorName;
            d.color_pct   = to_string(pct) + "%";
            d.shape       = shape;
            d.circularity = circ;
            d.vertices    = verts;
            d.category    = inferCategory(colorName, shape);
            d.source      = "Classical";
            d.validated   = false;
            d.h0 = h4.h0; d.h45 = h4.h45; d.h90 = h4.h90;
            d.h135 = h4.h135; d.hother = h4.hother;
            d.hscore = h4.score; d.v_closest = v_closest;

            detections.push_back(d);
            idx++;
        }

        // Hough Circle detection
        cout << "[INFO] Running Hough Circle detection...\n";
        Mat gray_full;
        cvtColor(blur, gray_full, COLOR_BGR2GRAY);
        vector<Vec3f> circles;
        HoughCircles(gray_full, circles, HOUGH_GRADIENT, 1, 30, 100, 30, 10, 200);
        cout << "[INFO] Hough Circles found: " << circles.size() << "\n";

        return detections;
    }

public:
    Detector() {
        yolo_loaded = loadYOLO();
        if (!yolo_loaded)
            cout << "[WARN] Fallback to classical HSV detection...\n";
    }

    pair<Mat, vector<Detection>> detect(const Mat& img) {

        cout << "\nDetecting...\n";

        imshow("0. Original", img);

        vector<Detection> dets = classicalDetect(img);

        // ---- Active COCO classes (static list shown when YOLO absent) ----
        cout << string(65,'=') << "\n";
        cout << "  " << dets.size() << " sign(s) detected\n";
        cout << string(65,'=') << "\n";

        if (!yolo_loaded) {
            cout << "[Active COCO classes]\n";
            cout << "  ID  9 : traffic light\n";
            cout << "  ID 11 : stop sign\n";
            cout << "  ID 12 : parking meter\n";
            cout << string(65,'-') << "\n";
        }

        Mat output = img.clone();

        for (int i = 0; i < (int)dets.size(); i++) {
            auto& d = dets[i];

            cout << "Sign " << (i+1) << ":\n";
            cout << "  Source         : " << d.source << "\n";
            if (d.class_id >= 0)
                cout << "  COCO Class     : [" << d.class_id << "] sign\n";
            else
                cout << "  COCO Class     : [N/A] classical\n";
            cout << "  Confidence     : "
                 << fixed << setprecision(1) << (d.confidence*100.0f) << "%\n";
            cout << "  Category       : " << d.category << "\n";
            cout << "  Color          : " << d.color << " (" << d.color_pct << ")\n";
            cout << "  Shape          : " << d.shape
                 << " (circularity=" << fixed << setprecision(2) << d.circularity << ")\n";
            cout << "  Validated      : " << (d.validated ? "YES" : "NO") << "\n";
            cout << "  BBox           : ["
                 << d.bbox.x << ", " << d.bbox.y << ", "
                 << d.bbox.width << ", " << d.bbox.height << "]\n";

            // Draw on output
            Scalar box_color(0, 255, 0);
            if      (d.color == "red")    box_color = Scalar(0, 0, 255);
            else if (d.color == "blue")   box_color = Scalar(255, 100, 0);
            else if (d.color == "yellow") box_color = Scalar(0, 220, 255);

            rectangle(output, d.bbox, box_color, 2);

            string label = d.color + " | " + d.shape
                         + " (" + d.color_pct + ")";
            int baseline = 0;
            Size ts = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            rectangle(output,
                      Point(d.bbox.x, d.bbox.y - ts.height - 6),
                      Point(d.bbox.x + ts.width, d.bbox.y),
                      box_color, FILLED);
            putText(output, label,
                    Point(d.bbox.x, d.bbox.y - 4),
                    FONT_HERSHEY_SIMPLEX, 0.5,
                    Scalar(255,255,255), 1);
        }

        // Save result
        imwrite("result_signs.jpg", output);
        cout << "Result saved: result_signs.jpg\n";

        return {output, dets};
    }
};

// ============================================================
int main() {

    Mat image = imread("3.png");
    if (image.empty()) {
        cout << "Error loading image\n";
        return -1;
    }

    Detector detector;

    auto result = detector.detect(image);

    imshow("FINAL RESULT", result.first);
    waitKey(0);
    destroyAllWindows();

    return 0;
}