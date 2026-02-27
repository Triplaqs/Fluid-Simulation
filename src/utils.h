#ifndef UTILS_H
#define UTILS_H

//openGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//utiles
#include <random>
#include <ctime>
#include <vector>
#include <chrono>
#include <tuple>

//Variables globales nécessaires pour structures:

//variable de récup pour la fenêtre ImgUI (saisie de texte, string)
extern char inputBuffer[256];
//variable test récup par la fenêtre ImgUI, float entre 0 et 1
extern float glob;
//variable qui stoque si la data a été envoyée -> update la simulation
extern bool sent;

//pour le nouvel affichage du fluide
extern unsigned int cellVAO;
extern unsigned int cellVBO;

//dimension de la grille par défaut
extern int gridCols;
extern int gridRows;

extern int winWdt;
extern int winHgt;

extern float cellHgt; //calcule la hauteur/largeur d'une cellule afin de normaliser les vecteur pour l'affichage du champ vect
extern float cellWdt;


extern int N;
extern float dt, diff, visc;

extern std::vector<float> u, v, u_prev, v_prev;
extern std::vector<float> dens, dens_prev;


//définition de la macro
#define IX(i,j) ((i) + (N+2) * (j))
#define SWAP(x0,x) {float *tmp=x0; x0=x; x=tmp;}


//Structures :

//Structure pour manipuler points
typedef struct Vec4 {
    double x;
    double y;
    double z;
    double w;
} Vec4;

//Structure pour champ vectoriel 2D
typedef struct Vec2{
    double x;
    double y;
    double getx() const{ 
        return x;
    }
    double gety() const{
        return y;
    }
} Vec2;

//nouvelle structure de cellulle
typedef struct Cell {
    unsigned int VAO1;  // VAO du premier triangle
    unsigned int VAO2;  // VAO du deuxième triangle
    unsigned int VBO1;  // VBO du premier triangle
    unsigned int VBO2;  // VBO du deuxième triangle
    float temperature = 0.0f; // température de la cellule (0.0f à 1.0f) (sera le facteur dans les fonctions heat)
    Vec2 vect = {0.0, 0.0}; // vecteur de la case
    float pression = 0.0f; // pression de la cellule
    float concentration = 0.0f; //concentration de la cellule
    int x,y; // position dans la grill

    //indique si c'est un bord ou non, afin de mieux manipuler les cases en bord de grille
    bool bh(){return x == 0.0;};
    bool bb(){return x == gridRows-1;};
    bool bg(){return y == 0.0;};
    bool bd(){return y == gridCols-1;};

    int nbVoisins(){
        int nb = 8;
        if(bh() || bb()) nb -= 3;
        if(bg() || bd()) nb -=3;
        if((bh() && bg()) || (bh() && bd()) || (bb() && bg()) || (bb() && bd())) nb += 1; 
        return nb;
    } // nombre de voisins (utile pour les bords)

    //Génération des 2 points d'affichage pour les flèches
    std::tuple<Vec2, Vec2> get_vec_pts(){
        std::tuple<Vec2, Vec2> vec;
        return vec;
    }
} Cell;

typedef struct Grid{
    int cols = gridCols;
    int rows = gridRows;
    int aff_mode = 0;
    int mode_max = 1;
    std::vector<Cell> grid;
    int taille = 100*100;

    void set_vec_rand(){
        
        return;  //à compléter avec la fonction de Gaëlle
    }
}Grid;



//Variables globales :

// Ajout : handles pour la grille
extern unsigned int gridVAO;
extern unsigned int gridVBO;
extern int gridVertexCount;


extern Grid cells;      // vecteur des cellules (tableau)
extern Grid cellsNext;  // next state
//std::vector<float> cellVertices; // per-vertex positions
//std::vector<float> cellColors;   // per-vertex colors (rgb)
extern bool simRunning; // start running by default
extern float simStepSeconds;
extern std::chrono::steady_clock::time_point lastStepTime;

// Vertices du triangle (global)
extern float vertices[9];

// État de la dilatation (accumulation)
extern float currentScale;
// État de la température (accumulation)
extern float currentHeat;  // -1.0 (bleu froid) à +1.0 (rouge chaud)


#endif