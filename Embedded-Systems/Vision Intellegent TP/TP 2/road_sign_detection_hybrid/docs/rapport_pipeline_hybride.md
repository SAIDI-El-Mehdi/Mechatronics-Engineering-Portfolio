# Partie rapport — Pipeline hybride de détection de panneaux routiers

## Introduction
Ce TP vise à réaliser un système de détection de panneaux routiers à partir d'images BGR. Le pipeline suit une logique hybride : si un modèle YOLO est disponible, une branche Python réalise l'inférence par réseau de neurones ; sinon, une branche classique C++/OpenCV applique une détection basée sur la couleur, la morphologie, les contours et l'analyse géométrique.

## Objectif
L'objectif est de comparer deux stratégies de vision par ordinateur :
1. une approche classique interprétable fondée sur HSV, morphologie, contours, circularité, approxPolyDP et Hough ;
2. une approche par apprentissage profond avec YOLO, plus robuste lorsque le modèle est bien entraîné.

## Description du pipeline hybride
Le système reçoit une image BGR. Un test vérifie si un modèle YOLO existe. Si oui, la branche Python charge le modèle et applique l'inférence. Si non, la branche C++ applique le pipeline classique : lissage gaussien, conversion HSV, masques rouge/bleu/jaune, ouverture/fermeture morphologique, extraction de contours, ROI, reconnaissance couleur/forme, fusion des détections et annotation.

## Branche YOLO
La branche YOLO repose sur un modèle pré-entraîné ou entraîné spécifiquement pour les panneaux routiers. Elle produit des boîtes englobantes, des classes et des scores de confiance. Elle est généralement plus robuste face aux variations d'éclairage, d'échelle et de perspective, mais dépend fortement de la qualité du modèle.

## Branche classique C++ OpenCV
La branche classique ne nécessite pas de modèle. Elle exploite les couleurs caractéristiques des panneaux et leur géométrie. Elle est simple à expliquer et adaptée à un TP, mais elle peut échouer dans les cas de faible luminosité, couleurs altérées, occultations ou arrière-plans complexes.

## Comparaison
YOLO est plus robuste et souvent plus précis, mais son installation et son entraînement sont plus lourds. La méthode classique est plus transparente et plus facile à relier aux notions du cours, mais moins générale. Pour un TP, la méthode classique permet de comprendre les blocs fondamentaux, tandis que YOLO sert d'extension moderne.

## Conclusion
Le pipeline hybride combine les avantages des deux approches. Il permet de travailler avec une solution classique immédiatement exploitable, tout en laissant ouverte l'intégration d'une branche YOLO pour améliorer la robustesse et comparer les performances.
