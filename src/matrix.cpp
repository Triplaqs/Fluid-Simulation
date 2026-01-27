#include <vector>
#include <iostream>

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

void printMatrix(const ImageMatrix& imgMatrix) {
    for (int y = 0; y < imgMatrix.getHeight(); ++y) {
        for (int x = 0; x < imgMatrix.getWidth(); ++x) {
            std::cout << imgMatrix.getPixel(x, y) << " ";
        }
        std::cout << std::endl;
    }
};

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
