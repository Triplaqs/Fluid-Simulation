#ifndef DISPLAY_H
#define DISPLAY_H


extern unsigned int flecheVAO;
extern unsigned int flecheVBO;
extern unsigned int shaderProgramCellsFleche;

// variable de temps de cooldown de pression de touche
extern float start_press; //-1 si aucune touche préssée
extern float duree_cooldown;
extern float t_press;

// Fonction pour éditer la position du triangle via uniform (exemple avec translation matrix)
void setTrianglePosition(unsigned int shaderProgram, float x, float y, float z = 0.0f, float w = 1.0f);


// Fonction pour éditer la couleur du triangle via uniform
void setTriangleColor(unsigned int shaderProgram, float r, float g, float b, float a);

//Donne une couleur aléatoire au triangle
void setTriangleColorRand(unsigned int shaderProgram);


//Fait tourner le triangle grâce au temps
void makeTriangleSpin(unsigned int shaderProgram, float time);


//dilate le triangle (c'est dans le nom)
void dilateTriangle(unsigned int shaderProgram, float factor);

//change la couleur du triangle d'un point de vue de la température (essai)
void heatTriangle(unsigned int shaderProgram, float factor);

//on vient juste appliquer heatTriangle sur les 2 triangles de chaque cellule
void heatCells(unsigned int shaderProgram, Cell cell, float factor);


//on construit notre fleche pour la dessiner
void drawArrow(float x1, float y1, float x2, float y2);

//on dessine une fleche grace aà drawarrow
void affichagefleche(const Cell& c);

void affichagefleche_aleatoire(const Cell& c);

//on dessine une fleche grace aà drawarrow
//void _affichagefleche(const Cell& c, float cellWidth, float cellHeight);


#endif