//OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//utiles
#include <random>
#include <ctime>
#include <vector>
#include <chrono>
//headers
#include "utils.h"



//Variables globales :
//variable de récup pour la fenêtre ImgUI (saisie de texte, string)
char inputBuffer[256] = "1.0"; //valeur par défaut
//variable test récup par la fenêtre ImgUI, float entre 0 et 1
float glob=1.0f;
//variable qui stoque si la data a été envoyée -> update la simulation
bool sent = false;

// Ajout : handles pour la grille
unsigned int gridVAO = 0;
unsigned int gridVBO = 0;
int gridVertexCount = 0;

//dimension de la grille par défaut
int gridCols = 100;
int gridRows = 100;
//Dimensions fenêtre :
int winWdt;
int winHgt;

//calcule la hauteur/largeur d'une cellule afin de normaliser les vecteur pour l'affichage du champ vect
float cellHgt; 
float cellWdt;

//implémentation stam
int N = 100; //taille de notre grille
float dt = 0.1f;
float diff = 0.0f;
float visc = 0.0f;

std::vector<float> u;
std::vector<float> v;
std::vector<float> u_prev;
std::vector<float> v_prev;

std::vector<float> dens;
std::vector<float> dens_prev;

Grid cells;      // vecteur des cellules (tableau)
Grid cellsNext;  // next state
//std::vector<float> cellVertices; // per-vertex positions
//std::vector<float> cellColors;   // per-vertex colors (rgb)
bool simRunning = true; // start running by default
float simStepSeconds = 0.1f;
std::chrono::steady_clock::time_point lastStepTime;

//Est-ce que la fenêtre est affichée ou non ? (se modifie avec H pour Hide)
bool show_ui = false;

// Vertices du triangle (global)
float vertices[] = {
    -0.75f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
};

// État de la dilatation (accumulation)
float currentScale = 1.0f;
// État de la température (accumulation)
float currentHeat = 0.0f;  // -1.0 (bleu froid) à +1.0 (rouge chaud)