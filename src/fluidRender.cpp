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
#include <math.h>
//headers
#include "utils.h"
#include "interaction.h"
#include "fluid_solver.h"
#include "fluides.h"
#include "fluidRender.h"

// -----------------------------------------------------------------------------
// obstacle support (multiple obstacles)
// -----------------------------------------------------------------------------
// obstacles stored in grid coordinates (1..N)
static std::vector<Obstacle> obstacles;
// previous frame obstacle positions (used to compute obstacle velocity)
static std::vector<Obstacle> prevObstacles;

// signed distance function for hexagram, in grid units and centered at (0,0)
static float sdHexagramGrid(float x, float y, float r)
{
    // Usage : r = radius of star
    // Implementation directly from the provided shader code
    float kx = -0.5f;
    float ky = 0.8660254038f;
    float kz = 0.5773502692f;
    float kw = 1.7320508076f;

    float px = fabsf(x);
    float py = fabsf(y);

    float dot1 = px * kx + py * ky;
    if (dot1 < 0.0f) {
        px -= 2.0f * dot1 * kx;
        py -= 2.0f * dot1 * ky;
    }

    float dot2 = px * ky + py * kx;
    if (dot2 < 0.0f) {
        px -= 2.0f * dot2 * ky;
        py -= 2.0f * dot2 * kx;
    }

    float clamp_x = fmaxf(r * kz, fminf(px, r * kw));
    px -= clamp_x;
    py -= r;

    float len = sqrtf(px * px + py * py);
    return len * (py > 0.0f ? 1.0f : -1.0f);
}

// signed distance function for heart, in grid units and centered at (0,0)
static float sdHeartGrid(float x, float y, float r)
{
    // Normalize by radius
    float px = x / r;
    float py = y / r;

    // Heart SDF formula
    px = fabsf(px);
    if (py + px > 1.0f) {
        float dx = px - 0.25f;
        float dy = py - 0.75f;
        return sqrtf(dx*dx + dy*dy) - 0.3535533906f; // sqrt(2)/4
    } else {
        float a = fmaxf(px + py, 0.0f) * 0.5f;
        float dx1 = px - 0.0f;
        float dy1 = py - 1.0f;
        float d1 = dx1*dx1 + dy1*dy1;
        float dx2 = px - a;
        float dy2 = py - a;
        float d2 = dx2*dx2 + dy2*dy2;
        return sqrtf(fminf(d1, d2)) - 0.7071067812f; // 1/sqrt(2)
    }
}

// helper to test whether a grid cell lies inside obstacle shape
bool isObstacleCell(int i, int j){
    for (const Obstacle& o : obstacles) {
        if (o.radius <= 0) continue;
        float dx = (float)i - (float)o.ci;
        float dy = (float)j - (float)o.cj;
        switch (o.shape) {
            case OBSTACLE_CIRCLE: {
                if ((dx*dx + dy*dy) <= (float)o.radius * (float)o.radius) return true;
                break;
            }
            case OBSTACLE_HEART: {
                // approximate by circle habit for simplicity in solver context
                if ((dx*dx + dy*dy) <= (float)o.radius * (float)o.radius) return true;
                break;
            }
            case OBSTACLE_HEXAGRAM: {
                float d = sdHexagramGrid(dx, dy, (float)o.radius);
                if (d <= 0.0f) return true;
                break;
            }
            default:
                if ((dx*dx + dy*dy) <= (float)o.radius * (float)o.radius) return true;
                break;
        }
    }
    return false;
}

void clearObstacles()
{
    obstacles.clear();
}

void addObstacle(int ci, int cj, int radius, ObstacleShape shape)
{
    obstacles.push_back({ci, cj, radius, shape});
}

int getObstacleCount()
{
    return (int)obstacles.size();
}

static bool isPointInObstacle(const Obstacle& o, int i, int j)
{
    if (o.radius <= 0) return false;
    float dx = (float)i - (float)o.ci;
    float dy = (float)j - (float)o.cj;
    switch (o.shape) {
        case OBSTACLE_CIRCLE:
            return (dx*dx + dy*dy) <= (float)o.radius * (float)o.radius;
        case OBSTACLE_HEART:
            return sdHeartGrid(dx, dy, (float)o.radius) <= 0.0f;
        case OBSTACLE_HEXAGRAM:
            return sdHexagramGrid(dx, dy, (float)o.radius) <= 0.0f;
        default:
            return (dx*dx + dy*dy) <= (float)o.radius * (float)o.radius;
    }
}

int findObstacleIndex(int i, int j)
{
    for (int idx = 0; idx < (int)obstacles.size(); ++idx) {
        if (isPointInObstacle(obstacles[idx], i, j)) return idx;
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
void bouled(int ci, int cj, int radius, ObstacleShape shape)
{
    clearObstacles();
    if (radius > 0) addObstacle(ci, cj, radius, shape);
}

void bouled(int ci, int cj, int radius)
{
    bouled(ci, cj, radius, OBSTACLE_CIRCLE);
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
