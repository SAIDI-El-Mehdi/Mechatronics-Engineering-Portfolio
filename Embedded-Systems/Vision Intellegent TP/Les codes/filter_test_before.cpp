#include <opencv2/opencv.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace cv;
using namespace std;

/*=========================================================
  Structures de donnees
=========================================================*/

struct NamedImage {
    string name;
    Mat image;
};

struct FilterRun {
    string filterName;
    vector<NamedImage> outputs;
    double executionTimeMs = 0.0;
};

struct FilterStats {
    double totalTimeMs = 0.0;
    int count = 0;
};

/*=========================================================
  Outils utilitaires
=========================================================*/

bool hasValidImageExtension(const fs::path& filePath) {
    string ext = filePath.extension().string();
    transform(ext.begin(), ext.end(), ext.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });

    return ext == ".jpg" || ext == ".jpeg" || ext == ".png" ||
           ext == ".bmp" || ext == ".tif" || ext == ".tiff";
}

vector<fs::path> listImagesRecursively(const fs::path& rootFolder) {
    vector<fs::path> images;

    if (!fs::exists(rootFolder) || !fs::is_directory(rootFolder)) {
        return images;
    }

    for (const auto& entry : fs::recursive_directory_iterator(rootFolder)) {
        if (entry.is_regular_file() && hasValidImageExtension(entry.path())) {
            images.push_back(entry.path());
        }
    }

    sort(images.begin(), images.end());
    return images;
}

void ensureDirectoryExists(const fs::path& dirPath) {
    if (!fs::exists(dirPath)) {
        fs::create_directories(dirPath);
    }
}

Mat convertToBGRIfNeeded(const Mat& image) {
    if (image.channels() == 1) {
        Mat converted;
        cvtColor(image, converted, COLOR_GRAY2BGR);
        return converted;
    }
    return image.clone();
}

Mat resizeToHeight(const Mat& image, int targetHeight) {
    if (image.empty()) {
        return image.clone();
    }

    double scale = static_cast<double>(targetHeight) / static_cast<double>(image.rows);
    int targetWidth = static_cast<int>(image.cols * scale);

    Mat resized;
    resize(image, resized, Size(targetWidth, targetHeight));
    return resized;
}

Mat createComparisonImage(const Mat& original, const Mat& result) {
    Mat left = resizeToHeight(convertToBGRIfNeeded(original), 450);
    Mat right = resizeToHeight(convertToBGRIfNeeded(result), 450);

    Mat comparison;
    hconcat(left, right, comparison);
    return comparison;
}

/*=========================================================
  Chronometrage generique
=========================================================*/

template <typename Func>
FilterRun measureFilterExecution(const string& filterName, Func filterFunction) {
    FilterRun run;
    run.filterName = filterName;

    auto start = chrono::steady_clock::now();
    run.outputs = filterFunction();
    auto end = chrono::steady_clock::now();

    run.executionTimeMs =
        chrono::duration<double, milli>(end - start).count();

    return run;
}

/*=========================================================
  Filtres
=========================================================*/

FilterRun applyPrewitt(const Mat& src) {
    return measureFilterExecution("prewitt", [&]() {
        vector<NamedImage> outputs;

        Mat gray;
        cvtColor(src, gray, COLOR_BGR2GRAY);
        GaussianBlur(gray, gray, Size(3, 3), 0, 0);

        Mat kernelX = (Mat_<float>(3, 3) <<
            -1, 0, 1,
            -1, 0, 1,
            -1, 0, 1);

        Mat kernelY = (Mat_<float>(3, 3) <<
            -1, -1, -1,
             0,  0,  0,
             1,  1,  1);

        Mat prewittX, prewittY;
        filter2D(gray, prewittX, CV_16S, kernelX);
        filter2D(gray, prewittY, CV_16S, kernelY);

        Mat absX, absY;
        convertScaleAbs(prewittX, absX);
        convertScaleAbs(prewittY, absY);

        Mat combined;
        addWeighted(absX, 0.5, absY, 0.5, 0, combined);

        outputs.push_back({"x", absX});
        outputs.push_back({"y", absY});
        outputs.push_back({"combined", combined});
        outputs.push_back({"comparison", createComparisonImage(src, combined)});

        return outputs;
    });
}

