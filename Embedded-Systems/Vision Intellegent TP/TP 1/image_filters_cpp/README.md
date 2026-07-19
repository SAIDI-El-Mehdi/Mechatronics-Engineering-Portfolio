# Projet C++ - Filtres de contours et detection de lignes

## 1. Contenu du projet
Ce projet suit le TP fourni et implemente exactement les quatre traitements presentes dans le PDF :
- Prewitt
- Sobel
- Canny
- HoughLinesP

Le programme charge automatiquement les images depuis :
- `images/agriculture`
- `images/automobile`

Puis il enregistre les resultats dans le dossier `output`.

## 2. Arborescence
```text
image_filters_cpp/
|- filter_test.cpp
|- images/
|  |- agriculture/
|  |  |- feuille1.jpg
|  |  |- plante1.jpg
|  |  `- serre1.jpg
|  `- automobile/
|     |- voiture1.jpg
|     |- route1.jpg
|     `- piece1.jpg
`- output/
```

## 3. Environnement recommande sur Windows
Le plus simple est d'utiliser **MSYS2 UCRT64**.

### Installation minimale dans MSYS2 UCRT64
```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-opencv
```

## 4. Compilation
Ouvrir **MSYS2 UCRT64**, puis se placer dans le dossier du projet.

Exemple si le projet est extrait dans `C:\image_filters_cpp` :
```bash
cd /c/image_filters_cpp
```

Compiler avec la commande explicite suivante :
```bash
g++ -std=c++17 filter_test.cpp -o filter_test.exe -I/ucrt64/include/opencv4 -L/ucrt64/lib -lopencv_imgcodecs -lopencv_imgproc -lopencv_core
```

## 5. Execution
Toujours depuis le dossier du projet :
```bash
./filter_test.exe
```

## 6. Resultats
Les resultats seront ranges automatiquement dans :
- `output/agriculture/prewitt`
- `output/agriculture/sobel`
- `output/agriculture/canny`
- `output/agriculture/hough`
- `output/automobile/prewitt`
- `output/automobile/sobel`
- `output/automobile/canny`
- `output/automobile/hough`

## 7. Noms des fichiers de sortie
Exemples :
- `feuille1_prewitt_x.png`
- `feuille1_prewitt_y.png`
- `feuille1_prewitt_combined.png`
- `feuille1_prewitt_comparison.png`
- `route1_canny_edges.png`
- `route1_hough_lines.png`
- `route1_hough_comparison.png`

## 8. Logique du code
- **Prewitt** : noyaux 3x3 en X et Y, `filter2D`, `convertScaleAbs`, combinaison 0.5 / 0.5
- **Sobel** : `Sobel(..., CV_16S, ...)`, conversion absolue, combinaison 0.5 / 0.5
- **Canny** : flou gaussien 3x3 puis `Canny(50, 150, 3)`
- **Hough** : flou gaussien 9x9, `Canny(50, 150)`, puis `HoughLinesP(..., 1, PI/180, 50, 50, 10)`

## 9. Remarque importante
Le programme suppose qu'il est execute **depuis le dossier racine du projet**.
Si tu lances l'executable depuis un autre dossier, les chemins relatifs `images/...` et `output/...` ne correspondront plus.
