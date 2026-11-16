# Documentation de la bibliothèque `traitement_image.h`

**Projet :** bibliothèque pédagogique de traitement d'images en C\
**Version :** première version de travail\
**Type d'images :** niveaux de gris, 8 bits par pixel\
**Plage des intensités :** 0 à 255

------------------------------------------------------------------------

## 1. Présentation

Le fichier `traitement_image.h` regroupe des fonctions de base pour
étudier et expérimenter le traitement d'images en niveaux de gris.

Il contient des fonctions pour :

-   mesurer la luminance et le contraste ;
-   transformer les intensités ;
-   calculer et égaliser les histogrammes ;
-   appliquer des opérations logiques et arithmétiques ;
-   agrandir une image par interpolation ;
-   appliquer une convolution numérique 2D.

Cette bibliothèque est destinée à l'apprentissage. Elle ne remplace pas
une bibliothèque industrielle de traitement d'images.

### 1.1 Fichiers du projet

``` text
projet/
├── traitement_image.h
└── test_traitement_image.c
```

-   `traitement_image.h` contient les fonctions.
-   `test_traitement_image.c` contient un programme d'exemple qui
    appelle plusieurs fonctions.

### 1.2 Convention de représentation

Une image est représentée par une matrice de `unsigned char` :

``` c
unsigned char Image[M][N];
```

-   `M` : nombre de lignes.
-   `N` : nombre de colonnes.
-   `Image[y][x]` : intensité du pixel situé à la ligne `y` et à la
    colonne `x`.
-   `0` : noir.
-   `255` : blanc.
-   Les valeurs entre 0 et 255 représentent les niveaux de gris.

La plupart des fonctions qui produisent une image prennent une image
source et une matrice de destination distincte. Il faut donc prévoir
`Resultat[M][N]`.

### 1.3 Convention de nommage

Les fonctions commencent par le préfixe `ti_`, abréviation de
*traitement image*. Cela permet de distinguer les fonctions de cette
bibliothèque des fonctions standard ou d'autres bibliothèques.

Les fonctions sont déclarées `static inline` dans le fichier d'en-tête.
Il suffit donc d'inclure le fichier dans le programme qui les utilise.

------------------------------------------------------------------------

## 2. Installation et compilation

### 2.1 Inclure la bibliothèque

Dans le fichier C :

``` c
#include <stdio.h>
#include "traitement_image.h"
```

Le fichier `traitement_image.h` doit se trouver dans le même dossier que
le programme, ou dans un dossier indiqué au compilateur.

### 2.2 Compiler le programme de test

Avec GCC :

``` bash
gcc -std=c11 -Wall -Wextra test_traitement_image.c -o test -lm
```

Puis lancer :

``` bash
./test
```

L'option `-lm` lie la bibliothèque mathématique utilisée par `sqrt()` et
`pow()`.

### 2.3 Points à connaître

-   Le code utilise les tableaux de longueur variable (VLA) de C99 ou
    ultérieur.
-   Les dimensions des matrices doivent être connues au moment de
    l'appel.
-   Les fonctions d'image supposent des dimensions positives et des
    matrices correctement dimensionnées.
-   Pour éviter d'écraser les données originales, utiliser une matrice
    de sortie séparée.
-   Plusieurs fonctions retournent immédiatement si certains paramètres
    sont invalides. Pour ces fonctions `void`, aucun message d'erreur
    n'est renvoyé : le programme appelant doit vérifier ses paramètres.

------------------------------------------------------------------------

# 3. Mesures de luminance et de contraste

## 3.1 `ti_luminance()`

**Prototype :**

``` c
double ti_luminance(int M, int N,
                    const unsigned char Image[M][N]);
```

**Rôle :** calculer la moyenne des intensités de tous les pixels.

**Principe :**

\[ L =
`\frac{1}{MN}`{=tex}`\sum`{=tex}*{y=0}\^{M-1}`\sum`{=tex}*{x=0}\^{N-1}
Image\[y\]\[x\] \]

**Paramètres :**

-   `M`, `N` : dimensions de l'image.
-   `Image` : image à analyser.

**Retour :** luminance moyenne sous forme de `double`. Retourne `0.0` si
une dimension est non positive.

**Test :**

``` c
unsigned char I[2][2] = {
    {10, 20},
    {30, 40}
};

printf("Luminance = %.2f\n", ti_luminance(2, 2, I));
// Résultat attendu : 25.00
```

## 3.2 `ti_contraste_ecart_type()`

**Prototype :**

