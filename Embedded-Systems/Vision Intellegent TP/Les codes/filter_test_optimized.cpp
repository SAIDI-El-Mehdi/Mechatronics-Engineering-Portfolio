#include <opencv2/opencv.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
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

struct NamedImage {
    string name;
    Mat image;
};

struct FilterResult {
    string filterName;
    vector<NamedImage> outputs;
    double coreTimeMs = 0.0;
    double comparisonTimeMs = 0.0;
};

struct ImageRun {
    string imagePath;
    double loadTimeMs = 0.0;
    double preprocessingTimeMs = 0.0;
    double saveTimeMs = 0.0;
    double totalTimeMs = 0.0;
    vector<FilterResult> filters;
};

struct AggregateStats {
    double totalMs = 0.0;
    int count = 0;
};

struct PrecomputedData {
    Mat gray;
    Mat blur3Gray;
    Mat sobelGray;
    Mat blur9Gray;
};

static const Mat PREWITT_KERNEL_X = (Mat_<float>(3, 3) <<
    -1, 0, 1,
    -1, 0, 1,
    -1, 0, 1);

static const Mat PREWITT_KERNEL_Y = (Mat_<float>(3, 3) <<
    -1, -1, -1,
     0,  0,  0,
     1,  1,  1);

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
    return image;
}

