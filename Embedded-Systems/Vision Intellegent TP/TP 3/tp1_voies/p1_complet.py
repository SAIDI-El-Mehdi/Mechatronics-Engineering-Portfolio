import cv2
import numpy as np
import matplotlib.pyplot as plt

# ================================================
# 1. Chargement et préparation de l'image (2 points)
# ================================================
# Charge l'image avec OpenCV (format BGR par défaut)
image = cv2.imread("6.png")

# Vérification que l'image a bien été chargée
if image is None:
    raise FileNotFoundError("Image '2.png' not found. Placez une image valide dans le répertoire.")
# Récupération des dimensions
height, width = image.shape[:2]

# Conversion BGR → RGB pour un affichage correct avec Matplotlib
image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

# ================================================
# 2. Filtrage par couleur en espace HSV (4 points)
# ================================================
# Conversion de l'image en espace HSV (Hue, Saturation, Value)
# HSV est plus robuste pour isoler des couleurs spécifiques malgré les variations d'éclairage
hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

# Masque pour les lignes blanches : haute luminosité (V), faible saturation (S)
lower_white = np.array([0, 0, 200])      # V > 200
upper_white = np.array([180, 50, 255])   # S < 50
mask_white = cv2.inRange(hsv, lower_white, upper_white)

# Masque pour les lignes jaunes : teinte (H) autour de jaune (15-35)
lower_yellow = np.array([15, 60, 100])
upper_yellow = np.array([35, 255, 255])
mask_yellow = cv2.inRange(hsv, lower_yellow, upper_yellow)

# Combinaison des deux masques : lignes blanches OU jaunes
color_mask = cv2.bitwise_or(mask_white, mask_yellow)

# Pourquoi HSV ? 
# - La teinte (H) est indépendante de l'intensité lumineuse.
# - Permet de détecter les marquages même sous ombre ou fort soleil.

# ================================================
# 3. Détection de contours améliorée (3 points)
# ================================================
# Passage en niveaux de gris
gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

# Flou gaussien pour réduire le bruit
blur = cv2.GaussianBlur(gray, (5, 5), 0)

# Détection de contours avec Canny
edges = cv2.Canny(blur, 50, 150)

# Combinaison des contours avec le masque couleur
# → on garde uniquement les contours situés sur des zones blanches/jaunes
combined = cv2.bitwise_and(edges, edges, mask=color_mask)

# Avantage : élimine beaucoup de faux positifs (ombres, textures de la route, etc.)

# ================================================
# 4. Transformation perspective – Bird's Eye View (6 points)
# ================================================
# Points source : trapèze sur l'image originale correspondant à la zone de la route
# À ADAPTER selon votre image !
src = np.float32([
    [width * 0.15, height],           # bas gauche
    [width * 0.45, height * 0.60],     # haut gauche (horizon approximatif)
    [width * 0.55, height * 0.60],     # haut droit
    [width * 0.85, height]            # bas droit
])

# Points destination : rectangle (vue de dessus)
dst = np.float32([
    [0, height],       # bas gauche
    [0, 0],            # haut gauche
    [width, 0],        # haut droit
    [width, height]    # bas droit
])

# Calcul de la matrice de transformation et de son inverse
M = cv2.getPerspectiveTransform(src, dst)
Minv = cv2.getPerspectiveTransform(dst, src)  # pour re-projeter plus tard

# Application de la transformation sur l'image binaire combinée
warped = cv2.warpPerspective(combined, M, (width, height), flags=cv2.INTER_LINEAR)

# Pourquoi Bird's Eye View ?
# - Les lignes deviennent (presque) parallèles → facilite la détection et le calcul de courbure
# - Permet des mesures métriques plus précises

