//openGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//utiles
#include <random>
#include <ctime>
#include <vector>
#include <cmath>
#include <cfloat>
#include <chrono>
#include <algorithm>
#include <math.h>
//headers
#include "utils.h"
#include "interaction.h"
#include "fluid_solver.h"
#include "fluides.h"
#include "matrix.h"
#include "fluidRender.h"

// -----------------------------------------------------------------------------
// obstacle support (multiple obstacles)
// -----------------------------------------------------------------------------
// obstacles stored in grid coordinates (1..N)
static std::vector<Obstacle> obstacles;
// previous frame obstacle positions (used to compute obstacle velocity)
static std::vector<Obstacle> prevObstacles;

// helper to test whether a grid cell lies inside any circular obstacle
bool isObstacleCell(int i, int j){
    for (const Obstacle& o : obstacles) {
        if (o.radius <= 0) continue;
        float dx = (float)i - (float)o.ci;
        float dy = (float)j - (float)o.cj;
        if ((dx*dx + dy*dy) <= (float)o.radius * (float)o.radius) return true;
    }
    return false;
}

void clearObstacles()
{
    obstacles.clear();
}

void addObstacle(int ci, int cj, int radius)
{
    obstacles.push_back({ci, cj, radius});
}

int getObstacleCount()
{
    return (int)obstacles.size();
}

int findObstacleIndex(int i, int j)
{
    for (int idx = 0; idx < (int)obstacles.size(); ++idx) {
        const Obstacle& o = obstacles[idx];
        if (o.radius <= 0) continue;
        float dx = (float)i - (float)o.ci;
        float dy = (float)j - (float)o.cj;
        if ((dx*dx + dy*dy) <= (float)o.radius * (float)o.radius) return idx;
    }
    return -1;
}

void moveObstacle(int index, int ci, int cj)
{
    if (index < 0 || index >= (int)obstacles.size()) return;
    obstacles[index].ci = ci;
    obstacles[index].cj = cj;
}

const std::vector<Obstacle>& getObstacles()
{
    return obstacles;
}

