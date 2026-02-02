#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

class ImageMatrix {
private:
    std::vector<std::vector<float>> matrix;
    int width, height;

public:
    ImageMatrix(int w, int h) : width(w), height(h) {
        matrix.assign(height, std::vector<float>(width, 0.0f));
    }

    void setPixel(int x, int y, float value) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            matrix[y][x] = value;
        }
    }

    float getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return matrix[y][x];
        }
        return 0.0f;
    }

    std::vector<std::vector<float>>& getMatrix() {
        return matrix;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

//affiche la matrice dans le terminal 
void printMatrix(const ImageMatrix& imgMatrix);

//Calcul le produit de convolution de input et kernel (varie selon le filtre appliqué) et le renvoie (effet de bord) dans output
void convolutionMatrix(const ImageMatrix& input, ImageMatrix& output, std::vector<std::vector<float>> kernel);

#endif // MATRIX_H