FilterRun applySobel(const Mat& src) {
    return measureFilterExecution("sobel", [&]() {
        vector<NamedImage> outputs;

        Mat blurred, gray;
        GaussianBlur(src, blurred, Size(3, 3), 0, 0, BORDER_DEFAULT);
        cvtColor(blurred, gray, COLOR_BGR2GRAY);

        Mat gradX, gradY;
        Sobel(gray, gradX, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
        Sobel(gray, gradY, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);

        Mat absX, absY;
        convertScaleAbs(gradX, absX);
        convertScaleAbs(gradY, absY);

        Mat combined;
        addWeighted(absX, 0.5, absY, 0.5, 0, combined);

        outputs.push_back({"x", absX});
        outputs.push_back({"y", absY});
        outputs.push_back({"combined", combined});
        outputs.push_back({"comparison", createComparisonImage(src, combined)});

        return outputs;
    });
}

FilterRun applyCanny(const Mat& src) {
    return measureFilterExecution("canny", [&]() {
        vector<NamedImage> outputs;

        Mat gray, blurred, edges;
        cvtColor(src, gray, COLOR_BGR2GRAY);
        GaussianBlur(gray, blurred, Size(3, 3), 0);
        Canny(blurred, edges, 50, 150, 3);

        outputs.push_back({"edges", edges});
        outputs.push_back({"comparison", createComparisonImage(src, edges)});

        return outputs;
    });
}

FilterRun applyHough(const Mat& src) {
    return measureFilterExecution("hough", [&]() {
        vector<NamedImage> outputs;

        Mat gray, edges;
        cvtColor(src, gray, COLOR_BGR2GRAY);
        GaussianBlur(gray, gray, Size(9, 9), 2, 2);
        Canny(gray, edges, 50, 150);

        vector<Vec4i> lines;
        HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 50, 10);

        Mat lineImage = src.clone();
        for (size_t i = 0; i < lines.size(); i++) {
            line(lineImage,
                 Point(lines[i][0], lines[i][1]),
                 Point(lines[i][2], lines[i][3]),
                 Scalar(0, 0, 255), 3);
        }

        outputs.push_back({"edges", edges});
        outputs.push_back({"lines", lineImage});
        outputs.push_back({"comparison", createComparisonImage(src, lineImage)});

        return outputs;
    });
}

/*=========================================================
  Sauvegarde des images dans output/images
=========================================================*/

bool saveFilterOutputs(const fs::path& inputRoot,
                       const fs::path& outputImagesRoot,
                       const fs::path& imagePath,
                       const FilterRun& run) {
    fs::path relativeParent;

    try {
        relativeParent = fs::relative(imagePath.parent_path(), inputRoot);
    } catch (...) {
        relativeParent.clear();
    }

    if (relativeParent == fs::path(".")) {
        relativeParent.clear();
    }

    fs::path filterDir = outputImagesRoot / relativeParent / run.filterName;
    ensureDirectoryExists(filterDir);

    string imageStem = imagePath.stem().string();

    for (const auto& item : run.outputs) {
        fs::path outputFile =
            filterDir / (imageStem + "_" + run.filterName + "_" + item.name + ".png");

        if (!imwrite(outputFile.string(), item.image)) {
            cerr << "Erreur lors de l'enregistrement : " << outputFile << endl;
            return false;
        }
    }

    return true;
}

/*=========================================================
  Rapport texte des temps d'execution
=========================================================*/

string buildTimingReport(const map<string, FilterStats>& stats,
                         const vector<pair<string, vector<FilterRun>>>& perImageRuns,
                         const fs::path& inputRoot,
                         const fs::path& outputRoot) {
    ostringstream report;
    report << fixed << setprecision(3);

    report << "MESURE DES TEMPS D'EXECUTION DES FILTRES\n";
    report << "========================================\n\n";
    report << "Dossier d'entree : " << inputRoot.string() << "\n";
    report << "Dossier de sortie : " << outputRoot.string() << "\n\n";

    report << "DETAIL PAR IMAGE\n";
    report << "----------------\n\n";

    for (const auto& entry : perImageRuns) {
        report << "Image : " << entry.first << "\n";
        for (const auto& run : entry.second) {
            report << "  - " << left << setw(8) << run.filterName
                   << " : temps d'execution = "
                   << run.executionTimeMs << " ms\n";
        }
        report << "\n";
    }

    report << "RESUME COMPARATIF GLOBAL\n";
    report << "------------------------\n\n";

    string fastestFilter;
    string slowestFilter;
    double minAverage = numeric_limits<double>::max();
    double maxAverage = numeric_limits<double>::lowest();

    for (const auto& [filterName, filterStats] : stats) {
        double averageTime = 0.0;
        if (filterStats.count > 0) {
            averageTime = filterStats.totalTimeMs / filterStats.count;
        }

        report << "- " << left << setw(8) << filterName
               << " : total = " << filterStats.totalTimeMs << " ms"
               << " | moyenne = " << averageTime << " ms"
               << " | executions = " << filterStats.count << "\n";

        if (filterStats.count > 0 && averageTime < minAverage) {
            minAverage = averageTime;
            fastestFilter = filterName;
        }

        if (filterStats.count > 0 && averageTime > maxAverage) {
            maxAverage = averageTime;
            slowestFilter = filterName;
        }
    }

    report << "\n";
    report << "Filtre le plus rapide : " << fastestFilter
           << " (moyenne = " << minAverage << " ms)\n";
    report << "Filtre le plus lent   : " << slowestFilter
           << " (moyenne = " << maxAverage << " ms)\n";

    return report.str();
}

