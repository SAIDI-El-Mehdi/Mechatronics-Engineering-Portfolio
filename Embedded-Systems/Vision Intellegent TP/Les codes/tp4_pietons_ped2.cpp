#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace cv::dnn;
using namespace std;

// ------------------------------------------------
//  CONFIGURATION
// ------------------------------------------------
const string MODEL_CFG     = "yolov4-tiny.cfg";
const string MODEL_WEIGHTS = "yolov4-tiny.weights";

const float CONF_THRESHOLD = 0.35f;
const float NMS_THRESHOLD  = 0.40f;

const int INPUT_WIDTH  = 416;
const int INPUT_HEIGHT = 416;

// ------------------------------------------------
//  HELPERS
// ------------------------------------------------
Mat getCannyEdges(const Mat& roi) {
    if (roi.empty()) return Mat();
    Mat gray, blurred, edges;
    cvtColor(roi, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(5, 5), 0);
    Canny(blurred, edges, 60, 180);
    return edges;
}

bool hasStrongVerticalStructure(const Mat& roi, double minVerticalRatio = 0.30) {
    Mat edges = getCannyEdges(roi);
    if (edges.empty()) return false;

    vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI/180, 50, 35, 12);

    if (lines.empty()) return false;

    int verticalCount = 0;
    for (const auto& l : lines) {
        int dx = abs(l[2] - l[0]);
        int dy = abs(l[3] - l[1]);
        if (dy > dx * 2.5) verticalCount++;  // quite vertical
    }

    double verticalRatio = static_cast<double>(verticalCount) / lines.size();
    return verticalRatio >= minVerticalRatio;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "\nUsage:\n";
        cout << "  " << argv[0] << " <input_image.jpg> [output_image.jpg]\n\n";
        cout << "Examples:\n";
        cout << "  " << argv[0] << " input.jpg\n";
        cout << "  " << argv[0] << " input.jpg output.jpg\n\n";
        cout << "Required files (place in same folder):\n";
        cout << "  yolov4-tiny.cfg\n";
        cout << "  yolov4-tiny.weights\n\n";
        return -1;
    }

    string inputPath = argv[1];
    string outputPath = (argc >= 3) ? argv[2] : "output_detected.jpg";

    // --- Load input image --------------------------------
    cout << "Reading image: " << inputPath << "\n";
    Mat img = imread(inputPath);
    
    if (img.empty()) {
        cerr << "ERROR: Cannot read image: " << inputPath << endl;
        return -1;
    }
    
    cout << "Image size: " << img.cols << "x" << img.rows << "\n";

    // --- Load YOLO ---------------------------------------
    cout << "Loading YOLOv4-tiny model...\n";
    Net net = readNetFromDarknet(MODEL_CFG, MODEL_WEIGHTS);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    vector<String> outNames = net.getUnconnectedOutLayersNames();

    // --- Prepare blob -------------------------------------
    cout << "Detecting pedestrians...\n";
    Mat blob = blobFromImage(img, 1/255.0, Size(INPUT_WIDTH, INPUT_HEIGHT),
                             Scalar(0,0,0), true, false);
    net.setInput(blob);

    // --- Forward pass -------------------------------------
    vector<Mat> outputs;
    net.forward(outputs, outNames);

    // --- Post-processing ----------------------------------
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;

    for (size_t i = 0; i < outputs.size(); ++i) {
        float* data = (float*)outputs[i].data;
        for (int row = 0; row < outputs[i].rows; ++row, data += outputs[i].cols) {
            Mat scores = outputs[i].row(row).colRange(5, outputs[i].cols);
            Point classIdPoint;
            double conf;
            minMaxLoc(scores, nullptr, &conf, nullptr, &classIdPoint);

            if (conf > CONF_THRESHOLD && classIdPoint.x == 0) { // person = class 0
                float centerX = data[0] * img.cols;
                float centerY = data[1] * img.rows;
                float w       = data[2] * img.cols;
                float h       = data[3] * img.rows;

                int left = max(0, (int)(centerX - w/2));
                int top  = max(0, (int)(centerY - h/2));

                classIds.push_back(0);
                confidences.push_back(static_cast<float>(conf));
                boxes.emplace_back(left, top, (int)w, (int)h);
            }
        }
    }

    // --- NMS ----------------------------------------------
    vector<int> indices;
    NMSBoxes(boxes, confidences, CONF_THRESHOLD, NMS_THRESHOLD, indices);

    cout << "\n========================================\n";
    cout << "Found " << indices.size() << " pedestrian(s)\n";
    cout << "========================================\n";

    // --- Draw results -------------------------------------
    Mat result = img.clone();
    
    for (size_t i = 0; i < indices.size(); i++) {
        int idx = indices[i];
        Rect box = boxes[idx];
        float conf = confidences[idx];

        // Make sure box is within image bounds
        box.x = max(0, box.x);
        box.y = max(0, box.y);
        box.width = min(box.width, img.cols - box.x);
        box.height = min(box.height, img.rows - box.y);

        // Crop person region and check classical edges
        Mat roi = img(box);
        bool classical_ok = hasStrongVerticalStructure(roi);

        Scalar color = classical_ok ? Scalar(0, 220, 30)   // green = confirmed
                                    : Scalar(0, 140, 255); // orange = DL only

        rectangle(result, box, color, 3);

        string label = "Person " + to_string(i + 1) + "  " + 
                      to_string(int(conf * 100 + 0.5)) + "%";
        if (!classical_ok) label += "  (weak edges)";

        int baseline = 0;
        Size textSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.65, 2, &baseline);
        Point textOrg(box.x, box.y - 8);

        rectangle(result, textOrg + Point(0, baseline),
                  textOrg + Point(textSize.width, -textSize.height - baseline),
                  color, FILLED);

        putText(result, label, textOrg,
                FONT_HERSHEY_SIMPLEX, 0.65, Scalar(255,255,255), 2);
        
        // Console output
        cout << "Person " << (i+1) << ":\n";
        cout << "  Confidence: " << int(conf*100) << "%\n";
        cout << "  Position: (" << box.x << ", " << box.y << ")\n";
        cout << "  Size: " << box.width << "x" << box.height << "\n";
        cout << "  Edge verification: " << (classical_ok ? "STRONG (confirmed)" : "WEAK") << "\n\n";
    }

    // --- Save output image --------------------------------
    imwrite(outputPath, result);
    cout << "Output saved to: " << outputPath << "\n";

    // --- Display results ----------------------------------
    cout << "\nDisplaying result. Press any key to exit...\n";
    imshow("Pedestrian Detection Result", result);
    waitKey(0);
    destroyAllWindows();

    cout << "\nDone!\n";
    return 0;
}