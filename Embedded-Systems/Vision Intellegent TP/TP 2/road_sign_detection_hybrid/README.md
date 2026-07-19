# Road Sign Detection — Pipeline Hybride YOLO / OpenCV C++

## 1. Objectif

Ce projet implémente un système hybride de détection de panneaux routiers.

Deux branches sont disponibles :

1. Branche Python YOLO
   - utilisée si un modèle YOLO existe dans `python_yolo/models/`
   - réalise l'inférence DNN
   - annote les panneaux détectés

2. Branche C++ classique OpenCV
   - utilisée sans YOLO
   - applique : flou gaussien, HSV, masques rouge/bleu/jaune, morphologie, contours, ROI, couleur + forme, Hough
   - annote l'image finale et génère un rapport

Le TP demande un pipeline hybride avec branche classique toujours active et branche YOLO optionnelle si les fichiers du modèle sont disponibles.

## 2. Structure du projet

```text
road_sign_detection/
├── run_hybrid.py
├── python_yolo/
│   ├── detect_yolo.py
│   └── models/
│       └── README_MODELS.md
├── cpp_classic/
│   ├── detect_classic.cpp
│   └── build/
├── images/
│   ├── input/
│   │   └── PUT_IMAGES_HERE.txt
│   └── output/
├── docs/
│   └── rapport_pipeline_hybride.md
└── README.md
```

## 3. Où mettre les images

Place les images dans :

```text
road_sign_detection/images/input/
```

Formats acceptés :

```text
.jpg, .jpeg, .png, .bmp, .tif, .tiff
```

## 4. Branche Python YOLO

### 4.1 Installation

Sous Windows, utilise Python normal ou Python dans un environnement virtuel.

```bash
pip install ultralytics opencv-python
```

### 4.2 Placer le modèle

Place ton modèle ici :

```text
python_yolo/models/best.pt
```

### 4.3 Exécution YOLO

Depuis le dossier `road_sign_detection` :

```bash
python python_yolo/detect_yolo.py --model python_yolo/models/best.pt --input images/input --output images/output/yolo
```

### 4.4 Résultats YOLO

Résultats :

```text
images/output/yolo/
├── *_yolo_annotated.png
├── terminal_report_yolo.txt
└── yolo_detections.csv
```

## 5. Branche C++ classique OpenCV

### 5.1 MSYS2 UCRT64

Ouvre le terminal :

```text
MSYS2 UCRT64
```

### 5.2 Installer OpenCV si nécessaire

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-opencv mingw-w64-ucrt-x86_64-pkgconf
```

### 5.3 Vérifier OpenCV

```bash
g++ --version
pkgconf --version
pkgconf --modversion opencv4
pkgconf --cflags --libs opencv4
```

### 5.4 Compiler

Depuis le dossier `road_sign_detection` :

```bash
cd /c/road_sign_detection/cpp_classic
mkdir -p build
g++ -std=c++17 detect_classic.cpp -o build/detect_classic.exe $(pkg-config --cflags --libs opencv4)
```

Si `pkg-config` ne fonctionne pas dans MSYS2, utilise `pkgconf` :

```bash
g++ -std=c++17 detect_classic.cpp -o build/detect_classic.exe $(pkgconf --cflags --libs opencv4)
```

### 5.5 Exécuter

Depuis `road_sign_detection` :

```bash
cd /c/road_sign_detection
./cpp_classic/build/detect_classic.exe --input images/input --output images/output/classic
```

### 5.6 Résultats C++

```text
images/output/classic/
├── <image_name>/
│   ├── win0_original.png
│   ├── win1_blur.png
│   ├── win2_hsv.png
│   ├── win3_red_mask.png
│   ├── win4_blue_mask.png
│   ├── win5_yellow_mask.png
│   ├── win6_combined_mask.png
│   ├── win7_opening.png
│   ├── win8_closing.png
│   ├── win9_contours.png
│   ├── win10_roi.png
│   ├── win12_canny_roi.png
│   ├── win13_contours_roi.png
│   ├── win_hough_circles.png
│   └── win_final_result.png
├── *_final_result.png
├── terminal_report_classic.txt
└── classic_detections.csv
```

## 6. Lancement hybride automatique

Le script `run_hybrid.py` choisit automatiquement :

- YOLO si `python_yolo/models/best.pt` existe
- C++ classique sinon

Depuis `road_sign_detection` :

```bash
python run_hybrid.py --input images/input --output images/output --model python_yolo/models/best.pt
```

Si YOLO n'existe pas, compile d'abord le C++.

## 7. Commandes rapides

### Compilation C++

```bash
cd /c/road_sign_detection/cpp_classic && mkdir -p build && g++ -std=c++17 detect_classic.cpp -o build/detect_classic.exe $(pkg-config --cflags --libs opencv4)
```

### Exécution C++

```bash
cd /c/road_sign_detection && ./cpp_classic/build/detect_classic.exe --input images/input --output images/output/classic
```

### Exécution YOLO

```bash
cd /c/road_sign_detection && python python_yolo/detect_yolo.py --model python_yolo/models/best.pt --input images/input --output images/output/yolo
```

### Exécution hybride

```bash
cd /c/road_sign_detection && python run_hybrid.py --input images/input --output images/output --model python_yolo/models/best.pt
```

## 8. Erreurs fréquentes

### Erreur : `opencv2/opencv.hpp: No such file or directory`

OpenCV n'est pas installé ou les flags ne sont pas fournis.

Correction :

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-opencv mingw-w64-ucrt-x86_64-pkgconf
g++ -std=c++17 detect_classic.cpp -o build/detect_classic.exe $(pkg-config --cflags --libs opencv4)
```

### Erreur : `pkg-config: command not found`

Correction :

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-pkgconf
```

Ou utilise :

```bash
pkgconf --cflags --libs opencv4
```

### Erreur : aucune image trouvée

Vérifie :

```bash
find images/input -type f
```

### Erreur YOLO : `ultralytics is not installed`

Correction :

```bash
pip install ultralytics opencv-python
```

### Erreur YOLO : modèle introuvable

Place le modèle ici :

```text
python_yolo/models/best.pt
```

## 9. Comparaison YOLO / OpenCV classique

| Critère | YOLO Python | OpenCV C++ classique |
|---|---|---|
| Précision | élevée si modèle entraîné | moyenne à bonne selon les seuils |
| Rapidité | dépend du modèle et du matériel | rapide sur CPU |
| Installation | plus lourde | plus légère avec OpenCV |
| Dépendances | Python, ultralytics, modèle YOLO | C++, OpenCV |
| Robustesse | meilleure en conditions réelles | sensible à l'éclairage et aux couleurs |
| Interprétabilité | plus faible | très bonne |
| Adaptation TP | extension moderne | base principale pédagogique |

## 10. Notes

La branche classique suit directement les blocs du TP :
BGR -> flou -> HSV -> masques R/B/J -> morphologie -> contours -> ROI -> couleur + forme -> annotation.
