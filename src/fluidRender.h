#ifndef FLUIDRENDER_H
#define FLUIDRENDER_H

#include "utils.h"

void addDensity(int i, int j, float qt);
void addForce(int i, int j, float fx, float fy);
void updateSimulation_nouveau(unsigned int shaderProgram);

// configure a circular obstacle (boule) in grid coordinates
// ci, cj : centre cell indices (1..N)
// radius : radius in cells
// configure a circular obstacle (boule) in grid coordinates
// ci, cj : centre cell indices (1..N)
// radius : radius in cells
void bouled(int ci, int cj, int radius);

// predicate used by rendering and solver helpers
bool isObstacleCell(int i, int j);



#endif 