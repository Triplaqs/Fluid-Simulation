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

void addDensity(int i, int j, float qt)
{
    dens_prev[IX(i,j)] += qt;
}


void addForce(int i, int j, float fx, float fy)
{
    u_prev[IX(i,j)] += fx;
    v_prev[IX(i,j)] += fy;
}

/* en attente d'avoir les fonctions implémentées 
void updateSimulation(unsigned int shaderProgram)
{
    //on met à jour la vitesse
    vel_step(N, u.data(), v.data(), u_prev.data(), v_prev.data(), visc, dt);

    //on met à la densité
    dens_step(N, dens.data(), dens_prev.data(), u.data(), v.data(), diff, dt);

    //on réinitialise les forces et densités ajoutées pour la prochaine frame
    std::fill(u_prev.begin(), u_prev.end(), 0.0f);
    std::fill(v_prev.begin(), v_prev.end(), 0.0f);
    std::fill(dens_prev.begin(), dens_prev.end(), 0.0f);
}*/