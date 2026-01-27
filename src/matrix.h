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

void printMatrix(const ImageMatrix& imgMatrix);
void convolutionMatrix(const ImageMatrix& input, ImageMatrix& output);

#endif // MATRIX_H
