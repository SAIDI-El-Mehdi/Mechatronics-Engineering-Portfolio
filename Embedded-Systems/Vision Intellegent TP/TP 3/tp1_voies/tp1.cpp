#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

// Structure to hold lane detection results
struct LaneDetectionResult {
    vector<double> left_fit;
    vector<double> right_fit;
    vector<Point2f> left_points;
    vector<Point2f> right_points;
    bool valid;
};

// Function to detect lanes using sliding window method
LaneDetectionResult detectLanes(const Mat& binary_warped) {
    LaneDetectionResult result;
    result.valid = false;
    
    // Calculate histogram of bottom half
    Mat bottom_half = binary_warped(Rect(0, binary_warped.rows/2, 
                                         binary_warped.cols, binary_warped.rows/2));
    Mat histogram;
    reduce(bottom_half, histogram, 0, REDUCE_SUM, CV_32S);
    
    // Find peak positions for left and right lanes
    int midpoint = histogram.cols / 2;
    Point leftMaxLoc, rightMaxLoc;
    minMaxLoc(histogram(Rect(0, 0, midpoint, 1)), nullptr, nullptr, nullptr, &leftMaxLoc);
    minMaxLoc(histogram(Rect(midpoint, 0, histogram.cols - midpoint, 1)), 
              nullptr, nullptr, nullptr, &rightMaxLoc);
    
    int leftx_base = leftMaxLoc.x;
    int rightx_base = rightMaxLoc.x + midpoint;
    
    // Sliding window parameters
    int nwindows = 9;
    int window_height = binary_warped.rows / nwindows;
    int margin = 100;
    int minpix = 50;
    
    // Get non-zero pixel positions
    vector<Point> nonzero_points;
    findNonZero(binary_warped, nonzero_points);
    
    // Current window positions
    int leftx_current = leftx_base;
    int rightx_current = rightx_base;
    
    // Store lane pixel indices
    vector<Point> left_lane_pixels;
    vector<Point> right_lane_pixels;
    
    // Iterate through windows
    for (int window = 0; window < nwindows; window++) {
        int win_y_low = binary_warped.rows - (window + 1) * window_height;
        int win_y_high = binary_warped.rows - window * window_height;
        
        int win_xleft_low = leftx_current - margin;
        int win_xleft_high = leftx_current + margin;
        int win_xright_low = rightx_current - margin;
        int win_xright_high = rightx_current + margin;
        
        // Find pixels within windows
        vector<Point> good_left_points, good_right_points;
        
        for (const Point& pt : nonzero_points) {
            if (pt.y >= win_y_low && pt.y < win_y_high) {
                if (pt.x >= win_xleft_low && pt.x < win_xleft_high) {
                    good_left_points.push_back(pt);
                }
                if (pt.x >= win_xright_low && pt.x < win_xright_high) {
                    good_right_points.push_back(pt);
                }
            }
        }
        
        // Add to lane pixels
        left_lane_pixels.insert(left_lane_pixels.end(), 
                               good_left_points.begin(), good_left_points.end());
        right_lane_pixels.insert(right_lane_pixels.end(), 
                                good_right_points.begin(), good_right_points.end());
        
        // Recenter window if enough pixels found
        if (good_left_points.size() > minpix) {
            int sum_x = 0;
            for (const Point& pt : good_left_points) sum_x += pt.x;
            leftx_current = sum_x / good_left_points.size();
        }
        if (good_right_points.size() > minpix) {
            int sum_x = 0;
            for (const Point& pt : good_right_points) sum_x += pt.x;
            rightx_current = sum_x / good_right_points.size();
        }
    }
    
    // Check if we have enough points
    if (left_lane_pixels.empty() || right_lane_pixels.empty()) {
        return result;
    }
    
    // Prepare data for polynomial fitting
    vector<Point2f> left_pts, right_pts;
    for (const Point& pt : left_lane_pixels) {
        left_pts.push_back(Point2f(pt.y, pt.x));
    }
    for (const Point& pt : right_lane_pixels) {
        right_pts.push_back(Point2f(pt.y, pt.x));
    }
    
    // Fit 2nd degree polynomial: x = Ay^2 + By + C
    Mat left_coeffs, right_coeffs;
    
    // Create matrices for polynomial fitting
    Mat left_A(left_pts.size(), 3, CV_32F);
    Mat left_b(left_pts.size(), 1, CV_32F);
    for (size_t i = 0; i < left_pts.size(); i++) {
        float y = left_pts[i].x;
        left_A.at<float>(i, 0) = y * y;
        left_A.at<float>(i, 1) = y;
        left_A.at<float>(i, 2) = 1.0f;
        left_b.at<float>(i, 0) = left_pts[i].y;
    }
    solve(left_A, left_b, left_coeffs, DECOMP_SVD);
    
    Mat right_A(right_pts.size(), 3, CV_32F);
    Mat right_b(right_pts.size(), 1, CV_32F);
    for (size_t i = 0; i < right_pts.size(); i++) {
        float y = right_pts[i].x;
        right_A.at<float>(i, 0) = y * y;
        right_A.at<float>(i, 1) = y;
        right_A.at<float>(i, 2) = 1.0f;
        right_b.at<float>(i, 0) = right_pts[i].y;
    }
    solve(right_A, right_b, right_coeffs, DECOMP_SVD);
    
    // Store coefficients
    result.left_fit = {left_coeffs.at<float>(0), left_coeffs.at<float>(1), 
                       left_coeffs.at<float>(2)};
    result.right_fit = {right_coeffs.at<float>(0), right_coeffs.at<float>(1), 
                        right_coeffs.at<float>(2)};
    
    // Generate points along the polynomial
    for (int y = 0; y < binary_warped.rows; y++) {
        float left_x = result.left_fit[0] * y * y + result.left_fit[1] * y + result.left_fit[2];
        float right_x = result.right_fit[0] * y * y + result.right_fit[1] * y + result.right_fit[2];
        result.left_points.push_back(Point2f(left_x, y));
        result.right_points.push_back(Point2f(right_x, y));
    }
    
    result.valid = true;
    return result;
}

