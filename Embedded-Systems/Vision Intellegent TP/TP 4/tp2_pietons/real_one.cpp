#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

using namespace cv;
using namespace cv::dnn;
using namespace std;

// ────────────────────────────────────────────────
//  CONFIGURATION
// ────────────────────────────────────────────────
const string MODEL_CFG     = "yolov4-tiny.cfg";
const string MODEL_WEIGHTS = "yolov4-tiny.weights";

const float CONF_THRESHOLD = 0.35f;
const float NMS_THRESHOLD  = 0.40f;

const int INPUT_WIDTH  = 416;
const int INPUT_HEIGHT = 416;

// Seuils pour la validation Sobel
const float SOBEL_MAGNITUDE_THRESHOLD = 50.0f;
const float VERTICAL_RATIO_THRESHOLD = 0.30f;
const float VERTICAL_DOMINANCE_THRESHOLD = 0.55f;

// ────────────────────────────────────────────────
//  FONCTIONS DE VALIDATION PAR SOBEL
// ────────────────────────────────────────────────

/**
 * Calcule la magnitude des gradients avec Sobel
 * Retourne une image 8U avec les magnitudes normalisées
 */
Mat getSobelMagnitude(const Mat& roi) {
    if (roi.empty()) return Mat();
    
    Mat gray, grad_x, grad_y, magnitude;
    
    // Conversion en niveaux de gris
    cvtColor(roi, gray, COLOR_BGR2GRAY);
    
    // Flou gaussien pour réduire le bruit
    GaussianBlur(gray, gray, Size(3, 3), 0.5);
    
    // Calcul des gradients avec Sobel (profondeur 16S pour éviter débordement)
    Sobel(gray, grad_x, CV_16S, 1, 0, 3);
    Sobel(gray, grad_y, CV_16S, 0, 1, 3);
    
    // Conversion en valeurs absolues
    Mat abs_grad_x, abs_grad_y;
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    
    // Calcul de la magnitude (moyenne pondérée)
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, magnitude);
    
    return magnitude;
}

/**
 * Version 1: Validation basée sur l'analyse des orientations des gradients
 * Utilise cartToPolar pour obtenir les angles précis
 */
bool validateWithOrientationAnalysis(const Mat& roi) {
    if (roi.empty() || roi.rows < 20 || roi.cols < 10) return false;
    
    Mat gray;
    cvtColor(roi, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 0.5);
    
    // Calcul des gradients avec Sobel (en flottant pour précision)
    Mat grad_x, grad_y;
    Sobel(gray, grad_x, CV_32F, 1, 0, 3);
    Sobel(gray, grad_y, CV_32F, 0, 1, 3);
    
    // Calcul de la magnitude et de l'orientation
    Mat magnitude, angle;
    cartToPolar(grad_x, grad_y, magnitude, angle, true); // true pour degrés
    
    // Seuillage sur la magnitude pour ne garder que les gradients significatifs
    Mat significant_mask;
    threshold(magnitude, significant_mask, SOBEL_MAGNITUDE_THRESHOLD, 255, THRESH_BINARY);
    significant_mask.convertTo(significant_mask, CV_8U);
    
    int total_significant = countNonZero(significant_mask);
    if (total_significant < 50) return false; // Pas assez de pixels de contour
    
    // Compter les pixels avec orientation verticale
    int vertical_count = 0;
    for (int y = 0; y < angle.rows; y++) {
        for (int x = 0; x < angle.cols; x++) {
            if (significant_mask.at<uchar>(y, x) > 0) {
                float ang = angle.at<float>(y, x);
                // Un contour vertical produit un gradient horizontal (~0° ou 180°)
                // Mais pour un piéton, on s'intéresse aux contours verticaux des jambes/buste
                // Donc on cherche des gradients horizontaux (perpendiculaires aux contours)
                if (ang < 20 || ang > 160) { // Gradients proches de l'horizontal
                    vertical_count++;
                }
            }
        }
    }
    
    double vertical_ratio = static_cast<double>(vertical_count) / total_significant;
    return vertical_ratio >= VERTICAL_RATIO_THRESHOLD;
}

/**
 * Version 2: Validation simplifiée basée sur la dominance verticale
 * Utilise le ratio des sommes de gradients
 */
bool validateWithVerticalDominance(const Mat& roi) {
    if (roi.empty() || roi.rows < 20 || roi.cols < 10) return false;
    
    Mat gray;
    cvtColor(roi, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 0.5);
    
    // Calcul des gradients
    Mat grad_x, grad_y;
    Sobel(gray, grad_x, CV_16S, 1, 0, 3);
    Sobel(gray, grad_y, CV_16S, 0, 1, 3);
    
    // Conversion en valeurs absolues
    Mat abs_grad_x, abs_grad_y;
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    
    // Somme des gradients
    Scalar sum_x = sum(abs_grad_x);
    Scalar sum_y = sum(abs_grad_y);
    
    double total_grad = sum_x[0] + sum_y[0];
    if (total_grad < 1000) return false; // Pas assez de gradients
    
    double vertical_dominance = sum_y[0] / total_grad;
    
    // Un piéton a généralement plus de contours verticaux
    // donc plus de gradients horizontaux (perpendiculaires)
    double horizontal_dominance = sum_x[0] / total_grad;
    
    // On cherche un équilibre (ni trop vertical, ni trop horizontal)
    // Les piétons ont un mélange de contours horizontaux et verticaux
    return (horizontal_dominance > 0.4 && horizontal_dominance < 0.7);
}

