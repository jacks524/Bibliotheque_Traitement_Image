#ifndef TRAITEMENT_IMAGE_H
#define TRAITEMENT_IMAGE_H

/*
 * Bibliotheque pedagogique de traitement d'images
 * Images en niveaux de gris sur 8 bits : [0, 255].
 *
 * Convention :
 *   M = nombre de lignes
 *   N = nombre de colonnes
 *   Image[y][x] = pixel de la ligne y et colonne x
 *
 * Les fonctions produisent une image Resultat distincte.
 * Les dimensions des tableaux doivent etre connues a la compilation
 * (VLA C99 ou plus recent).
 */

#include <stddef.h>
#include <math.h>

/* ---------- Mesures et contraste ---------- */

static inline double ti_luminance(int M, int N,
                                  const unsigned char Image[M][N])
{
    if (M <= 0 || N <= 0) return 0.0;
    double somme = 0.0;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x)
            somme += Image[y][x];
    return somme / ((double)M * N);
}

static inline double ti_contraste_ecart_type(int M, int N,
                                              const unsigned char Image[M][N])
{
    if (M <= 0 || N <= 0) return 0.0;
    double moyenne = ti_luminance(M, N, Image);
    double somme = 0.0;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            double d = Image[y][x] - moyenne;
            somme += d * d;
        }
    return sqrt(somme / ((double)M * N));
}

/* Contraste min-max tel que vu dans le cours : (max-min)/(max+min).
   Retourne 0 si max+min vaut 0. */
static inline double ti_contraste_min_max(int M, int N,
                                           const unsigned char Image[M][N])
{
    if (M <= 0 || N <= 0) return 0.0;
    int minv = 255, maxv = 0;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            int v = Image[y][x];
            if (v < minv) minv = v;
            if (v > maxv) maxv = v;
        }
    if (maxv + minv == 0) return 0.0;
    return (double)(maxv - minv) / (maxv + minv);
}

/* ---------- Transformations ponctuelles ---------- */

static inline void ti_transformation_lineaire(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N])
{
    if (M <= 0 || N <= 0) return;
    int minv = 255, maxv = 0;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            int v = Image[y][x];
            if (v < minv) minv = v;
            if (v > maxv) maxv = v;
        }

    if (maxv == minv) {
        for (int y = 0; y < M; ++y)
            for (int x = 0; x < N; ++x)
                Resultat[y][x] = Image[y][x];
        return;
    }

    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            double v = 255.0 * (Image[y][x] - minv) / (maxv - minv);
            Resultat[y][x] = (unsigned char)(v + 0.5);
        }
}

static inline void ti_transformation_saturation(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    int Smin, int Smax)
{
    if (M <= 0 || N <= 0 || Smin < 0 || Smax > 255 || Smin >= Smax)
        return;

    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            int I = Image[y][x];
            if (I <= Smin) Resultat[y][x] = 0;
            else if (I >= Smax) Resultat[y][x] = 255;
            else {
                double v = 255.0 * (I - Smin) / (Smax - Smin);
                Resultat[y][x] = (unsigned char)(v + 0.5);
            }
        }
}

static inline int ti_transformation_lineaire_par_morceaux(
    int r, int r1, int s1, int r2, int s2)
{
    if (r1 <= 0 || r1 >= r2 || r2 >= 255 ||
        s1 < 0 || s1 > 255 || s2 < 0 || s2 > 255)
        return -1;

    double v;
    if (r <= r1)
        v = (double)s1 * r / r1;
    else if (r <= r2)
        v = s1 + (double)(s2 - s1) * (r - r1) / (r2 - r1);
    else
        v = s2 + (double)(255 - s2) * (r - r2) / (255 - r2);

    if (v < 0.0) v = 0.0;
    if (v > 255.0) v = 255.0;
    return (int)(v + 0.5);
}

static inline void ti_transformation_gamma(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    double gamma)
{
    if (M <= 0 || N <= 0 || gamma <= 0.0) return;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            double n = Image[y][x] / 255.0;
            double v = 255.0 * pow(n, 1.0 / gamma);
            Resultat[y][x] = (unsigned char)(v + 0.5);
        }
}