bool saveTimingReport(const fs::path& outputRoot, const string& reportText) {
    ensureDirectoryExists(outputRoot);

    fs::path reportFile = outputRoot / "execution_times.txt";
    ofstream file(reportFile);

    if (!file) {
        cerr << "Erreur : impossible de creer le fichier " << reportFile << endl;
        return false;
    }

    file << reportText;
    file.close();

    return true;
}

/*=========================================================
  Affichage console
=========================================================*/

void printFilterTime(const FilterRun& run) {
    cout << "    " << left << setw(8) << run.filterName
         << " : temps d'execution = "
         << fixed << setprecision(3)
         << run.executionTimeMs << " ms" << endl;
}

/*=========================================================
  Programme principal
=========================================================*/

int main() {
    fs::path projectRoot = fs::current_path();
    fs::path inputRoot = projectRoot / "images";
    fs::path outputRoot = projectRoot / "output";
    fs::path outputImagesRoot = outputRoot / "images";

    ensureDirectoryExists(outputRoot);
    ensureDirectoryExists(outputImagesRoot);

    vector<fs::path> images = listImagesRecursively(inputRoot);

    if (images.empty()) {
        cerr << "Aucune image trouvee dans le dossier : " << inputRoot << endl;
        return 1;
    }

    map<string, FilterStats> globalStats = {
        {"prewitt", {}},
        {"sobel",   {}},
        {"canny",   {}},
        {"hough",   {}}
    };

    vector<pair<string, vector<FilterRun>>> perImageRuns;

    cout << "==============================================" << endl;
    cout << "Application des 4 filtres avec chronometrage" << endl;
    cout << "Dossier d'entree  : " << inputRoot << endl;
    cout << "Dossier de sortie : " << outputRoot << endl;
    cout << "==============================================" << endl;

    for (const auto& imagePath : images) {
        cout << "\nTraitement de : " << imagePath.filename().string() << endl;
        cout << "----------------------------------------------" << endl;

        Mat src = imread(imagePath.string(), IMREAD_COLOR);
        if (src.empty()) {
            cerr << "Erreur : impossible de lire l'image " << imagePath << endl;
            continue;
        }

        vector<FilterRun> runs;
        runs.push_back(applyPrewitt(src));
        runs.push_back(applySobel(src));
        runs.push_back(applyCanny(src));
        runs.push_back(applyHough(src));

        for (const auto& run : runs) {
            bool saved = saveFilterOutputs(inputRoot, outputImagesRoot, imagePath, run);

            if (!saved) {
                cerr << "Erreur lors de la sauvegarde pour le filtre : "
                     << run.filterName << endl;
                continue;
            }

            printFilterTime(run);
            globalStats[run.filterName].totalTimeMs += run.executionTimeMs;
            globalStats[run.filterName].count++;
        }

        perImageRuns.push_back({imagePath.string(), runs});
    }

    string reportText = buildTimingReport(globalStats, perImageRuns, inputRoot, outputRoot);

    if (!saveTimingReport(outputRoot, reportText)) {
        cerr << "Erreur : le fichier execution_times.txt n'a pas pu etre enregistre." << endl;
        return 1;
    }

    cout << "\n==============================================" << endl;
    cout << "Resume global" << endl;
    cout << "==============================================" << endl;
    cout << reportText << endl;

    cout << "\nTraitement termine." << endl;
    cout << "Images enregistrees dans : " << outputImagesRoot << endl;
    cout << "Temps enregistres dans   : " << (outputRoot / "execution_times.txt") << endl;

    return 0;
}