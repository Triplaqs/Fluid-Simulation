#ifndef FLUID_RENDER_H
#define FLUID_RENDER_H
//pour que ca soit inclut qu'une seule fois (sinon erreurs à la compilation de redéfinition de fonctions...)
#pragma once

void updateSimulation(unsigned int shaderProgram);
void addDensity(int i, int j, float qt);
void addForce(int i, int j, float fx, float fy);



#endif 