/* Correction de dynamique = etirement min-max. */
static inline void ti_correction_dynamique(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N])
{
    ti_transformation_lineaire(M, N, Image, Resultat);
}

/* ---------- Histogrammes ---------- */

static inline void ti_calcul_histogramme(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned int H[256])
{
    for (int i = 0; i < 256; ++i) H[i] = 0;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x)
            H[Image[y][x]]++;
}

static inline void ti_egalisation_globale(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N])
{
    if (M <= 0 || N <= 0) return;
    unsigned int H[256];
    double C[256];
    ti_calcul_histogramme(M, N, Image, H);

    double cumul = 0.0;
    double total = (double)M * N;
    for (int i = 0; i < 256; ++i) {
        cumul += H[i] / total;
        C[i] = cumul;
    }

    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            double v = 255.0 * C[Image[y][x]];
            Resultat[y][x] = (unsigned char)(v + 0.5);
        }
}

/* Egalisation locale avec fenetre impaire (ex. 7). Aux bords,
   seuls les pixels existants sont comptabilises. */
static inline void ti_egalisation_locale(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    int tailleFenetre)
{
    if (M <= 0 || N <= 0 || tailleFenetre < 1 ||
        tailleFenetre % 2 == 0) return;

    int rayon = tailleFenetre / 2;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            unsigned int H[256] = {0};
            int nb = 0;

            for (int dy = -rayon; dy <= rayon; ++dy)
                for (int dx = -rayon; dx <= rayon; ++dx) {
                    int yy = y + dy, xx = x + dx;
                    if (yy >= 0 && yy < M && xx >= 0 && xx < N) {
                        H[Image[yy][xx]]++;
                        nb++;
                    }
                }

            unsigned int cumul = 0;
            int I = Image[y][x];
            for (int i = 0; i <= I; ++i) cumul += H[i];

            double v = 255.0 * cumul / nb;
            Resultat[y][x] = (unsigned char)(v + 0.5);
        }
}

/* ---------- Operations logiques sur images binaires ---------- */

static inline void ti_image_and(int M, int N,
    const unsigned char A[M][N], const unsigned char B[M][N],
    unsigned char R[M][N])
{
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x)
            R[y][x] = (A[y][x] != 0 && B[y][x] != 0) ? 255 : 0;
}

static inline void ti_image_or(int M, int N,
    const unsigned char A[M][N], const unsigned char B[M][N],
    unsigned char R[M][N])
{
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x)
            R[y][x] = (A[y][x] != 0 || B[y][x] != 0) ? 255 : 0;
}

static inline void ti_image_not(int M, int N,
    const unsigned char A[M][N], unsigned char R[M][N])
{
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x)
            R[y][x] = (A[y][x] == 0) ? 255 : 0;
}

static inline void ti_image_xor(int M, int N,
    const unsigned char A[M][N], const unsigned char B[M][N],
    unsigned char R[M][N])
{
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x)
            R[y][x] = ((A[y][x] != 0) != (B[y][x] != 0)) ? 255 : 0;
}

/* ---------- Operations arithmetiques ---------- */

static inline void ti_addition_images(int M, int N,
    const unsigned char A[M][N], const unsigned char B[M][N],
    unsigned char R[M][N])
{
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            int v = A[y][x] + B[y][x];
            R[y][x] = (unsigned char)(v > 255 ? 255 : v);
        }
}

static inline void ti_soustraction_images(int M, int N,
    const unsigned char A[M][N], const unsigned char B[M][N],
    unsigned char R[M][N])
{
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            int v = (int)A[y][x] - (int)B[y][x];
            R[y][x] = (unsigned char)(v < 0 ? 0 : v);
        }
}

static inline void ti_multiplication_image(int M, int N,
    const unsigned char A[M][N], double ratio,
    unsigned char R[M][N])
{
    if (ratio < 0.0) return;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            double v = A[y][x] * ratio;
            if (v > 255.0) v = 255.0;
            R[y][x] = (unsigned char)(v + 0.5);
        }
}

