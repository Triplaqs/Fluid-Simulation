#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

class ImageMatrix {
private:
    std::vector<std::vector<float>> matrix;
    int width, height;

public:
    ImageMatrix(int w, int h);
    
    void setPixel(int x, int y, float value);
    float getPixel(int x, int y) const;
    
    std::vector<std::vector<float>>& getMatrix();
    
    int getWidth() const;
    int getHeight() const;
};

//affiche la matrice dans le terminal 
void printMatrix(const ImageMatrix& imgMatrix);

//Calcul le produit de convolution de input et kernel (varie selon le filtre appliqué) et le renvoie (effet de bord) dans output
void convolutionMatrix(const ImageMatrix& input, ImageMatrix& output, std::vector<std::vector<float>> kernel);

#endif // MATRIX_H