``` c
double ti_contraste_ecart_type(int M, int N,
                               const unsigned char Image[M][N]);
```

**Rôle :** mesurer la dispersion des intensités autour de la luminance
moyenne, au moyen de l'écart-type.

\[ C = `\sqrt{\frac{1}{MN}
\sum_y\sum_x (Image[y][x]-L)^2}`{=tex} \]

Un écart-type élevé indique une plus grande dispersion des intensités.
La fonction utilise `sqrt()`.

**Test :**

``` c
unsigned char I[2][2] = {
    {10, 20},
    {30, 40}
};

printf("Contraste = %.2f\n",
       ti_contraste_ecart_type(2, 2, I));
```

Pour vérifier le comportement, comparer cette image à une image uniforme
:

``` c
unsigned char uniforme[2][2] = {
    {100, 100},
    {100, 100}
};
```

L'écart-type de l'image uniforme doit être nul.

## 3.3 `ti_contraste_min_max()`

**Prototype :**

``` c
double ti_contraste_min_max(int M, int N,
                            const unsigned char Image[M][N]);
```

**Rôle :** calculer l'indicateur de contraste min-max utilisé dans notre
cours :

\[ C = `\frac{I_{\max}-I_{\min}}{I_{\max}+I_{\min}}`{=tex} \]

La fonction recherche d'abord l'intensité minimale et l'intensité
maximale.

**Cas particulier :** si le dénominateur est nul, la fonction retourne
`0.0`.

**Test :**

``` c
unsigned char I[2][2] = {
    {20, 40},
    {60, 100}
};

printf("Contraste min-max = %.4f\n",
       ti_contraste_min_max(2, 2, I));
```

------------------------------------------------------------------------

# 4. Transformations des intensités

## 4.1 `ti_transformation_lineaire()`

**Prototype :**

``` c
void ti_transformation_lineaire(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N]);
```

**Rôle :** étendre linéairement les intensités réellement présentes dans
l'image vers la plage 0--255.

La fonction recherche `minv` et `maxv`, puis applique :

\[ I' = 255 `\times `{=tex}`\frac{I-I_{\min}}{I_{\max}-I_{\min}}`{=tex}
\]

**Cas particulier :** si tous les pixels ont la même intensité
(`maxv == minv`), la fonction copie l'image au lieu de diviser par zéro.

**Test :**

``` c
unsigned char I[1][3] = {{40, 80, 120}};
unsigned char R[1][3];

ti_transformation_lineaire(1, 3, I, R);
// Résultat attendu : environ {0, 128, 255}
```

## 4.2 `ti_transformation_saturation()`

**Prototype :**

``` c
void ti_transformation_saturation(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    int Smin, int Smax);
```

**Rôle :** appliquer une transformation linéaire avec saturation.

-   Si `I <= Smin`, le résultat vaut 0.
-   Si `I >= Smax`, le résultat vaut 255.
-   Entre les deux seuils, l'intensité est transformée linéairement.

\[ I' = 255 `\times `{=tex}`\frac{I-S_{\min}}{S_{\max}-S_{\min}}`{=tex}
\]

**Contraintes :** `0 <= Smin < Smax <= 255`.

**Test :**

``` c
unsigned char I[1][5] = {{20, 50, 75, 150, 220}};
unsigned char R[1][5];

ti_transformation_saturation(1, 5, I, R, 50, 150);
// Résultat attendu : {0, 0, environ 64, 255, 255}
```

## 4.3 `ti_transformation_lineaire_par_morceaux()`

**Prototype :**

``` c
int ti_transformation_lineaire_par_morceaux(
    int r, int r1, int s1, int r2, int s2);
```

**Rôle :** transformer une seule intensité `r` au moyen de trois
segments définis par les points de contrôle `(0,0)`, `(r1,s1)`,
`(r2,s2)` et `(255,255)`.

La fonction choisit la formule correspondant à la portion dans laquelle
se trouve `r`.

**Contraintes :**

-   `0 < r1 < r2 < 255`
-   `s1` et `s2` doivent être compris entre 0 et 255.

**Retour :** intensité transformée, ou `-1` si les paramètres sont
invalides.

**Test :**

``` c
int resultat = ti_transformation_lineaire_par_morceaux(
    130, 80, 40, 180, 220);

printf("%d\n", resultat);
// Résultat attendu : 130
```

Cette fonction transforme un pixel, pas une image entière. Pour traiter
une image, il faut l'appeler dans deux boucles parcourant les lignes et
les colonnes.

