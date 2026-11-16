#include <stdio.h>
#include "traitement_image.h"

#define M 4
#define N 4

static void afficher_image(const char *titre,
                           int lignes, int colonnes,
                           unsigned char image[lignes][colonnes])
{
    printf("\n%s\n", titre);
    for (int y = 0; y < lignes; ++y) {
        for (int x = 0; x < colonnes; ++x)
            printf("%3u ", image[y][x]);
        printf("\n");
    }
}

int main(void)
{
    unsigned char A[M][N] = {
        { 20,  40,  60,  80},
        { 40,  80, 120, 160},
        { 60, 120, 180, 220},
        { 80, 160, 220, 255}
    };

    unsigned char B[M][N] = {
        { 10,  20,  30,  40},
        { 20,  40,  60,  80},
        { 30,  60,  90, 120},
        { 40,  80, 120, 160}
    };

    unsigned char R[M][N];

    afficher_image("Image A", M, N, A);
    printf("\nLuminance de A : %.2f\n", ti_luminance(M, N, A));
    printf("Contraste (ecart-type) : %.2f\n",
           ti_contraste_ecart_type(M, N, A));

    ti_addition_images(M, N, A, B, R);
    afficher_image("A + B avec saturation", M, N, R);

    ti_soustraction_images(M, N, A, B, R);
    afficher_image("A - B", M, N, R);

    ti_transformation_gamma(M, N, A, R, 2.0);
    afficher_image("Correction gamma (gamma=2)", M, N, R);

    ti_egalisation_globale(M, N, A, R);
    afficher_image("Egalisation globale", M, N, R);

    /* Noyau moyenneur 3x3 : flou */
    const double noyau_flou[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };

    ti_convolution_2D(M, N, A, R, 3, noyau_flou, 9.0, 0.0);
    afficher_image("Convolution 3x3 moyenneuse (flou)", M, N, R);

    return 0;
}