Mat resizeToHeight(const Mat& image, int targetHeight) {
    if (image.empty()) {
        return image.clone();
    }

    if (image.rows == targetHeight) {
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

template <typename Func>
double measureMilliseconds(Func&& func) {
    auto start = chrono::steady_clock::now();
    func();
    auto end = chrono::steady_clock::now();
    return chrono::duration<double, milli>(end - start).count();
}

PrecomputedData buildPrecomputedData(const Mat& src) {
    PrecomputedData data;

    cvtColor(src, data.gray, COLOR_BGR2GRAY);
    GaussianBlur(data.gray, data.blur3Gray, Size(3, 3), 0, 0);

    Mat blur3Bgr;
    GaussianBlur(src, blur3Bgr, Size(3, 3), 0, 0, BORDER_DEFAULT);
    cvtColor(blur3Bgr, data.sobelGray, COLOR_BGR2GRAY);

    GaussianBlur(data.gray, data.blur9Gray, Size(9, 9), 2, 2);

    return data;
}

FilterResult applyPrewitt(const Mat& original, const PrecomputedData& data) {
    FilterResult result;
    result.filterName = "prewitt";
    result.outputs.reserve(4);

    Mat absX, absY, combined;

    result.coreTimeMs = measureMilliseconds([&]() {
        Mat prewittX, prewittY;
        filter2D(data.blur3Gray, prewittX, CV_16S, PREWITT_KERNEL_X);
        filter2D(data.blur3Gray, prewittY, CV_16S, PREWITT_KERNEL_Y);
        convertScaleAbs(prewittX, absX);
        convertScaleAbs(prewittY, absY);
        addWeighted(absX, 0.5, absY, 0.5, 0, combined);
    });

    result.outputs.push_back({"x", absX});
    result.outputs.push_back({"y", absY});
    result.outputs.push_back({"combined", combined});

    result.comparisonTimeMs = measureMilliseconds([&]() {
        result.outputs.push_back({"comparison", createComparisonImage(original, combined)});
    });

    return result;
}

FilterResult applySobel(const Mat& original, const PrecomputedData& data) {
    FilterResult result;
    result.filterName = "sobel";
    result.outputs.reserve(4);

    Mat absX, absY, combined;

    result.coreTimeMs = measureMilliseconds([&]() {
        Mat gradX, gradY;
        Sobel(data.sobelGray, gradX, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
        Sobel(data.sobelGray, gradY, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
        convertScaleAbs(gradX, absX);
        convertScaleAbs(gradY, absY);
        addWeighted(absX, 0.5, absY, 0.5, 0, combined);
    });

    result.outputs.push_back({"x", absX});
    result.outputs.push_back({"y", absY});
    result.outputs.push_back({"combined", combined});

    result.comparisonTimeMs = measureMilliseconds([&]() {
        result.outputs.push_back({"comparison", createComparisonImage(original, combined)});
    });

    return result;
}

FilterResult applyCanny(const Mat& original, const PrecomputedData& data) {
    FilterResult result;
    result.filterName = "canny";
    result.outputs.reserve(2);

    Mat edges;

    result.coreTimeMs = measureMilliseconds([&]() {
        Canny(data.blur3Gray, edges, 50, 150, 3);
    });

    result.outputs.push_back({"edges", edges});

    result.comparisonTimeMs = measureMilliseconds([&]() {
        result.outputs.push_back({"comparison", createComparisonImage(original, edges)});
    });

    return result;
}

FilterResult applyHough(const Mat& original, const PrecomputedData& data) {
    FilterResult result;
    result.filterName = "hough";
    result.outputs.reserve(3);

    Mat edges;
    Mat lineImage;

    result.coreTimeMs = measureMilliseconds([&]() {
        Canny(data.blur9Gray, edges, 50, 150);

        vector<Vec4i> lines;
        HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 50, 10);

        lineImage = original.clone();
        for (const auto& lineSegment : lines) {
            line(lineImage,
                 Point(lineSegment[0], lineSegment[1]),
                 Point(lineSegment[2], lineSegment[3]),
                 Scalar(0, 0, 255), 3);
        }
    });

    result.outputs.push_back({"edges", edges});
    result.outputs.push_back({"lines", lineImage});

    result.comparisonTimeMs = measureMilliseconds([&]() {
        result.outputs.push_back({"comparison", createComparisonImage(original, lineImage)});
    });

    return result;
}

bool saveFilterOutputs(const fs::path& inputRoot,
                       const fs::path& outputImagesRoot,
                       const fs::path& imagePath,
                       const FilterResult& result) {
    fs::path relativeParent;

    try {
        relativeParent = fs::relative(imagePath.parent_path(), inputRoot);
    } catch (...) {
        relativeParent.clear();
    }

    if (relativeParent == fs::path(".")) {
        relativeParent.clear();
    }

    fs::path filterDir = outputImagesRoot / relativeParent / result.filterName;
    ensureDirectoryExists(filterDir);

    const string imageStem = imagePath.stem().string();

    for (const auto& item : result.outputs) {
        fs::path outputFile = filterDir / (imageStem + "_" + result.filterName + "_" + item.name + ".png");
        if (!imwrite(outputFile.string(), item.image)) {
            cerr << "Erreur lors de l'enregistrement : " << outputFile << endl;
            return false;
        }
    }

    return true;
}

void updateStats(map<string, AggregateStats>& stats, const string& key, double value) {
    auto& stat = stats[key];
    stat.totalMs += value;
    stat.count += 1;
}

string buildTimingReport(const vector<ImageRun>& runs,
                         const map<string, AggregateStats>& stats,
                         const fs::path& inputRoot,
                         const fs::path& outputRoot,
                         double reportWriteTimeMs,
                         double programTotalTimeMs) {
    ostringstream report;
    report << fixed << setprecision(3);

    report << "MESURE DES TEMPS D'EXECUTION APRES OPTIMISATION\n";
    report << "=============================================\n\n";
    report << "Dossier d'entree : " << inputRoot.string() << "\n";
    report << "Dossier de sortie : " << outputRoot.string() << "\n\n";

    report << "DETAIL PAR IMAGE\n";
    report << "----------------\n\n";

    for (const auto& imageRun : runs) {
        report << "Image : " << imageRun.imagePath << "\n";
        report << "  - chargement      : " << imageRun.loadTimeMs << " ms\n";
        report << "  - pretraitement   : " << imageRun.preprocessingTimeMs << " ms\n";

        for (const auto& filter : imageRun.filters) {
            report << "  - filtre " << setw(8) << left << filter.filterName
                   << " : calcul = " << filter.coreTimeMs << " ms"
                   << " | comparaison = " << filter.comparisonTimeMs << " ms\n";
        }

        report << "  - sauvegarde      : " << imageRun.saveTimeMs << " ms\n";
        report << "  - temps total     : " << imageRun.totalTimeMs << " ms\n\n";
    }

    report << "RESUME GLOBAL PAR BLOC\n";
    report << "----------------------\n\n";

    string fastestBlock;
    string slowestBlock;
    double minAverage = numeric_limits<double>::max();
    double maxAverage = numeric_limits<double>::lowest();

    for (const auto& [name, stat] : stats) {
        const double average = (stat.count > 0) ? (stat.totalMs / stat.count) : 0.0;
        report << "- " << left << setw(20) << name
               << " : total = " << stat.totalMs << " ms"
               << " | moyenne = " << average << " ms"
               << " | executions = " << stat.count << "\n";

        if (stat.count > 0 && average < minAverage) {
            minAverage = average;
            fastestBlock = name;
        }
        if (stat.count > 0 && average > maxAverage) {
            maxAverage = average;
            slowestBlock = name;
        }
    }

    report << "\nBloc le plus rapide : " << fastestBlock
           << " (moyenne = " << minAverage << " ms)\n";
    report << "Bloc le plus lent   : " << slowestBlock
           << " (moyenne = " << maxAverage << " ms)\n\n";

    report << "Temps d'ecriture du rapport : " << reportWriteTimeMs << " ms\n";
    report << "Temps total du programme    : " << programTotalTimeMs << " ms\n";

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
    return true;
}

void printImageSummary(const ImageRun& imageRun) {
    cout << "\nTraitement de : " << fs::path(imageRun.imagePath).filename().string() << '\n';
    cout << "----------------------------------------------\n";
    cout << "  chargement    : " << fixed << setprecision(3) << imageRun.loadTimeMs << " ms\n";
    cout << "  pretraitement : " << imageRun.preprocessingTimeMs << " ms\n";

    for (const auto& filter : imageRun.filters) {
        cout << "  " << left << setw(10) << filter.filterName
             << " : calcul = " << filter.coreTimeMs << " ms"
             << " | comparaison = " << filter.comparisonTimeMs << " ms\n";
    }

    cout << "  sauvegarde    : " << imageRun.saveTimeMs << " ms\n";
    cout << "  total image   : " << imageRun.totalTimeMs << " ms\n";
}

int main() {
    const auto programStart = chrono::steady_clock::now();

    const fs::path projectRoot = fs::current_path();
    const fs::path inputRoot = projectRoot / "images";
    const fs::path outputRoot = projectRoot / "output";
    const fs::path outputImagesRoot = outputRoot / "images";

    ensureDirectoryExists(outputRoot);
    ensureDirectoryExists(outputImagesRoot);

    const vector<fs::path> images = listImagesRecursively(inputRoot);
    if (images.empty()) {
        cerr << "Aucune image trouvee dans le dossier : " << inputRoot << endl;
        return 1;
    }

    map<string, AggregateStats> stats;
    vector<ImageRun> allRuns;
    allRuns.reserve(images.size());

    cout << "==============================================\n";
    cout << "Application optimisee des 4 filtres\n";
    cout << "Dossier d'entree  : " << inputRoot << '\n';
    cout << "Dossier de sortie : " << outputRoot << '\n';
    cout << "==============================================\n";

    for (const auto& imagePath : images) {
        ImageRun imageRun;
        imageRun.imagePath = imagePath.string();
        imageRun.filters.reserve(4);

        Mat src;
        imageRun.loadTimeMs = measureMilliseconds([&]() {
            src = imread(imagePath.string(), IMREAD_COLOR);
        });

        if (src.empty()) {
            cerr << "Erreur : impossible de lire l'image " << imagePath << endl;
            continue;
        }
        updateStats(stats, "chargement", imageRun.loadTimeMs);

        PrecomputedData data;
        imageRun.preprocessingTimeMs = measureMilliseconds([&]() {
            data = buildPrecomputedData(src);
        });
        updateStats(stats, "pretraitement", imageRun.preprocessingTimeMs);

        imageRun.filters.push_back(applyPrewitt(src, data));
        imageRun.filters.push_back(applySobel(src, data));
        imageRun.filters.push_back(applyCanny(src, data));
        imageRun.filters.push_back(applyHough(src, data));

        for (const auto& filter : imageRun.filters) {
            updateStats(stats, filter.filterName + "_calcul", filter.coreTimeMs);
            updateStats(stats, filter.filterName + "_comparaison", filter.comparisonTimeMs);
        }

        imageRun.saveTimeMs = measureMilliseconds([&]() {
            for (const auto& filter : imageRun.filters) {
                if (!saveFilterOutputs(inputRoot, outputImagesRoot, imagePath, filter)) {
                    cerr << "Erreur lors de la sauvegarde pour le filtre : "
                         << filter.filterName << endl;
                }
            }
        });
        updateStats(stats, "sauvegarde", imageRun.saveTimeMs);

        imageRun.totalTimeMs = imageRun.loadTimeMs + imageRun.preprocessingTimeMs + imageRun.saveTimeMs;
        for (const auto& filter : imageRun.filters) {
            imageRun.totalTimeMs += filter.coreTimeMs + filter.comparisonTimeMs;
        }
        updateStats(stats, "temps_total_image", imageRun.totalTimeMs);

        printImageSummary(imageRun);
        allRuns.push_back(std::move(imageRun));
    }

    double reportWriteTimeMs = 0.0;
    string reportText = buildTimingReport(allRuns, stats, inputRoot, outputRoot, reportWriteTimeMs, 0.0);
    bool firstReportSaved = false;
    reportWriteTimeMs = measureMilliseconds([&]() {
        firstReportSaved = saveTimingReport(outputRoot, reportText);
    });
    if (!firstReportSaved) {
        cerr << "Erreur : le fichier execution_times.txt n'a pas pu etre enregistre." << endl;
        return 1;
    }

    const double programTotalTimeMs = chrono::duration<double, milli>(chrono::steady_clock::now() - programStart).count();
    reportText = buildTimingReport(allRuns, stats, inputRoot, outputRoot, reportWriteTimeMs, programTotalTimeMs);
    if (!saveTimingReport(outputRoot, reportText)) {
        cerr << "Erreur : le fichier execution_times.txt n'a pas pu etre mis a jour." << endl;
        return 1;
    }

    cout << "\n==============================================\n";
    cout << "Resume global\n";
    cout << "==============================================\n";
    cout << "Bloc le plus couteux observe dans les anciennes mesures : hough\n";
    cout << "Nouveau rapport de temps : " << (outputRoot / "execution_times.txt") << '\n';
    cout << "Temps total programme    : " << fixed << setprecision(3) << programTotalTimeMs << " ms\n";
    cout << "\nTraitement termine.\n";

    return 0;
}
