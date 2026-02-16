#ifndef FLUIDES_H
#define FLUIDES_H

#include "utils.h"

extern float cellWidth;
extern float cellHeight;

// Macro d'indexation (utilise la variable `N` présente dans chaque unité de compilation
// ou passée en paramètre aux fonctions qui prennent `N` en argument)
#define IX(i,j) ((i) + (N+2) * (j))

#define SWAP(x0,x) {float *tmp=x0;x0=x;x=tmp;}

extern int N;
extern float dt;
extern float diff;
extern float visc;

extern std::vector<float> u, v, u_prev, v_prev;
extern std::vector<float> dens, dens_prev;


//créait une cellule dans la grille
Cell createCell(int x, int y, float cellW, float cellH);

// Initialise les tableaux du solveur de fluide
void initFluid();

//Initialise la grille de cellules de simulation
void initCellsGrid(int gridWidth=100, int gridHeight=100);

// Remplissage des cellules aléatoirement
void randomizeCells();

// Set les vecteurs aléatoirement
void randomizeVecs();

//Met à jour la simulation (diffusion)
void updateSimulation(unsigned int shaderProgram);

#endif 