/**
 * Version 3: Validation par analyse de la texture verticale
 * Utilise des filtres Sobel et des statistiques de voisinage
 */
bool validateWithVerticalTexture(const Mat& roi) {
    if (roi.empty() || roi.rows < 20 || roi.cols < 10) return false;
    
    Mat gray;
    cvtColor(roi, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 0.5);
    
    // Calcul des gradients renforcés verticalement
    Mat grad_y;
    Sobel(gray, grad_y, CV_16S, 0, 2, 3); // Dérivée seconde verticale
    convertScaleAbs(grad_y, grad_y);
    
    // Moyenne et écart-type des gradients verticaux
    Scalar mean, stddev;
    meanStdDev(grad_y, mean, stddev);
    
    // Une région avec un piéton a une certaine régularité dans les gradients verticaux
    // (alternance jambes/fond, bras/corps)
    return (stddev[0] > 15 && stddev[0] < 80);
}

/**
 * Fonction principale de validation combinant plusieurs approches Sobel
 */
bool hasStrongVerticalStructure(const Mat& roi) {
    if (roi.empty()) return false;
    
    // Combinaison des trois méthodes pour plus de robustesse
    bool orientation_ok = validateWithOrientationAnalysis(roi);
    bool dominance_ok = validateWithVerticalDominance(roi);
    bool texture_ok = validateWithVerticalTexture(roi);
    
    // Au moins 2 méthodes sur 3 doivent réussir
    int success_count = (orientation_ok ? 1 : 0) + 
                        (dominance_ok ? 1 : 0) + 
                        (texture_ok ? 1 : 0);
    
    return success_count >= 2;
}

/**
 * Fonction de debug pour visualiser les gradients Sobel
 */
Mat visualizeSobelGradients(const Mat& roi) {
    Mat gray, grad_x, grad_y, grad_x_abs, grad_y_abs;
    
    cvtColor(roi, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 0.5);
    
    // Calcul des gradients
    Sobel(gray, grad_x, CV_16S, 1, 0, 3);
    Sobel(gray, grad_y, CV_16S, 0, 1, 3);
    
    convertScaleAbs(grad_x, grad_x_abs);
    convertScaleAbs(grad_y, grad_y_abs);
    
    // Création d'une image couleur pour visualisation
    Mat visualization = Mat::zeros(roi.size(), CV_8UC3);
    
    // Canal B = gradients horizontaux, Canal R = gradients verticaux
    vector<Mat> channels(3);
    channels[0] = grad_x_abs; // Bleu
    channels[1] = Mat::zeros(roi.size(), CV_8U); // Vert
    channels[2] = grad_y_abs; // Rouge
    
    merge(channels, visualization);
    
    return visualization;
}

