#include <stdio.h>
#include <iostream>

//openGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//utiles
#include <random>
#include <ctime>
#include <vector>
#include <chrono>
//headers
#include "fluides.h"
#include "utils.h"
#include "display.h"
#include "interaction.h"
#include "fluidRender.h"  // for obstacle helper and bouled

//Variable temps cooldown pression touche
float start_press = -1; //-1 aucune touche préssée
float duree_cooldown = 0.3; //cooldown entre de pression 
float t_press=1;

//init pour affichage 
unsigned int cellVAO = 0;
unsigned int cellVBO = 0;

// Obstacle renderer globals
unsigned int obstacleVAO = 0;
unsigned int obstacleVBO = 0;
unsigned int obstacleProgram = 0;

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

void init_fluid_vao_vbo()
{
    glGenVertexArrays(1, &cellVAO);
    glGenBuffers(1, &cellVBO);

    glBindVertexArray(cellVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cellVBO);

    // 2 floats par vertex (x,y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}


// ---------- obstacle renderer ----------
static unsigned int compileShader(const char* src, unsigned int type)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    return id;
}

void initObstacleRenderer()
{
    // simple pass-through vertex shader
    const char* vertSrc = "#version 330 core\n"
    "layout(location=0) in vec2 aPos;\n"
    "out vec2 v_pos;\n"
    "void main(){ v_pos = aPos; gl_Position = vec4(aPos,0.0,1.0); }\n";

    const char* fragSrc = "#version 330 core\n"
    "in vec2 v_pos;\n"
    "out vec4 FragColor;\n"
    "uniform vec2 u_center;\n"
    "uniform float u_radius;\n"
    "uniform float u_edge;\n"
    "void main(){\n"
    "  float d = distance(v_pos, u_center);\n"
    "  float alpha = 1.0 - smoothstep(u_radius - u_edge, u_radius + u_edge, d);\n"
    "  if (alpha <= 0.001) discard;\n"
    "  vec3 inner = vec3(1.0, 0.75, 0.80);" // centre rose clair
    "  vec3 outer = vec3(0.70, 0.30, 0.50);" // bord rose foncé
    "  float t = clamp(d / u_radius, 0.0, 1.0);\n"
    "  vec3 col = mix(inner, outer, t);\n"
    "  FragColor = vec4(col, alpha);\n"
    "}\n";

    unsigned int vs = compileShader(vertSrc, GL_VERTEX_SHADER);
    unsigned int fs = compileShader(fragSrc, GL_FRAGMENT_SHADER);
    obstacleProgram = glCreateProgram();
    glAttachShader(obstacleProgram, vs);
    glAttachShader(obstacleProgram, fs);
    glLinkProgram(obstacleProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenVertexArrays(1, &obstacleVAO);
    glGenBuffers(1, &obstacleVBO);
    glBindVertexArray(obstacleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, obstacleVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// draw a smooth circle by uploading a triangle fan in NDC
void drawObstacleNDC(float cx, float cy, float radius)
{
    const int SEG = 64;
    std::vector<float> verts;
    verts.reserve((SEG+2)*2);
    verts.push_back(cx);
    verts.push_back(cy);
    for (int s=0; s<=SEG; ++s) {
        float a = (float)s / (float)SEG * 2.0f * 3.14159265f;
        float x = cx + cosf(a) * radius;
        float y = cy + sinf(a) * radius;
        verts.push_back(x);
        verts.push_back(y);
    }

    glUseProgram(obstacleProgram);
    glBindVertexArray(obstacleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, obstacleVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

    // set uniforms
    GLint loc_c = glGetUniformLocation(obstacleProgram, "u_center");
    GLint loc_r = glGetUniformLocation(obstacleProgram, "u_radius");
    GLint loc_e = glGetUniformLocation(obstacleProgram, "u_edge");
    if (loc_c >= 0) glUniform2f(loc_c, cx, cy);
    if (loc_r >= 0) glUniform1f(loc_r, radius);
    float edge = fmaxf(0.5f / (float)N, radius * 0.02f);
    if (loc_e >= 0) glUniform1f(loc_e, edge);

    // smooth alpha via blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLE_FAN, 0, SEG+2);

    glDisable(GL_BLEND);
    glBindVertexArray(0);
}


//on vient appliquer les vecteurs sur chaque cellule (pour le moment juste un trait)
/*void drawArrow1(float x1, float y1, float x2, float y2)
{
    std::cout << "drawArrow called\n";

    float vertices[4] = {
        //x1, y1,
        //x2, y2
    };

    glUseProgram(shaderProgramCellsFleche);

    int colorLoc = glGetUniformLocation(shaderProgramCellsFleche, "color");
    glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f); // noir

    glBindVertexArray(flecheVAO);

    glBindBuffer(GL_ARRAY_BUFFER, flecheVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
}*/

void drawArrow(float x1, float y1, float x2, float y2)
{
    glUseProgram(shaderProgramCellsFleche);
    glBindVertexArray(flecheVAO);

    float vertices[4] = {
        x1, y1,
        x2,  y2
    };

    glBindBuffer(GL_ARRAY_BUFFER, flecheVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, 2);
}


void affichagefleche(const Cell& c){
    float x1 = -1.0f + 2.0f * (c.x / (float)gridCols);
    float y1 = -1.0f + 2.0f * (c.y / (float)gridRows);

    float x2 = -1.0f + 2.0f * ((c.x + 1) / (float)gridCols);
    float y2 = -1.0f + 2.0f * ((c.y + 1) / (float)gridRows);

    drawArrow(x1, y1, x2, y2);

    // --- pointe de la fleche ---

    // milieu bord droit
    float x_chd = -1.0f + 2.0f * ((c.x + 1) / (float)gridCols);
    float y_chd = -1.0f + 2.0f * ((c.y + 0.5f) / (float)gridRows);

    // milieu du bord haut
    float x_mbh = -1.0f + 2.0f * ((c.x + 0.5f) / (float)gridCols);
    float y_mbh = -1.0f + 2.0f * ((c.y + 1) / (float)gridRows);

    // deux traits pour la pointe
    drawArrow(x2, y2, x_chd, y_chd);
    drawArrow(x2, y2, x_mbh, y_mbh);

}

void affichagefleche_aleatoire(const Cell& c)
{
    float x1 = -1.0f + 2.0f * (c.x / (float)gridCols);
    float y1 = -1.0f + 2.0f * (c.y / (float)gridRows);

    float scale = 0.4f / gridCols;

    float x2 = x1 + c.vect.getx() * scale;
    float y2 = y1 + c.vect.gety() * scale;

    drawArrow(x1, y1, x2, y2);
}


void affichage_nouveau_fluide(unsigned int shaderProgram)
{
    //printf("STEP SIMU\n"); //OKKK ca marche
    glUseProgram(shaderProgram);

    for(int i = 1; i <= N; i++)
    {
        for(int j = 1; j <= N; j++)
        {
            float r, g, b;
            if (isObstacleCell(i, j)) {
                // obstacle colour (dark gray)
                r = g = b = 0.2f;
            } else {
                float d = dens[IX(i,j)];
                if (d < 0.0f) d = 0.0f;
                else if (d > 1.0f) d = 1.0f;
                r = 0.0f;
                g = 0.0f;
                b = d;
            }

            
            float x1 = -1.0f + 2.0f * ((j-1) / (float)N);
            float y1 = -1.0f + 2.0f * ((i-1) / (float)N);
            float x2 = -1.0f + 2.0f * (j / (float)N);
            float y2 = -1.0f + 2.0f * (i / (float)N);

            float vertices[] = {
                x1, y1,
                x2, y1,
                x2, y2,
                x1, y2
            };

            glBindVertexArray(cellVAO);
            glBindBuffer(GL_ARRAY_BUFFER, cellVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

            int colorLoc = glGetUniformLocation(shaderProgram, "color");
            glUniform4f(colorLoc, r, g, b, 1.0f);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }
}
