#ifndef FLUIDRENDER_H
#define FLUIDRENDER_H

#include "utils.h"

void updateSimulation(unsigned int shaderProgram);
void addDensity(int i, int j, float qt);
void addForce(int i, int j, float fx, float fy);



#endif 