#include <vector>
#include <iostream>
//headers
#include "matrix.h"

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
