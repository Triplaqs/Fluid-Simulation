#include <vector>
#include <iostream>
//headers
#include "matrix.h"
#include <math.h>


//affiche la matrice dans le terminal 
void printMatrix(const ImageMatrix& imgMatrix) {
    for (int y = 0; y < imgMatrix.getHeight(); ++y) {
        for (int x = 0; x < imgMatrix.getWidth(); ++x) {
            std::cout << imgMatrix.getPixel(x, y) << " ";
        }
        std::cout << std::endl;
    }
};


//Calcul le produit de convolution de input et kernel (varie selon le filtre appliqué) et le renvoie (effet de bord) dans output
void convolutionMatrix(const ImageMatrix& input, ImageMatrix& output, std::vector<std::vector<float>> kernel) {
    //std::vector<std::vector<float>> kernel = {
        //{1/16,2/16,1/16},
        //{2/16,4/16,2/16},
        //{1/16,2/16,1/16}
    //};// noyau de flou 3x3 le matrice de convolution
    int kw = kernel[0].size();
    int kh = kernel.size();
    int kHalfW = kw / 2;
    int kHalfH = kh / 2;

    for (int y = 0; y < input.getHeight(); ++y) {
        for (int x = 0; x < input.getWidth(); ++x) {
            float sum = 0.0f;
            for (int ky = -kHalfH; ky <= kHalfH; ++ky) {
                for (int kx = -kHalfW; kx <= kHalfW; ++kx) {
                    int ix = x + kx;
                    int iy = y + ky;
                    if (ix >= 0 && ix < input.getWidth() && iy >= 0 && iy < input.getHeight()) {
                        sum += input.getPixel(ix, iy) * kernel[ky + kHalfH][kx + kHalfW];
                    }
                }
            }
            output.setPixel(x, y, sum);
        }
    }
};
void afficheTableauDeCellules(const ImageMatrix& imgMatrix) {
    for (int y = 0; y < imgMatrix.getHeight(); ++y) {
        for (int x = 0; x < imgMatrix.getWidth(); ++x) {
            float val = imgMatrix.getPixel(x, y);
            if (val > 0.5f) {
                std::cout << "1 ";
            } else {
                std::cout << "0 ";
            }
        }
        std::cout << std::endl;
    }
};

void gradientSobel(const ImageMatrix& input, ImageMatrix& output) {
    std::vector<std::vector<float>> sobelX = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    std::vector<std::vector<float>> sobelY = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

    ImageMatrix gradX(input.getWidth(), input.getHeight());
    ImageMatrix gradY(input.getWidth(), input.getHeight());

    convolutionMatrix(input, gradX, sobelX);
    convolutionMatrix(input, gradY, sobelY);

    for (int y = 0; y < input.getHeight(); ++y) {
        for (int x = 0; x < input.getWidth(); ++x) {
            float gx = gradX.getPixel(x, y);
            float gy = gradY.getPixel(x, y);
            float magnitude = std::sqrt(gx * gx + gy * gy);
            output.setPixel(x, y, magnitude);
        }
    }
};
