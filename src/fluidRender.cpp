//openGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//utiles
#include <random>
#include <ctime>
#include <vector>
#include <chrono>
#include <math.h>
//headers
#include "utils.h"
#include "interaction.h"
#include "fluid_solver.h"
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


void updateSimulation_nouveau(unsigned int shaderProgram)
{
    // on teste ajouter une source de densité 

    //on choisit les coordonnées pour l'injection de la densité (en bas à gauche avec 5,5 et milieu N/2,N/2)
    int i = N/2;   
    int j = N/2;
    dens_prev[IX(i, j)] = glob*50.0f;
    u_prev[IX(i, j)] = force*cos((angle - 90.0f)*M_PI/180.0f)*(50.0f);   // se diffuse vers le haut si valeur positive et vers le bas si valeur négative 
    v_prev[IX(i, j)] = force*sin((angle - 90.0f)*M_PI/180.0f)*(-50.0f);  //se diffuse vers la droite si valeur positive et vers la gauche si négative
    

    // le depart est une colonne presque entiere
    /*for (int i = 20; i < 80; i++){
        dens_prev[IX(i, 20)] = 30.0f;
        u_prev[IX(N/2,N/2)] = 50.0f;
        v_prev[IX(N/2,N/2)] = -50.0f;
    }*/




    //test des valeurs
    diff = 0.00001f; //au plus la valeur est petite au plus ca se diffuse vite
    visc = 0.00001f; //au plus la valeur est petite au plus le fluide est visqueux (moins compact)

    // Mise à jour vitesse
    vel_step(N, u.data(), v.data(), u_prev.data(), v_prev.data(), visc, dt);

    // Mise à jour densité
    dens_step(N, dens.data(), dens_prev.data(), u.data(), v.data(), diff, dt);

    // Reset des forces pour la prochaine frame
    std::fill(u_prev.begin(), u_prev.end(), 0.0f);
    std::fill(v_prev.begin(), v_prev.end(), 0.0f);
    std::fill(dens_prev.begin(), dens_prev.end(), 0.0f);
}