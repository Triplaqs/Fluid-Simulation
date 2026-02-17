#ifndef FLUIDRENDER_H
#define FLUIDRENDER_H

#include "utils.h"

void addDensity(int i, int j, float qt);
void addForce(int i, int j, float fx, float fy);
void updateSimulation_nouveau(unsigned int shaderProgram);



#endif 