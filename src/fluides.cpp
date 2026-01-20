//openGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//utiles
#include <random>
#include <ctime>
#include <vector>
#include <chrono>
//headers
#include "utils.h"
#include "interaction.h"
#include "fluides.h"




//créait une cellule dans la grille
Cell createCell(int x, int y, float cellW, float cellH){
    Cell cell;
    cell.x = x;
    cell.y = y;

    float x0 = -1.0f + x * cellW;
    float y0 = -1.0f + y * cellH;
    float x1 = x0 + cellW;
    float y1 = y0 + cellH;

    // Triangle 1 :  bas-gauche, bas-droit, haut-droit
    float vertices1[] = {
        x0, y0, 0.0f,
        x1, y0, 0.0f,
        x1, y1, 0.0f
    };
    
    // Triangle 2 : bas-gauche, haut-droit, haut-gauche
    float vertices2[] = {
        x0, y0, 0.0f,
        x1, y1, 0.0f,
        x0, y1, 0.0f
    };
    
    // === Création Triangle 1 ===
    glGenVertexArrays(1, &cell.VAO1);
    glGenBuffers(1, &cell.VBO1);
    
    glBindVertexArray(cell.VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, cell.VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // === Création Triangle 2 ===
    glGenVertexArrays(1, &cell.VAO2);
    glGenBuffers(1, &cell.VBO2);
    
    glBindVertexArray(cell.VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, cell.VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return cell;
}

//Initialise la grille de cellules de simulation (valeurs par défaut définies à 100x100 dans le hearders)
void initCellsGrid(int gridWidth, int gridHeight) {
    float cellWidth = 2.0f / gridWidth;   //2.0f = longueur de la fenêtre (-1 -> 1)
    float cellHeight = 2.0f / gridHeight;   
    cells.clear();
    cells.reserve(gridWidth * gridHeight);
    
    for (int j = 0; j < gridHeight; ++j) {
        for (int i = 0; i < gridWidth; ++i) {     
            // Crée et ajoute la cellule
            Cell cell = createCell(i, j, cellWidth, cellHeight);
            cells.push_back(cell);
            cellsNext.push_back(cell);
        }
    }
}

// Remplissage des cellules aléatoirement
void randomizeCells(){
    //initialisation random de la température des cellules
    std::mt19937 rng((unsigned int)time(NULL));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    /*for(size_t i = 0; i < cells.size(); ++i){
        cells[i].temperature = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }*/
   //nouvelle DA
   for (Cell& c : cells){
    c.temperature = dist(rng);
   }
/*
    // mise à couleur de l'initialisation random
    for(int i = 0; i < gridCols * gridRows; ++i){
        int base = i * 6 * 3;
        float r = cells[i].temperature; 
        float g = cells[i].temperature;
        float b = cells[i].temperature;
        for(int v = 0; v < 6; ++v){
            cellColors[base + v*3 + 0] = r;
            cellColors[base + v*3 + 1] = g;
            cellColors[base + v*3 + 2] = b;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, cellsCBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cellColors.size() * sizeof(float), cellColors.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);*/
}


//Met à jour la simulation (diffusion)
void updateSimulation(unsigned int shaderProgram){
    //float newVals[gridRows][gridCols]; (pas besoin de matrice)
    for(int y = 0; y < gridRows; ++y){
        for(int x = 0; x < gridCols; ++x){
            //ordonnancement linéaire de la grille
            int idx = y * gridCols + x;

            //calcul de la moyenne des voisins
            float mean = 0.0f;
            for(int oy = -1; oy <= 1; ++oy){
                for(int ox = -1; ox <= 1; ++ox){
                    if(ox == 0 && oy == 0) continue;
                    if(cells[idx].bh() && oy == -1) continue;
                    if(cells[idx].bb() && oy == 1) continue;
                    if(cells[idx].bg() && ox == -1) continue;
                    if(cells[idx].bd() && ox == 1) continue;
                    int nx = x + ox;
                    int ny = y + oy;
                    mean += cells[nx + ny * gridCols].temperature;
                }
            }
            //Ici aux bords on prend de l'autre coté, par soucis de simplicité (on modifira après hein)
            mean /= cells[idx].nbVoisins();
            //update
            cellsNext[idx].temperature = mean; //moyenne de toutes les cases voisines
            //changement de couleur
            //heatCells(shaderProgram, cells[idx], mean);
        
            //Askip le rendemendent je dois le faire dans la boucle de rendue... ici seulement calculs
        }
    }
    // passe le contenu de CellsNext dans Cells
    cells.swap(cellsNext);
}