# ================================================
# 5. Détection des voies par fenêtres glissantes et ajustement polynomial (8 points)
# ================================================
def detect_lanes(binary_warped):
    """
    Détecte les voies gauche et droite par fenêtres glissantes
    et ajuste un polynôme de degré 2 sur chaque voie.
    Retourne les coefficients des polynômes et les points x correspondants.
    """
    # Histogramme de la moitié inférieure de l'image (où les lignes sont les plus visibles)
    histogram = np.sum(binary_warped[binary_warped.shape[0]//2:, :], axis=0)
    
    # Identification des pics → bases des voies gauche et droite
    midpoint = np.int32(histogram.shape[0] / 2)
    leftx_base = np.argmax(histogram[:midpoint])
    rightx_base = np.argmax(histogram[midpoint:]) + midpoint

    # Paramètres des fenêtres glissantes
    nwindows = 9
    window_height = np.int32(binary_warped.shape[0] / nwindows)
    margin = 100      # largeur de chaque fenêtre
    minpix = 50       # nombre minimum de pixels pour recentrer la fenêtre

    # Récupération des coordonnées des pixels non nuls
    nonzero = binary_warped.nonzero()
    nonzeroy = np.array(nonzero[0])
    nonzerox = np.array(nonzero[1])

    # Positions courantes des fenêtres
    leftx_current = leftx_base
    rightx_current = rightx_base

    # Listes pour stocker les indices des pixels appartenant à chaque voie
    left_lane_inds = []
    right_lane_inds = []

    # Parcours des fenêtres de bas en haut
    for window in range(nwindows):
        # Coordonnées y de la fenêtre courante
        win_y_low = binary_warped.shape[0] - (window + 1) * window_height
        win_y_high = binary_warped.shape[0] - window * window_height

        # Coordonnées x des fenêtres gauche et droite
        win_xleft_low = leftx_current - margin
        win_xleft_high = leftx_current + margin
        win_xright_low = rightx_current - margin
        win_xright_high = rightx_current + margin

        # Indices des pixels activés dans chaque fenêtre
        good_left_inds = ((nonzeroy >= win_y_low) & (nonzeroy < win_y_high) &
                          (nonzerox >= win_xleft_low) & (nonzerox < win_xleft_high)).nonzero()[0]
        good_right_inds = ((nonzeroy >= win_y_low) & (nonzeroy < win_y_high) &
                           (nonzerox >= win_xright_low) & (nonzerox < win_xright_high)).nonzero()[0]

        # Ajout des indices
        left_lane_inds.append(good_left_inds)
        right_lane_inds.append(good_right_inds)

        # Recentrage de la fenêtre si assez de pixels détectés
        if len(good_left_inds) > minpix:
            leftx_current = np.int32(np.mean(nonzerox[good_left_inds]))
        if len(good_right_inds) > minpix:
            rightx_current = np.int32(np.mean(nonzerox[good_right_inds]))

    # Concaténation de tous les indices
    left_lane_inds = np.concatenate(left_lane_inds)
    right_lane_inds = np.concatenate(right_lane_inds)

    # Extraction des coordonnées x et y pour chaque voie
    leftx = nonzerox[left_lane_inds]
    lefty = nonzeroy[left_lane_inds]
    rightx = nonzerox[right_lane_inds]
    righty = nonzeroy[right_lane_inds]

    # Gestion du cas où une voie n'est pas détectée
    if len(leftx) == 0 or len(rightx) == 0:
        return None, None, None, None, None

    # Ajustement d'un polynôme de degré 2 : f(y) = Ay² + By + C
    left_fit = np.polyfit(lefty, leftx, 2)
    right_fit = np.polyfit(righty, rightx, 2)

    # Génération des y pour tracer la courbe complète
    ploty = np.linspace(0, binary_warped.shape[0] - 1, binary_warped.shape[0])

    # Calcul des x correspondants sur toute la hauteur
    left_fitx = left_fit[0] * ploty**2 + left_fit[1] * ploty + left_fit[2]
    right_fitx = right_fit[0] * ploty**2 + right_fit[1] * ploty + right_fit[2]

    return left_fit, right_fit, ploty, left_fitx, right_fitx

# Application de la fonction
left_fit, right_fit, ploty, left_fitx, right_fitx = detect_lanes(warped)

# Pourquoi degré 2 ?
# Les routes réelles ont une courbure douce → un polynôme quadratique suffit généralement.

# ================================================
# 6. Visualisation des voies détectées (4 points)
# ================================================
# Création d'une image couleur vide (même taille que la vue warpage)
lane_warp = np.zeros_like(warped)
lane_warp = cv2.cvtColor(lane_warp, cv2.COLOR_GRAY2BGR)

if left_fit is not None:
    # Points pour la voie gauche
    pts_left = np.array([np.transpose(np.vstack([left_fitx, ploty]))])
    # Points pour la voie droite (inversés pour fillPoly)
    pts_right = np.array([np.flipud(np.transpose(np.vstack([right_fitx, ploty])))])
    pts = np.hstack((pts_left, pts_right))

    # Tracé des courbes en vert épais
    cv2.polylines(lane_warp, np.int32([pts_left]), False, (0, 255, 0), thickness=25)
    cv2.polylines(lane_warp, np.int32([pts_right]), False, (0, 255, 0), thickness=25)

    # Remplissage de la zone entre les voies en vert translucide
    cv2.fillPoly(lane_warp, np.int32([pts]), (0, 255, 0, 80))

# Re-projection sur l'image originale
lane_original = cv2.warpPerspective(lane_warp, Minv, (width, height))

# Superposition sur l'image RGB originale (transparence)
result = cv2.addWeighted(image_rgb, 1.0, lane_original, 0.5, 0)

# ================================================
# 7. Calcul du rayon de courbure et du décalage véhicule (5 points)
# ================================================
if left_fit is not None:
    # Facteurs de conversion pixels → mètres (valeurs courantes dans les tutoriels)
    ym_per_pix = 30 / 720   # 30 mètres visibles sur la hauteur de l'image
    xm_per_pix = 3.7 / 700  # largeur typique d'une voie ≈ 3.7 m

    # Conversion des coefficients en mètres
    left_fit_cr = left_fit * np.array([xm_per_pix / (ym_per_pix ** 2), xm_per_pix / ym_per_pix, xm_per_pix])
    right_fit_cr = right_fit * np.array([xm_per_pix / (ym_per_pix ** 2), xm_per_pix / ym_per_pix, xm_per_pix])

    # Évaluation au bas de l'image (y maximum)
    y_eval = np.max(ploty) * ym_per_pix

    # Formule du rayon de courbure
    left_curverad = (1 + (2 * left_fit_cr[0] * y_eval + left_fit_cr[1]) ** 2) ** 1.5 / np.abs(2 * left_fit_cr[0])
    right_curverad = (1 + (2 * right_fit_cr[0] * y_eval + right_fit_cr[1]) ** 2) ** 1.5 / np.abs(2 * right_fit_cr[0])

    radius = (left_curverad + right_curverad) / 2

    # Décalage du véhicule par rapport au centre de la voie
    lane_center = (left_fitx[-1] + right_fitx[-1]) / 2
    vehicle_center = warped.shape[1] / 2
    offset_pix = (vehicle_center - lane_center) * xm_per_pix
    offset_str = f"{abs(offset_pix):.2f} m {'à gauche' if offset_pix > 0 else 'à droite'}"
else:
    radius = None
    offset_str = "Non détecté"

# Affichage des informations sur l'image finale
if radius is not None:
    cv2.putText(result, f"Rayon de courbure: {radius:.0f} m", (50, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (255, 255, 255), 3)
    cv2.putText(result, f"Decalage vehicule: {offset_str}", (50, 120),
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (255, 255, 255), 3)

# ================================================
# 8. Affichage complet des étapes (4 points)
# ================================================
plt.figure(figsize=(20, 12))

plt.subplot(3, 4, 1)
plt.title("1. Image originale")
plt.imshow(image_rgb)
plt.axis("off")

plt.subplot(3, 4, 2)
plt.title("2. Masque couleur (blanc/jaune)")
plt.imshow(color_mask, cmap="gray")
plt.axis("off")

plt.subplot(3, 4, 3)
plt.title("3. Contours + masque couleur")
plt.imshow(combined, cmap="gray")
plt.axis("off")

plt.subplot(3, 4, 4)
plt.title("4. Bird's Eye View")
plt.imshow(warped, cmap="gray")
plt.axis("off")

plt.subplot(3, 4, 5)
plt.title("5. Voies détectées (Bird's Eye)")
plt.imshow(cv2.cvtColor(lane_warp, cv2.COLOR_BGR2RGB))
plt.axis("off")

plt.subplot(3, 4, 6)
plt.title("6. Résultat final")
plt.imshow(result)
plt.axis("off")

plt.tight_layout()
plt.show()

# ================================================
# 9. Questions de synthèse (bonus)
# ================================================
# Limites : nuit, pluie forte, marquages effacés, virages très serrés, occlusion par véhicules.
# Amélioration temporelle : lissage exponentiel des coefficients polynomiaux entre frames.
# Deep learning : entraînement d'un U-Net ou SCNN pour segmentation sémantique des voies.