// ────────────────────────────────────────────────
//  FONCTION PRINCIPALE
// ────────────────────────────────────────────────
int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "\n╔════════════════════════════════════════════════╗\n";
        cout << "║     DÉTECTION DE PIÉTONS AVEC VALIDATION SOBEL ║\n";
        cout << "╚════════════════════════════════════════════════╝\n\n";
        cout << "Usage:\n";
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

    // ─── Load input image ────────────────────────────────
    cout << "\n[1] Lecture de l'image: " << inputPath << "\n";
    Mat img = imread(inputPath);
    
    if (img.empty()) {
        cerr << "ERREUR: Impossible de lire l'image: " << inputPath << endl;
        return -1;
    }
    
    cout << "    Taille: " << img.cols << "x" << img.rows << "\n";
    cout << "    Canaux: " << img.channels() << "\n";

    // ─── Load YOLO ───────────────────────────────────────
    cout << "\n[2] Chargement du modèle YOLOv4-tiny...\n";
    Net net = readNetFromDarknet(MODEL_CFG, MODEL_WEIGHTS);
    
    if (net.empty()) {
        cerr << "ERREUR: Impossible de charger le modèle YOLO\n";
        cerr << "Vérifiez que les fichiers existent:\n";
        cerr << "  - " << MODEL_CFG << "\n";
        cerr << "  - " << MODEL_WEIGHTS << "\n";
        return -1;
    }
    
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    vector<String> outNames = net.getUnconnectedOutLayersNames();
    cout << "    " << outNames.size() << " couches de sortie\n";

    // ─── Prepare blob ─────────────────────────────────────
    cout << "\n[3] Préparation du blob d'entrée...\n";
    Mat blob = blobFromImage(img, 1/255.0, Size(INPUT_WIDTH, INPUT_HEIGHT),
                             Scalar(0,0,0), true, false);
    net.setInput(blob);
    cout << "    Taille blob: " << INPUT_WIDTH << "x" << INPUT_HEIGHT << "\n";

    // ─── Forward pass ─────────────────────────────────────
    cout << "\n[4] Inférence YOLO en cours...\n";
    vector<Mat> outputs;
    net.forward(outputs, outNames);
    cout << "    Inférence terminée\n";

    // ─── Post-processing ──────────────────────────────────
    cout << "\n[5] Extraction des détections...\n";
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
                
                // S'assurer que le rectangle reste dans l'image
                int width = min((int)w, img.cols - left);
                int height = min((int)h, img.rows - top);

                classIds.push_back(0);
                confidences.push_back(static_cast<float>(conf));
                boxes.emplace_back(left, top, width, height);
            }
        }
    }
    
    cout << "    Détections brutes: " << boxes.size() << "\n";

    // ─── NMS ──────────────────────────────────────────────
    cout << "\n[6] Application NMS (Non-Maximum Suppression)...\n";
    vector<int> indices;
    NMSBoxes(boxes, confidences, CONF_THRESHOLD, NMS_THRESHOLD, indices);

    cout << "========================================\n";
    cout << "RÉSULTATS:\n";
    cout << "========================================\n";
    cout << "Piétons détectés: " << indices.size() << "\n";

    // ─── Draw results ─────────────────────────────────────
    Mat result = img.clone();
    Mat debug_sobel = Mat::zeros(img.size(), CV_8UC3);
    
    for (size_t i = 0; i < indices.size(); i++) {
        int idx = indices[i];
        Rect box = boxes[idx];
        float conf = confidences[idx];

        // Extraction ROI et validation Sobel
        Mat roi = img(box);
        bool classical_ok = hasStrongVerticalStructure(roi);
        
        // Visualisation des gradients Sobel pour debug
        Mat sobel_viz = visualizeSobelGradients(roi);
        // Redimensionner et placer dans l'image de debug
        Mat sobel_resized;
        resize(sobel_viz, sobel_resized, Size(80, 160));
        sobel_resized.copyTo(debug_sobel(Rect(i*90, 10, 80, 160)));

        // Choix de la couleur selon validation
        Scalar color = classical_ok ? Scalar(0, 255, 0)   // vert = confirmé
                                    : Scalar(0, 165, 255); // orange = DL only

        rectangle(result, box, color, 3);

        // Création du label
        string label = "Personne " + to_string(i + 1) + " " + 
                      to_string(int(conf * 100 + 0.5)) + "%";
        
        string sobel_status = classical_ok ? "✓ Sobel OK" : "✗ Sobel faible";
        label += "  " + sobel_status;

        int baseline = 0;
        Size textSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.55, 2, &baseline);
        Point textOrg(box.x, box.y - 8);

        // Fond du texte
        rectangle(result, textOrg + Point(0, baseline),
                  textOrg + Point(textSize.width, -textSize.height - baseline),
                  color, FILLED);

        // Texte
        putText(result, label, textOrg,
                FONT_HERSHEY_SIMPLEX, 0.55, Scalar(255,255,255), 2);
        
        // Affichage console détaillé
        cout << "\nPiéton " << (i+1) << ":\n";
        cout << "  └─ Confiance YOLO: " << int(conf*100) << "%\n";
        cout << "  └─ Position: (" << box.x << ", " << box.y << ")\n";
        cout << "  └─ Taille: " << box.width << "x" << box.height << "\n";
        cout << "  └─ Validation Sobel: " << (classical_ok ? "✓ PASSÉE" : "✗ NON PASSÉE") << "\n";
        
        // Détails Sobel
        Mat gray;
        cvtColor(roi, gray, COLOR_BGR2GRAY);
        Mat grad_x, grad_y;
        Sobel(gray, grad_x, CV_16S, 1, 0, 3);
        Sobel(gray, grad_y, CV_16S, 0, 1, 3);
        Scalar mean_x = mean(abs(grad_x));
        Scalar mean_y = mean(abs(grad_y));
        cout << "  └─ Gradients (moyenne): H=" << int(mean_x[0]) 
             << ", V=" << int(mean_y[0]) << "\n";
    }

    // ─── Sauvegarde et affichage ────────────────────────────────
    cout << "\n========================================\n";
    imwrite(outputPath, result);
    cout << "Image sauvegardée: " << outputPath << "\n";

    // Affichage des résultats
    cout << "\n[7] Affichage des résultats...\n";
    
    // Fenêtre principale
    namedWindow("Détection Piétons + Validation Sobel", WINDOW_NORMAL);
    imshow("Détection Piétons + Validation Sobel", result);
    
    // Fenêtre de debug Sobel
    if (indices.size() > 0) {
        namedWindow("Visualisation Sobel (Rouge=Vertical, Bleu=Horizontal)", WINDOW_NORMAL);
        imshow("Visualisation Sobel (Rouge=Vertical, Bleu=Horizontal)", debug_sobel);
    }
    
    // Image originale pour comparaison
    namedWindow("Image originale", WINDOW_NORMAL);
    imshow("Image originale", img);

    cout << "\nAppuyez sur une touche pour quitter...\n";
    waitKey(0);
    destroyAllWindows();

    cout << "\nTraitement terminé!\n";
    return 0;
}