//OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//utiles
#include <random>
#include <ctime>
#include <vector>
#include <chrono>
//headers
#include "utils.h"


//Variables globales :

// Ajout : handles pour la grille
unsigned int gridVAO = 0;
unsigned int gridVBO = 0;
int gridVertexCount = 0;

//dimension de la grille par défaut
int gridCols = 100;
int gridRows = 100;


//essai avec création de cellule, mise en suspend
/*
// Cellule de la grille de simulation
unsigned int cellsVAO = 0;
unsigned int cellsVBO = 0;     // positions
unsigned int cellsCBO = 0;     // colors
int cellsVertexCount = 0;      // number of vertices (6 * cols * rows)
*/

std::vector<Cell> cells;      // vecteur des cellules (tableau)
std::vector<Cell> cellsNext;  // next state
//std::vector<float> cellVertices; // per-vertex positions
//std::vector<float> cellColors;   // per-vertex colors (rgb)
bool simRunning = true; // start running by default
float simStepSeconds = 0.1f;
std::chrono::steady_clock::time_point lastStepTime;

// Vertices du triangle (global)
float vertices[] = {
    -0.75f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
};

// État de la dilatation (accumulation)
float currentScale = 1.0f;
// État de la température (accumulation)
float currentHeat = 0.0f;  // -1.0 (bleu froid) à +1.0 (rouge chaud)