## 4.4 `ti_transformation_gamma()`

**Prototype :**

``` c
void ti_transformation_gamma(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    double gamma);
```

**Rôle :** appliquer la correction gamma à chaque pixel.

La fonction normalise l'intensité entre 0 et 1, applique la puissance,
puis revient à la plage 0--255 :

\[ I' = 255
`\left`{=tex}(`\frac{I}{255}`{=tex}`\right`{=tex})\^{1/`\gamma`{=tex}}
\]

**Contrainte :** `gamma > 0`.

**Test :**

``` c
unsigned char I[1][3] = {{0, 64, 255}};
unsigned char R[1][3];

ti_transformation_gamma(1, 3, I, R, 2.0);
```

Avec la convention de cette fonction, `gamma = 2.0` éclaircit
généralement les intensités intermédiaires. La valeur 0 reste 0 et 255
reste 255.

## 4.5 `ti_correction_dynamique()`

**Prototype :**

``` c
void ti_correction_dynamique(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N]);
```

**Rôle :** corriger la dynamique de l'image par étirement min-max.

**Implémentation :** cette fonction appelle
`ti_transformation_lineaire()`. Elle produit donc le même résultat.

**Test :**

``` c
ti_correction_dynamique(M, N, Image, Resultat);
```

------------------------------------------------------------------------

# 5. Histogramme et égalisation

## 5.1 `ti_calcul_histogramme()`

**Prototype :**

``` c
void ti_calcul_histogramme(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned int H[256]);
```

**Rôle :** compter le nombre de pixels pour chacune des 256 intensités
possibles.

-   `H[0]` contient le nombre de pixels noirs.
-   `H[255]` contient le nombre de pixels blancs.
-   La somme des 256 cases doit être égale à `M * N`.

**Test :**

``` c
unsigned char I[2][2] = {
    {0, 10},
    {10, 255}
};
unsigned int H[256];

ti_calcul_histogramme(2, 2, I, H);

printf("Pixels d'intensite 10 : %u\n", H[10]);
// Résultat attendu : 2
```

## 5.2 `ti_egalisation_globale()`

**Prototype :**

``` c
void ti_egalisation_globale(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N]);
```

**Rôle :** améliorer la répartition des niveaux de gris en utilisant
l'histogramme de toute l'image.

Étapes effectuées :

1.  Calcul de l'histogramme.
2.  Normalisation par le nombre total de pixels.
3.  Calcul de la somme cumulative des fréquences.
4.  Transformation de chaque pixel avec `255 * C[intensité]`.

**Test :**

``` c
ti_egalisation_globale(M, N, Image, Resultat);
```

La fonction applique la formule cumulative présentée dans le cours. Elle
n'utilise pas de correction `CDF_min`.

## 5.3 `ti_egalisation_locale()`

**Prototype :**

``` c
void ti_egalisation_locale(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    int tailleFenetre);
```

**Rôle :** égaliser localement l'histogramme en calculant la
distribution des intensités dans le voisinage de chaque pixel.

**Paramètre supplémentaire :** `tailleFenetre`, qui doit être un entier
impair positif. Pour reproduire l'exemple du cours, utiliser `7`
(fenêtre 7 × 7).

Aux bords de l'image, seuls les pixels réellement présents sont pris en
compte. La fonction ne complète pas artificiellement la fenêtre.

**Test :**

``` c
ti_egalisation_locale(M, N, Image, Resultat, 7);
```

**Remarque :** cette implémentation pédagogique recalcule l'histogramme
local pour chaque pixel. Elle est donc plus coûteuse que l'égalisation
globale.

------------------------------------------------------------------------

# 6. Opérations logiques sur les images binaires

Les quatre fonctions suivantes travaillent sur des masques binaires.
Toute valeur non nulle est considérée comme vraie et le résultat est
codé en 0 ou 255.

## 6.1 `ti_image_and()`

**Prototype :**

``` c
void ti_image_and(int M, int N,
    const unsigned char A[M][N],
    const unsigned char B[M][N],
    unsigned char R[M][N]);
```

**Rôle :** ET logique pixel par pixel. Le résultat vaut 255 si les deux
pixels sont non nuls, sinon 0.

**Test :**

``` c
unsigned char A[1][2] = {{255, 0}};
unsigned char B[1][2] = {{255, 255}};
unsigned char R[1][2];

ti_image_and(1, 2, A, B, R);
// Résultat attendu : {255, 0}
```

## 6.2 `ti_image_or()`

