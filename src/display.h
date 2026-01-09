#ifndef DISPLAY_H
#define DISPLAY_H


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


#endif