// public API used by main.cpp
void bouled(int ci, int cj, int radius)
{
    clearObstacles();
    if (radius > 0) addObstacle(ci, cj, radius);
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
//gros test : on ajoute une source de densité à chaque frame pour voir le résultat
static void applyConvolutionToDensity(std::vector<float>& density)
{
    ImageMatrix input(N+2, N+2);
    ImageMatrix output(N+2, N+2);

    for (int j = 0; j <= N+1; ++j) {
        for (int i = 0; i <= N+1; ++i) {
            input.setPixel(i, j, density[IX(i,j)]);
        }
    }

    std::vector<std::vector<float>> kernel = {
        {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
        {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
        {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f}
    };

    convolutionMatrix(input, output, kernel);

    for (int j = 0; j <= N+1; ++j) {
        for (int i = 0; i <= N+1; ++i) {
            density[IX(i,j)] = output.getPixel(i,j);
        }
    }
}


void updateSimulation_nouveau(unsigned int shaderProgram)
{
    // on teste ajouter une source de densité 

    auto clampGrid = [&](float value) { // debut de mes tests ici
        int iv = (int)(value + 0.5f);
        if (iv < 1) iv = 1;
        if (iv > N) iv = N;
        return iv;
    };

    auto& activePrev = nextFluidIsHot ? dens_red_prev : dens_blue_prev;

    if (fluid_start == 0) {
        int i_source = clampGrid(x);
        int j_source = clampGrid(y);
        activePrev[IX(i_source, j_source)] = glob * 50.0f;
        u_prev[IX(i_source, j_source)] = force * cos((angle - 90.0f) * M_PI / 180.0f) * 50.0f;
        v_prev[IX(i_source, j_source)] = force * sin((angle - 90.0f) * M_PI / 180.0f) * (-50.0f);
    } else {
        int i0 = clampGrid(x);
        int j0 = clampGrid(y);
        int i1 = clampGrid(x2);
        int j1 = clampGrid(y2);
        int steps = std::max(std::abs(i1 - i0), std::abs(j1 - j0)) + 1;
        for (int k = 0; k < steps; ++k) {
            float t = steps > 1 ? (float)k / (float)(steps - 1) : 0.0f;
            int i_src = i0 + (int)roundf(t * (i1 - i0));
            int j_src = j0 + (int)roundf(t * (j1 - j0));
            if (i_src < 1) i_src = 1;
            if (i_src > N) i_src = N;
            if (j_src < 1) j_src = 1;
            if (j_src > N) j_src = N;
            activePrev[IX(i_src, j_src)] = glob * 50.0f;
            u_prev[IX(i_src, j_src)] = force * cos((angle - 90.0f) * M_PI / 180.0f) * 50.0f;
            v_prev[IX(i_src, j_src)] = force * sin((angle - 90.0f) * M_PI / 180.0f) * (-50.0f);
        }
    } // fin de mes tests ici

    //test des valeurs
    diff = 0.00001f; //au plus la valeur est petite au plus ca se diffuse vite
    visc = 0.00001f; //au plus la valeur est petite au plus le fluide est visqueux (moins compact)

    // Mise à jour vitesse
    vel_step(N, u.data(), v.data(), u_prev.data(), v_prev.data(), visc, dt);

    
    // Mise à jour densité séparée par couleur pour que le fluide déjà présent garde sa teinte (encore mes tests)
    dens_step(N, dens_red.data(), dens_red_prev.data(), u.data(), v.data(), diff, dt);
    dens_step(N, dens_blue.data(), dens_blue_prev.data(), u.data(), v.data(), diff, dt);

    // Homogénéisation par convolution à chaque étape
    applyConvolutionToDensity(dens_red);
    applyConvolutionToDensity(dens_blue);

    // Mettre à jour la densité totale si besoin
    int size = (N+2) * (N+2);
    for (int idx = 0; idx < size; ++idx) {
        dens[idx] = dens_red[idx] + dens_blue[idx];
    }

    // Reset des forces et des densités pour la prochaine frame
    std::fill(u_prev.begin(), u_prev.end(), 0.0f);
    std::fill(v_prev.begin(), v_prev.end(), 0.0f);
    
    std::fill(dens_red_prev.begin(), dens_red_prev.end(), 0.0f);
    std::fill(dens_blue_prev.begin(), dens_blue_prev.end(), 0.0f);
    
    std::fill(dens_prev.begin(), dens_prev.end(), 0.0f);

    // appliquer les obstacles : rendre la zone solide et repousser le fluide
    if (getObstacleCount() > 0) {
        const float restitution = 0.7f;

        // 1) intérieur : appliquer la vitesse de l'obstacle pour déplacer le fluide
        //    (ne pas effacer la densité, juste contraindre la vitesse à celle de l'obstacle)
        for (int ii = 1; ii <= N; ++ii) {
            for (int jj = 1; jj <= N; ++jj) {
                int obsIdx = findObstacleIndex(ii, jj);
                if (obsIdx >= 0) {
                    // compute obstacle velocity based on its movement since last frame
                    float ob_vx = 0.0f;
                    float ob_vy = 0.0f;
                    if (obsIdx < (int)prevObstacles.size()) {
                        ob_vx = (float)(obstacles[obsIdx].ci - prevObstacles[obsIdx].ci);
                        ob_vy = (float)(obstacles[obsIdx].cj - prevObstacles[obsIdx].cj);
                    }
                    u[IX(ii,jj)] = ob_vx;
                    v[IX(ii,jj)] = ob_vy;
                    u_prev[IX(ii,jj)] = ob_vx;
                    v_prev[IX(ii,jj)] = ob_vy;
                }
            }
        }

        // 2) bord : réfléchir la composante normale pour pousser le fluide
        for (int ii = 1; ii <= N; ++ii) {
            for (int jj = 1; jj <= N; ++jj) {
                if (isObstacleCell(ii, jj)) continue;
                bool adjacent = false;
                Obstacle closest;
                float closestDist2 = FLT_MAX;

                // déterminer l'obstacle le plus proche parmi les voisins
                for (const Obstacle& o : obstacles) {
                    // vérifier l'adjacence au bord de cet obstacle
                    bool adj = false;
                    if (ii-1 >= 1 && ( (ii-1 - o.ci)*(ii-1 - o.ci) + (jj - o.cj)*(jj - o.cj) <= o.radius*o.radius )) adj = true;
                    if (ii+1 <= N && ( (ii+1 - o.ci)*(ii+1 - o.ci) + (jj - o.cj)*(jj - o.cj) <= o.radius*o.radius )) adj = true;
                    if (jj-1 >= 1 && ( (ii - o.ci)*(ii - o.ci) + (jj-1 - o.cj)*(jj-1 - o.cj) <= o.radius*o.radius )) adj = true;
                    if (jj+1 <= N && ( (ii - o.ci)*(ii - o.ci) + (jj+1 - o.cj)*(jj+1 - o.cj) <= o.radius*o.radius )) adj = true;
                    if (!adj) continue;
                    adjacent = true;

                    float dx = (float)ii - (float)o.ci;
                    float dy = (float)jj - (float)o.cj;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < closestDist2) {
                        closestDist2 = d2;
                        closest = o;
                    }
                }

                if (!adjacent) continue;

                float nx = (float)ii - (float)closest.ci;
                float ny = (float)jj - (float)closest.cj;
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
    }

    // Mettre à jour l'état précédent des obstacles pour la prochaine frame
    prevObstacles = obstacles;
}
