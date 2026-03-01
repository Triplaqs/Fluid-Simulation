//openGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//utiles
#include <random>
#include <ctime>
#include <vector>
#include <cmath>
#include <chrono>
//headers
#include "utils.h"
#include "interaction.h"
#include "fluid_solver.h"
#include "fluides.h"

// -----------------------------------------------------------------------------
// obstacle (boule) support
// -----------------------------------------------------------------------------
// obstacle parameters are stored in grid coordinates (1..N)
static int obs_ci = -1;
static int obs_cj = -1;
static int obs_radius = 0;
static int prev_obs_ci = -1;
static int prev_obs_cj = -1;
static float obs_vx = 0.0f;
static float obs_vy = 0.0f;

// helper to test whether a grid cell lies inside the circular obstacle
bool isObstacleCell(int i, int j)
{
    if (obs_radius <= 0) return false;           // no obstacle defined
    float dx = (float)i - (float)obs_ci;
    float dy = (float)j - (float)obs_cj;
    return (dx*dx + dy*dy) <= (float)obs_radius * (float)obs_radius;
}

// public API used by main.cpp
void bouled(int ci, int cj, int radius)
{
    obs_ci = ci;
    obs_cj = cj;
    obs_radius = radius;
}



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
    dens_prev[IX(i, j)] = 50.0f;
    u_prev[IX(i, j)] = 50.0f;   // se diffuse vers le haut si valeur positive et vers le bas si valeur négative 
    v_prev[IX(i, j)] = -50.0f;  //se diffuse vers la droite si valeur positive et vers la gauche si négative
    

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

    // appliquer l'obstacle : rendre la zone solide et repousser le fluide
    if (obs_radius > 0) {
        const float restitution = 0.7f;
        const float velScale = 10.0f;

        if (prev_obs_ci < 0) { prev_obs_ci = obs_ci; prev_obs_cj = obs_cj; }
        obs_vx = (float)(obs_ci - prev_obs_ci);
        obs_vy = (float)(obs_cj - prev_obs_cj);

        // 1) intérieur : vider densité et donner la vitesse de l'obstacle
        for (int ii = 1; ii <= N; ++ii) {
            for (int jj = 1; jj <= N; ++jj) {
                if (isObstacleCell(ii, jj)) {
                    dens[IX(ii,jj)] = 0.0f;
                    dens_prev[IX(ii,jj)] = 0.0f;
                    float ob_u = obs_vx * velScale;
                    float ob_v = obs_vy * velScale;
                    u[IX(ii,jj)] = ob_u; v[IX(ii,jj)] = ob_v;
                    u_prev[IX(ii,jj)] = ob_u; v_prev[IX(ii,jj)] = ob_v;
                }
            }
        }

        // 2) bord : réfléchir la composante normale pour pousser le fluide
        for (int ii = 1; ii <= N; ++ii) {
            for (int jj = 1; jj <= N; ++jj) {
                if (isObstacleCell(ii, jj)) continue;
                bool adjacent = false;
                if (ii-1 >= 1 && isObstacleCell(ii-1, jj)) adjacent = true;
                if (ii+1 <= N && isObstacleCell(ii+1, jj)) adjacent = true;
                if (jj-1 >= 1 && isObstacleCell(ii, jj-1)) adjacent = true;
                if (jj+1 <= N && isObstacleCell(ii, jj+1)) adjacent = true;
                if (!adjacent) continue;

                float nx = (float)ii - (float)obs_ci;
                float ny = (float)jj - (float)obs_cj;
                float nlen = sqrtf(nx*nx + ny*ny);
                if (nlen < 1e-6f) continue;
                nx /= nlen; ny /= nlen;

                float uu = u[IX(ii,jj)];
                float vv = v[IX(ii,jj)];
                float vn = uu*nx + vv*ny;
                float new_vn = -restitution * vn;
                float ut = uu - vn*nx;
                float vt = vv - vn*ny;
                u[IX(ii,jj)] = ut + new_vn*nx;
                v[IX(ii,jj)] = vt + new_vn*ny;
            }
        }

        prev_obs_ci = obs_ci;
        prev_obs_cj = obs_cj;
    }
}