int main() {
    // ================================================
    // 1. Load and prepare image
    // ================================================
    Mat image = imread("6.png");
    if (image.empty()) {
        cerr << "Error: Could not load image '6.png'" << endl;
        return -1;
    }
    
    int height = image.rows;
    int width = image.cols;
    
    Mat image_rgb;
    cvtColor(image, image_rgb, COLOR_BGR2RGB);
    
    // ================================================
    // 2. Color filtering in HSV space
    // ================================================
    Mat hsv;
    cvtColor(image, hsv, COLOR_BGR2HSV);
    
    // White mask
    Scalar lower_white(0, 0, 200);
    Scalar upper_white(180, 50, 255);
    Mat mask_white;
    inRange(hsv, lower_white, upper_white, mask_white);
    
    // Yellow mask
    Scalar lower_yellow(15, 60, 100);
    Scalar upper_yellow(35, 255, 255);
    Mat mask_yellow;
    inRange(hsv, lower_yellow, upper_yellow, mask_yellow);
    
    // Combine masks
    Mat color_mask;
    bitwise_or(mask_white, mask_yellow, color_mask);
    
    // ================================================
    // 3. Edge detection
    // ================================================
    Mat gray, blur, edges;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blur, Size(5, 5), 0);
    Canny(blur, edges, 50, 150);
    
    // Combine edges with color mask
    Mat combined;
    bitwise_and(edges, edges, combined, color_mask);
    
    // ================================================
    // 4. Perspective transformation (Bird's Eye View)
    // ================================================
    vector<Point2f> src_pts = {
        Point2f(width * 0.15f, height),
        Point2f(width * 0.45f, height * 0.60f),
        Point2f(width * 0.55f, height * 0.60f),
        Point2f(width * 0.85f, height)
    };
    
    vector<Point2f> dst_pts = {
        Point2f(0, height),
        Point2f(0, 0),
        Point2f(width, 0),
        Point2f(width, height)
    };
    
    Mat M = getPerspectiveTransform(src_pts, dst_pts);
    Mat Minv = getPerspectiveTransform(dst_pts, src_pts);
    
    Mat warped;
    warpPerspective(combined, warped, M, Size(width, height));
    
    // ================================================
    // 5. Lane detection with sliding windows
    // ================================================
    LaneDetectionResult lanes = detectLanes(warped);
    
    // ================================================
    // 6. Visualize detected lanes
    // ================================================
    Mat lane_warp = Mat::zeros(warped.size(), CV_8UC3);
    
    if (lanes.valid) {
        // Convert points to integer format
        vector<Point> left_pts_int, right_pts_int;
        for (const Point2f& pt : lanes.left_points) {
            left_pts_int.push_back(Point(cvRound(pt.x), cvRound(pt.y)));
        }
        for (const Point2f& pt : lanes.right_points) {
            right_pts_int.push_back(Point(cvRound(pt.x), cvRound(pt.y)));
        }
        
        // Draw lane lines
        polylines(lane_warp, left_pts_int, false, Scalar(0, 255, 0), 25);
        polylines(lane_warp, right_pts_int, false, Scalar(0, 255, 0), 25);
        
        // Fill lane area
        vector<Point> pts;
        pts.insert(pts.end(), left_pts_int.begin(), left_pts_int.end());
        pts.insert(pts.end(), right_pts_int.rbegin(), right_pts_int.rend());
        vector<vector<Point>> fillPts = {pts};
        fillPoly(lane_warp, fillPts, Scalar(0, 255, 0));
    }
    
    // Warp back to original perspective
    Mat lane_original;
    warpPerspective(lane_warp, lane_original, Minv, Size(width, height));
    
    // Overlay on original image
    Mat result;
    addWeighted(image_rgb, 1.0, lane_original, 0.5, 0, result);
    
    // ================================================
    // 7. Calculate curvature and vehicle offset
    // ================================================
    if (lanes.valid) {
        double ym_per_pix = 30.0 / 720.0;
        double xm_per_pix = 3.7 / 700.0;
        
        // Convert coefficients to meters
        vector<double> left_fit_cr = {
            lanes.left_fit[0] * xm_per_pix / (ym_per_pix * ym_per_pix),
            lanes.left_fit[1] * xm_per_pix / ym_per_pix,
            lanes.left_fit[2] * xm_per_pix
        };
        
        vector<double> right_fit_cr = {
            lanes.right_fit[0] * xm_per_pix / (ym_per_pix * ym_per_pix),
            lanes.right_fit[1] * xm_per_pix / ym_per_pix,
            lanes.right_fit[2] * xm_per_pix
        };
        
        double y_eval = (warped.rows - 1) * ym_per_pix;
        
        // Calculate curvature radius
        double left_curv = pow(1 + pow(2 * left_fit_cr[0] * y_eval + left_fit_cr[1], 2), 1.5) 
                          / abs(2 * left_fit_cr[0]);
        double right_curv = pow(1 + pow(2 * right_fit_cr[0] * y_eval + right_fit_cr[1], 2), 1.5) 
                           / abs(2 * right_fit_cr[0]);
        
        double radius = (left_curv + right_curv) / 2.0;
        
        // Calculate vehicle offset
        double lane_center = (lanes.left_points.back().x + lanes.right_points.back().x) / 2.0;
        double vehicle_center = warped.cols / 2.0;
        double offset_pix = (vehicle_center - lane_center) * xm_per_pix;
        
        string offset_str = to_string(abs(offset_pix)).substr(0, 4) + " m " + 
                           (offset_pix > 0 ? "a gauche" : "a droite");
        
        // Display information on result image
        putText(result, "Rayon de courbure: " + to_string(int(radius)) + " m",
                Point(50, 60), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(255, 255, 255), 3);
        putText(result, "Decalage vehicule: " + offset_str,
                Point(50, 120), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(255, 255, 255), 3);
    }
    
    // ================================================
    // 8. Display results
    // ================================================
    namedWindow("1. Original Image", WINDOW_NORMAL);
    imshow("1. Original Image", image_rgb);
    
    namedWindow("2. Color Mask", WINDOW_NORMAL);
    imshow("2. Color Mask", color_mask);
    
    namedWindow("3. Edges + Color Mask", WINDOW_NORMAL);
    imshow("3. Edges + Color Mask", combined);
    
    namedWindow("4. Bird's Eye View", WINDOW_NORMAL);
    imshow("4. Bird's Eye View", warped);
    
    namedWindow("5. Detected Lanes (Bird's Eye)", WINDOW_NORMAL);
    imshow("5. Detected Lanes (Bird's Eye)", lane_warp);
    
    namedWindow("6. Final Result", WINDOW_NORMAL);
    imshow("6. Final Result", result);
    
    cout << "Press any key to exit..." << endl;
    waitKey(0);
    
    return 0;
}