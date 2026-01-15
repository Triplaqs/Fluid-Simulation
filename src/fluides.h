#ifndef FLUIDES_H
#define FLUIDES_H

#include "utils.h"


//créait une cellule dans la grille
Cell createCell(int x, int y, float cellW, float cellH);

//Initialise la grille de cellules de simulation
void initCellsGrid(int gridWidth=100, int gridHeight=100);

// Remplissage des cellules aléatoirement
void randomizeCells();

#endif 