**Rôle :** OU logique. Le résultat vaut 255 si au moins un pixel
correspondant est non nul.

``` c
ti_image_or(M, N, A, B, R);
```

## 6.3 `ti_image_not()`

**Rôle :** inversion logique d'un masque : 0 devient 255 et toute valeur
non nulle devient 0.

``` c
ti_image_not(M, N, A, R);
```

## 6.4 `ti_image_xor()`

**Rôle :** OU exclusif. Le résultat vaut 255 si un seul des deux pixels
est non nul, et 0 s'ils ont le même état logique.

``` c
ti_image_xor(M, N, A, B, R);
```

**Attention :** ces fonctions réalisent des opérations logiques sur des
masques. Elles ne réalisent pas un ET ou un OU arithmétique sur les
intensités en niveaux de gris.

------------------------------------------------------------------------

# 7. Opérations arithmétiques sur les images

## 7.1 `ti_addition_images()`

**Prototype :**

``` c
void ti_addition_images(int M, int N,
    const unsigned char A[M][N],
    const unsigned char B[M][N],
    unsigned char R[M][N]);
```

**Rôle :** additionner les pixels correspondants. Le résultat est saturé
à 255.

\[ R(x,y)=`\min`{=tex}(A(x,y)+B(x,y),255) \]

**Test :**

``` c
unsigned char A[1][3] = {{100, 200, 250}};
unsigned char B[1][3] = {{50, 100, 20}};
unsigned char R[1][3];

ti_addition_images(1, 3, A, B, R);
// Résultat attendu : {150, 255, 255}
```

Les deux images doivent avoir les mêmes dimensions.

## 7.2 `ti_soustraction_images()`

**Rôle :** soustraire B à A pixel par pixel. Les résultats négatifs sont
ramenés à zéro.

\[ R(x,y)=`\max`{=tex}(A(x,y)-B(x,y),0) \]

**Test :**

``` c
ti_soustraction_images(1, 3, A, B, R);
```

Exemple : `A = {100, 50}` et `B = {30, 80}` donnent `{70, 0}`.

## 7.3 `ti_multiplication_image()`

**Prototype :**

``` c
void ti_multiplication_image(int M, int N,
    const unsigned char A[M][N],
    double ratio,
    unsigned char R[M][N]);
```

**Rôle :** multiplier chaque intensité par un facteur `ratio`. Le
résultat est limité à 255.

**Test :**

``` c
unsigned char A[1][3] = {{50, 100, 200}};
unsigned char R[1][3];

ti_multiplication_image(1, 3, A, 1.5, R);
// Résultat attendu : {75, 150, 255}
```

La fonction ne fait rien si le ratio est négatif.

## 7.4 `ti_fusion_images()`

**Prototype :**

``` c
void ti_fusion_images(int M, int N,
    const unsigned char A[M][N],
    const unsigned char B[M][N],
    double alpha,
    unsigned char R[M][N]);
```

**Rôle :** effectuer une combinaison pondérée de deux images :

\[ R=`\alpha `{=tex}A+(1-`\alpha`{=tex})B \]

**Contrainte :** `0 <= alpha <= 1`.

**Test :**

``` c
unsigned char A[1][1] = {{100}};
unsigned char B[1][1] = {{200}};
unsigned char R[1][1];

ti_fusion_images(1, 1, A, B, 0.5, R);
// Résultat attendu : 150
```

`alpha = 0.5` donne une contribution de 50 % pour chaque image.

------------------------------------------------------------------------

# 8. Interpolation et changement d'échelle

Les fonctions d'interpolation présentes dans cette version réalisent un
agrandissement par un facteur entier positif. La matrice de sortie doit
avoir les dimensions `M * facteur` par `N * facteur`.

## 8.1 `ti_interpolation_plus_proche_voisin()`

**Prototype :**

``` c
void ti_interpolation_plus_proche_voisin(
    int M, int N, int facteur,
    const unsigned char Image[M][N],
    unsigned char R[M * facteur][N * facteur]);
```

**Rôle :** agrandir l'image en recopiant le pixel source le plus proche.

**Test :**

``` c
unsigned char I[2][2] = {
    {12, 14},
    {15, 17}
};
unsigned char R[4][4];

ti_interpolation_plus_proche_voisin(2, 2, 2, I, R);
```

Chaque pixel source est reproduit dans un bloc de 2 × 2 pixels.

**Résultat :**

``` text
12 12 14 14
12 12 14 14
15 15 17 17
15 15 17 17
```

