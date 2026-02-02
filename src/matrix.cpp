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
// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"
/*
int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, S;
  
  if (argc != 3) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite);

   OCTET *ImgIn, *ImgOut;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
   nTaille = nH * nW;
  
   allocation_tableau(ImgIn, OCTET, nTaille);
   lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
   allocation_tableau(ImgOut, OCTET, nTaille);
	
   //   for (int i=0; i < nTaille; i++)
   // {
   //  if ( ImgIn[i] < S) ImgOut[i]=0; else ImgOut[i]=255;
   //  }

    ImageMatrix inputImage(nW, nH);
    ImageMatrix outputImage(nW, nH);
    // Appliquer la convolution avec un noyau de flou
    std::vector<std::vector<float>> blurKernel = {
        {1/16.0f, 2/16.0f, 1/16.0f},
        {2/16.0f, 4/16.0f, 2/16.0f},
        {1/16.0f, 2/16.0f, 1/16.0f}
    };
// remplir inputImage depuis ImgIn (OCTET values)
    for (int y = 0; y < nH; ++y) {
        for (int x = 0; x < nW; ++x) {
            int idx = y * nW + x;
            inputImage.setPixel(x, y, (float)ImgIn[idx]);
        }
    }

    // Appliquer la convolution avec le noyau de flou
    convolutionMatrix(inputImage, outputImage, blurKernel);

    // copier outputImage dans ImgOut (avec clamp 0..255)
    for (int y = 0; y < nH; ++y) {
        for (int x = 0; x < nW; ++x) {
            int idx = y * nW + x;
            float v = outputImage.getPixel(x, y);
            if (v < 0.0f) v = 0.0f;
            if (v > 255.0f) v = 255.0f;
            ImgOut[idx] = (OCTET)(v + 0.5f);
        }
    }
    printMatrix(inputImage);
    std::cout << "------------------" << std::endl;
    printMatrix(outputImage);

   ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
   free(ImgIn); free(ImgOut);

   return 9;
}*/
