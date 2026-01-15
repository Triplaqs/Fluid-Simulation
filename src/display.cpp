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
#include "display.h"

//Edit de rendering
// Fonction pour éditer la position du triangle via uniform (exemple avec translation matrix) (z et w set à 0 dans le header)
void setTrianglePosition(unsigned int shaderProgram, float x, float y, float z, float w) {
    glUseProgram(shaderProgram);
    int posLoc = glGetUniformLocation(shaderProgram, "offset");
    glUniform4f(posLoc, x, y, z, w);
}

// Fonction pour éditer la couleur du triangle via uniform
void setTriangleColor(unsigned int shaderProgram, float r, float g, float b, float a) {
    glUseProgram(shaderProgram);
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, r, g, b, a);
}

//Donne une couleur aléatoire au triangle
void setTriangleColorRand(unsigned int shaderProgram) {
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float a = 1.0f;
    setTriangleColor(shaderProgram, r, g, b, a);
}

//Fait tourner le triangle grâce au temps
void makeTriangleSpin(unsigned int shaderProgram, float time) {
    float angle = time;
    //float angle = (float)glfwGetTime();
    float x = 0.5f * cos(angle);
    float y = 0.5f * sin(angle);
    setTrianglePosition(shaderProgram, x, y);
}

//dilate le triangle (c'est dans le nom)
void dilateTriangle(unsigned int shaderProgram, float factor) {
    //Calcul du centre du triangle
    float cx = (vertices[0] + vertices[3] + vertices[6]) / 3.0f;
    float cy = (vertices[1] + vertices[4] + vertices[7]) / 3.0f;
    float cz = (vertices[2] + vertices[5] + vertices[8]) / 3.0f;
    
    glUseProgram(shaderProgram);
    GLint loc_centroid = glGetUniformLocation(shaderProgram, "u_centroid");
    GLint loc_scale = glGetUniformLocation(shaderProgram, "u_scale");
    
    glUniform4f(loc_centroid, cx, cy, cz, 1.0f);
    glUniform1f(loc_scale, factor);
}

//change la couleur du triangle d'un point de vue de la température (essai)
void heatTriangle(unsigned int shaderProgram, float factor) {
    // Clamp la température entre -1 et 1
    if (currentHeat + factor > 1.0f) currentHeat = 1.0f;
    else if (currentHeat + factor < -1.0f) currentHeat = -1.0f;
    else currentHeat += factor;
    
    // Interpoler entre bleu (froid) et rouge (chaud)
    // currentHeat = -1.0 -> (0, 0, 1) bleu
    // currentHeat =  0.0 -> (1, 1, 1) blanc
    // currentHeat =  1.0 -> (1, 0, 0) rouge
    float r = (currentHeat + 1.0f) / 2.0f;  // 0 à 1
    float g = (1.0f - fabs(currentHeat)) * 0.5f;  // Max au milieu
    float b = (1.0f - currentHeat) / 2.0f;  // 1 à 0
    
    setTriangleColor(shaderProgram, r, g, b, 1.0f);
}

//on vient juste appliquer heatTriangle sur les 2 triangles de chaque cellule
void heatCells(unsigned int shaderProgram, Cell cell, float factor){
    // Appliquer la chaleur au premier triangle
    glBindVertexArray(cell.VAO1);
    heatTriangle(shaderProgram, factor);
    
    // Appliquer la chaleur au deuxième triangle
    glBindVertexArray(cell.VAO2);
    heatTriangle(shaderProgram, factor); 
}