## 8.2 `ti_interpolation_bilineaire()`

**Prototype :**

``` c
void ti_interpolation_bilineaire(
    int M, int N, int facteur,
    const unsigned char Image[M][N],
    unsigned char R[M * facteur][N * facteur]);
```

**Rôle :** calculer les nouveaux pixels par interpolation pondérée des
quatre pixels voisins.

La fonction utilise les coordonnées fractionnaires de la position source
et une combinaison des quatre intensités.

**Test :**

``` c
unsigned char I[2][2] = {
    {100, 120},
    {140, 160}
};
unsigned char R[4][4];

ti_interpolation_bilineaire(2, 2, 2, I, R);
```

Les bords sont traités en répétant la dernière ligne ou la dernière
colonne disponible.

## 8.3 `ti_interpolation_cubique_1D()`

**Prototype :**

``` c
double ti_interpolation_cubique_1D(
    double xm1, double x0, double x1, double x2);
```

**Rôle :** calculer un point intermédiaire à mi-distance à partir de
quatre échantillons, selon la formule pédagogique présentée dans le
cours :

\[ y\_{i+1/2} = `\frac{-x_{i-1}+9x_i+9x_{i+1}-x_{i+2}}{16}`{=tex} \]

**Test :**

``` c
double valeur = ti_interpolation_cubique_1D(10, 20, 40, 50);
printf("%.2f\n", valeur);
// Résultat : 30.00
```

**Limite importante :** cette fonction est uniquement le calcul 1D de la
formule à mi-distance. Elle n'est pas une fonction complète de
redimensionnement bicubique 2D. Une fonction
`ti_interpolation_bicubique()` complète reste à développer.

------------------------------------------------------------------------

# 9. Convolution numérique 2D

## 9.1 `ti_convolution_2D()`

**Prototype :**

``` c
void ti_convolution_2D(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    int K,
    const double noyau[K][K],
    double normalisation,
    double biais);
```

**Rôle :** appliquer un noyau de convolution carré à chaque pixel de
l'image.

La fonction :

1.  place le centre du noyau sur le pixel traité ;
2.  multiplie les pixels voisins par les coefficients du noyau ;
3.  additionne les produits ;
4.  divise par `normalisation` ;
5.  ajoute `biais` ;
6.  limite le résultat entre 0 et 255.

**Paramètres :**

-   `M`, `N` : dimensions de l'image.
-   `Image` : image source.
-   `Resultat` : image filtrée.
-   `K` : taille du noyau, obligatoirement impaire et positive.
-   `noyau` : matrice des coefficients.
-   `normalisation` : diviseur du résultat, différent de zéro.
-   `biais` : valeur ajoutée après normalisation.

La fonction retourne les résultats dans la plage 0--255. Elle traite les
bords par répétition du pixel périphérique (*clamp*). Elle retourne
immédiatement si `K` est pair, inférieur à 1, ou si la normalisation
vaut zéro.

**Important :** le code retourne le noyau dans les deux dimensions avant
de l'appliquer. C'est la convention de la convolution mathématique. Pour
les noyaux symétriques, comme le filtre moyenneur, cette inversion ne
change pas le résultat.

### Exemple 1 : filtre moyenneur 3 × 3

``` c
const double flou[3][3] = {
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1}
};

ti_convolution_2D(M, N, Image, Resultat,
                  3, flou, 9.0, 0.0);
```

Ce noyau calcule une moyenne locale et produit un effet de flou.

### Exemple 2 : filtre de netteté

``` c
const double nettete[3][3] = {
    { 0, -1,  0},
    {-1,  5, -1},
    { 0, -1,  0}
};

ti_convolution_2D(M, N, Image, Resultat,
                  3, nettete, 1.0, 0.0);
```

Ce noyau accentue les variations locales. Les valeurs négatives peuvent
produire des sommes négatives, qui sont ensuite limitées à zéro.

### Exemple 3 : Sobel horizontal

``` c
const double sobel_x[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

ti_convolution_2D(M, N, Image, Resultat,
                  3, sobel_x, 1.0, 128.0);
```

Le biais 128 permet de décaler les valeurs signées vers la plage
affichable. Cette sortie est une visualisation du gradient horizontal,
pas une magnitude complète du gradient.

------------------------------------------------------------------------

# 10. Programme de test fourni

Le fichier `test_traitement_image.c` contient un exemple d'utilisation
de la bibliothèque.

Il :