static inline void ti_fusion_images(int M, int N,
    const unsigned char A[M][N], const unsigned char B[M][N],
    double alpha, unsigned char R[M][N])
{
    if (alpha < 0.0 || alpha > 1.0) return;
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            double v = alpha * A[y][x] + (1.0 - alpha) * B[y][x];
            R[y][x] = (unsigned char)(v + 0.5);
        }
}

/* ---------- Interpolation / changement d'echelle ---------- */

/* Agrandissement par facteur entier >= 1. */
static inline void ti_interpolation_plus_proche_voisin(
    int M, int N, int facteur,
    const unsigned char Image[M][N],
    unsigned char R[M * facteur][N * facteur])
{
    if (facteur < 1) return;
    for (int y = 0; y < M * facteur; ++y)
        for (int x = 0; x < N * facteur; ++x)
            R[y][x] = Image[y / facteur][x / facteur];
}

/* Bilineaire, facteur entier >= 1. Bords traites par duplication
   du dernier pixel/ligne disponible. */
static inline void ti_interpolation_bilineaire(
    int M, int N, int facteur,
    const unsigned char Image[M][N],
    unsigned char R[M * facteur][N * facteur])
{
    if (facteur < 1) return;
    for (int y = 0; y < M * facteur; ++y)
        for (int x = 0; x < N * facteur; ++x) {
            double sy = (double)y / facteur;
            double sx = (double)x / facteur;
            int y0 = (int)sy, x0 = (int)sx;
            int y1 = (y0 + 1 < M) ? y0 + 1 : y0;
            int x1 = (x0 + 1 < N) ? x0 + 1 : x0;
            double a = sx - x0, b = sy - y0;

            double v =
                (1-a)*(1-b)*Image[y0][x0] +
                a*(1-b)*Image[y0][x1] +
                (1-a)*b*Image[y1][x0] +
                a*b*Image[y1][x1];

            R[y][x] = (unsigned char)(v + 0.5);
        }
}

/* Formule 1D de demi-pixel du support de cours.
   Cette fonction est un noyau pedagogique, pas un resize bicubique 2D complet. */
static inline double ti_interpolation_cubique_1D(
    double xm1, double x0, double x1, double x2)
{
    return (-xm1 + 9.0*x0 + 9.0*x1 - x2) / 16.0;
}

/* ---------- Convolution numerique 2D ---------- */

/*
 * Convolution avec un noyau impair (ex. 3x3 ou 5x5).
 * Bords : extension par repetition du pixel de bord (clamp).
 * Le resultat est borne a [0,255].
 * IMPORTANT : un noyau de flou, de nettete ou de detection de contours
 * peut avoir une somme differente de 1. Le facteur normalisation permet
 * de diviser la somme calculee (mettre 1.0 si aucune normalisation).
 */
static inline void ti_convolution_2D(
    int M, int N,
    const unsigned char Image[M][N],
    unsigned char Resultat[M][N],
    int K,
    const double noyau[K][K],
    double normalisation,
    double biais)
{
    if (M <= 0 || N <= 0 || K < 1 || K % 2 == 0 ||
        normalisation == 0.0) return;

    int rayon = K / 2;

    for (int y = 0; y < M; ++y)
        for (int x = 0; x < N; ++x) {
            double somme = 0.0;

            for (int ky = 0; ky < K; ++ky)
                for (int kx = 0; kx < K; ++kx) {
                    int yy = y + ky - rayon;
                    int xx = x + kx - rayon;

                    /* Extension par repetition des bords */
                    if (yy < 0) yy = 0;
                    if (yy >= M) yy = M - 1;
                    if (xx < 0) xx = 0;
                    if (xx >= N) xx = N - 1;

                    /* Noyau retourne : convolution mathematique */
                    double coefficient = noyau[K - 1 - ky][K - 1 - kx];
                    somme += Image[yy][xx] * coefficient;
                }

            double v = somme / normalisation + biais;
            if (v < 0.0) v = 0.0;
            if (v > 255.0) v = 255.0;
            Resultat[y][x] = (unsigned char)(v + 0.5);
        }
}

#endif /* TRAITEMENT_IMAGE_H */