1.  crée deux matrices 4 × 4 ;
2.  affiche l'image A ;
3.  calcule sa luminance et son contraste par écart-type ;
4.  teste l'addition ;
5.  teste la soustraction ;
6.  teste la correction gamma ;
7.  teste l'égalisation globale ;
8.  applique un noyau moyenneur 3 × 3 par convolution.

Le programme de test ne teste pas encore toutes les fonctions. Les
exemples de cette documentation peuvent être ajoutés progressivement à
`main()`.

### 10.1 Afficher une matrice

Le programme utilise une fonction auxiliaire `afficher_image()` pour
afficher les intensités sous forme de tableau.

Pour tester une nouvelle opération :

1.  créer l'image source ;
2.  créer une matrice de sortie de mêmes dimensions ;
3.  appeler la fonction ;
4.  afficher la matrice de sortie ;
5.  comparer le résultat aux valeurs attendues.

### 10.2 Tester plusieurs fonctions

Exemple :

``` c
unsigned char resultat[M][N];

ti_image_and(M, N, A, B, resultat);
afficher_image("A AND B", M, N, resultat);

ti_transformation_lineaire(M, N, A, resultat);
afficher_image("Transformation lineaire", M, N, resultat);
```

### 10.3 Vérifier les résultats automatiquement

Pour les petits tableaux, il est utile d'ajouter des assertions :

``` c
#include <assert.h>

assert(R[0][0] == 150);
```

Si la condition est fausse, le programme s'arrête au point de
l'assertion. Cela permet de repérer rapidement une erreur de calcul.

------------------------------------------------------------------------

# 11. Inventaire des fonctions

  Domaine           Fonction
  ----------------- ---------------------------------------------
  Mesures           `ti_luminance()`
  Mesures           `ti_contraste_ecart_type()`
  Mesures           `ti_contraste_min_max()`
  Transformations   `ti_transformation_lineaire()`
  Transformations   `ti_transformation_saturation()`
  Transformations   `ti_transformation_lineaire_par_morceaux()`
  Transformations   `ti_transformation_gamma()`
  Dynamique         `ti_correction_dynamique()`
  Histogrammes      `ti_calcul_histogramme()`
  Histogrammes      `ti_egalisation_globale()`
  Histogrammes      `ti_egalisation_locale()`
  Logique           `ti_image_and()`
  Logique           `ti_image_or()`
  Logique           `ti_image_not()`
  Logique           `ti_image_xor()`
  Arithmétique      `ti_addition_images()`
  Arithmétique      `ti_soustraction_images()`
  Arithmétique      `ti_multiplication_image()`
  Arithmétique      `ti_fusion_images()`
  Interpolation     `ti_interpolation_plus_proche_voisin()`
  Interpolation     `ti_interpolation_bilineaire()`
  Interpolation     `ti_interpolation_cubique_1D()`
  Filtrage          `ti_convolution_2D()`

------------------------------------------------------------------------

# 12. Limites et prochaines améliorations

Cette bibliothèque est une base pédagogique évolutive. Plusieurs
éléments pourront être améliorés après les révisions du chapitre 2 :

-   ajouter une véritable fonction de redimensionnement bicubique 2D ;
-   améliorer la gestion des erreurs en retournant des codes de statut ;
-   ajouter la lecture et l'écriture de fichiers image ;
-   ajouter des tests automatiques pour chaque fonction ;
-   optimiser l'égalisation locale ;
-   développer d'autres noyaux de convolution : gaussien, Laplacien,
    Sobel vertical et autres ;
-   étudier les conventions de bord et les différences entre corrélation
    et convolution.

**État actuel :** première version de travail. Les fonctions décrites
correspondent à celles présentes dans `traitement_image.h`. Les
fonctions annoncées comme restant à développer ne sont pas encore
implémentées.

------------------------------------------------------------------------

## 13. Ordre conseillé pour apprendre et tester

Pour bien maîtriser la bibliothèque, il est conseillé de tester les
fonctions dans cet ordre :

1.  `ti_luminance()` et les deux mesures de contraste.
2.  Les transformations linéaires et gamma.
3.  Le calcul et l'égalisation des histogrammes.
4.  Les opérations logiques sur des masques binaires.
5.  L'addition, la soustraction et la fusion.
6.  Les deux méthodes d'interpolation déjà implémentées.
7.  La convolution avec le filtre moyenneur, puis avec les filtres de
    netteté et Sobel.

L'objectif est de comprendre chaque fonction avec de petites matrices
dont les valeurs peuvent être calculées à la main avant de travailler
sur de vraies images.
