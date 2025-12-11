//OpenGL et rendering
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <ostream>
//outils pour la géométrie c++
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
//autre
#include <random>
#include <cstdlib>
#include <ctime>

// Ajout : handles pour la grille
unsigned int gridVAO = 0;
unsigned int gridVBO = 0;
int gridVertexCount = 0;

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


//GESTION FENETRE
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

//Structure pour manipuler points
typedef struct {
    double x;
    double y;
    double z;
    double w;
} Vec4;

//GESTION INPUTS
//gestion input clavier : ici, si KEY_ESCAPE préssée
void processInput(GLFWwindow *window, bool* moveRight, bool* moveLeft, bool* moveUp, bool* moveDown){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    *moveRight = (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
    *moveLeft = (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
    *moveUp = (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
    *moveDown = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
}

//Edit de rendering
// Fonction pour éditer la position du triangle via uniform (exemple avec translation matrix)
void setTrianglePosition(unsigned int shaderProgram, float x, float y, float z = 0.0f, float w = 1.0f) {
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

void setTriangleColorRand(unsigned int shaderProgram) {
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float a = 1.0f;
    setTriangleColor(shaderProgram, r, g, b, a);
}

void makeTriangleSpin(unsigned int shaderProgram, float time) {
    float angle = time;
    //float angle = (float)glfwGetTime();
    float x = 0.5f * cos(angle);
    float y = 0.5f * sin(angle);
    setTrianglePosition(shaderProgram, x, y);
}

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


//créait une grille affichée dans la fenêtre
void generate_grid(int scalev = 20, int scaleh = 20){
    // crée des lignes dans l'espace clip [-1,1] en X et Y
    std::vector<float> verts;
    verts.reserve((scaleh+1 + scalev+1) * 2 * 3);

    // lignes verticales
    for (int i = 0; i <= scaleh; ++i){
        float x = -1.0f + 2.0f * i / (float)scaleh;
        verts.push_back(x); verts.push_back(-1.0f); verts.push_back(0.0f);
        verts.push_back(x); verts.push_back( 1.0f); verts.push_back(0.0f);
    }
    // lignes horizontales
    for (int j = 0; j <= scalev; ++j){
        float y = -1.0f + 2.0f * j / (float)scalev;
        verts.push_back(-1.0f); verts.push_back(y); verts.push_back(0.0f);
        verts.push_back( 1.0f); verts.push_back(y); verts.push_back(0.0f);
    }

    gridVertexCount = (int)verts.size() / 3;

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


int main(int argc, char* argv[]){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    //Création objet fenêtre
    GLFWwindow* window = glfwCreateWindow(800, 600, "Fluid Simulation", NULL, NULL);
    if (window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //Initialisation de GLAD 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //position et dimensions de la fenêtre
    glViewport(0, 0, 800, 600);

    //possibililté de resize 
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    //préciser que l'ont veut qu'il resize régulièrement
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Création d'objets :
    //création 3 points :

    //Genère un buffer et créé son l'ID
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    //associe le buffer à la carte graphique 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //Associe les modifications de buffer à celui qu'on a créé (c'est le buffer de référence)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //paramètres :
    //GL_STREAM_DRAW : La donnée est fixée une seule fois et peu utilisée par le GPU
    //GL_STATIC_DRAW : La donnée est fixée une seule fois et utilisée plusieurs fois
    //GL_DYNAMIC_DRAW : la donnée est changée et utilisée plusieurs fois
    
//DEF DES SHADERS
    //Def de Shader (basique) à travers un C String
    const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform vec4 u_centroid;\n" // centre autour duquel on scale
    "uniform float u_scale;  \n" // 1.05 ou 0.95
    "uniform vec4 offset; \n"    
    "void main()\n"
    "{\n"
    "vec4 centered = vec4(aPos, 1.0) - u_centroid;\n"
    "vec4 scaled = u_centroid + u_scale * centered;\n"
    " gl_Position = scaled + offset;\n"
    "}\0";


    const char *vertexShaderSourceGrid = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform vec4 offset;\n"
    "void main()\n"
    "{\n"
    " gl_Position = vec4(aPos + offset.xyz, 1.0);\n"
    "}\0";

    //création objet Shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER); // -> type de shader
    unsigned int vertexShaderGrid;
    vertexShaderGrid = glCreateShader(GL_VERTEX_SHADER);

    //Association de l'objet et de notre shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glShaderSource(vertexShaderGrid, 1, &vertexShaderSourceGrid, NULL);
    //compilation
    glCompileShader(vertexShader);
    glCompileShader(vertexShaderGrid);

    //Idem que le vertex shader mais avec la couleur
    //Def de Shader 
    const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 color = vec4(1.0f, 0.2f, 0.2f, 1.0f);\n"
    "void main()\n"
    "{\n"
    " FragColor = color;\n"
    "}\0";
//vec4(1.0f, 0.2f, 0.2f, 1.0f)

    const char *fragmentShaderSourceGrid = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 color=vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "void main()\n"
    "{\n"
    " FragColor = color;\n"
    "}\0";


//création objet Shader
    unsigned int fragmentShader;
    unsigned int fragmentShaderGrid;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // -> type de shader
    fragmentShaderGrid = glCreateShader(GL_FRAGMENT_SHADER);
    //Association de l'objet et de notre shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glShaderSource(fragmentShaderGrid, 1, &fragmentShaderSourceGrid, NULL);
    //compilation
    glCompileShader(fragmentShader);
    glCompileShader(fragmentShaderGrid);

    //Creer objet programme
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    unsigned int shaderProgramGrid;
    shaderProgramGrid = glCreateProgram();

    //attache les objets au programme
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgramGrid, vertexShaderGrid);
    glAttachShader(shaderProgramGrid, fragmentShaderGrid);
    glLinkProgram(shaderProgram);
    glLinkProgram(shaderProgramGrid);

    //appel au programme 
    glUseProgram(shaderProgram);
    glUseProgram(shaderProgramGrid);

    //On supprime les objets après les avoir attaché
    glDeleteShader(vertexShader);
    glDeleteShader(vertexShaderGrid);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShaderGrid);

    //On précise à OpenGL comment interpréter nos données pour les afficher
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),  (void*)0);
    glEnableVertexAttribArray(0);

    
//Manipulation d'objet sans structure :
    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 1. then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
    (void*)0);
    glEnableVertexAttribArray(0);
    // 2. use our shader program when we want to render an object
    glUseProgram(shaderProgram);
    // 3. now draw the object
    //someOpenGLFunctionThatDrawsOurTriangle();

//Manipulation d'objet avec structure :
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. then set our vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
    (void*)0);
    glEnableVertexAttribArray(0);
    // Drawing code (in render loop) :: ..
    // 4. draw the object
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    //someOpenGLFunctionThatDrawsOurTriangle();

    // Initialiser les uniforms du dilate triangle
    float cx = (vertices[0] + vertices[3] + vertices[6]) / 3.0f;
    float cy = (vertices[1] + vertices[4] + vertices[7]) / 3.0f;
    float cz = (vertices[2] + vertices[5] + vertices[8]) / 3.0f;
    GLint loc_centroid = glGetUniformLocation(shaderProgram, "u_centroid");
    GLint loc_scale = glGetUniformLocation(shaderProgram, "u_scale");
    glUniform4f(loc_centroid, cx, cy, cz, 1.0f);
    glUniform1f(loc_scale, 1.0f);

    //génère grille ou pas
    generate_grid(10, 15);


    //render loop (maintient la fenêtre ouverte, une loop = une frame)
    //se divise en 4 parties : nettoyage, input, render puis cloture
    while(!glfwWindowShouldClose(window)){
    //P1 : nettoyage
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // Aussi GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT
        
        
    //P2 : gestion input clavier
        bool moveRight = false;
        bool moveLeft = false;
        bool moveUp = false;
        bool moveDown = false;
        processInput(window, &moveRight, &moveLeft, &moveUp, &moveDown);

    //P3 : gestion du render
        //Attention : au choix du programme Shader utilisé

        //Dessine la grille
        
        glUseProgram(shaderProgramGrid);
        if (gridVAO != 0 && gridVertexCount > 0) {
            glLineWidth(1.5f); //épaisseur des lignes
            glBindVertexArray(gridVAO);
            glDrawArrays(GL_LINES, 0, gridVertexCount);
        }

        //dessin du triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        
        // Accumuler la dilatation et la température
        if(moveUp){    
            currentScale *= 1.01f;  // Augmenter de 1% par frame
            heatTriangle(shaderProgram, -0.01f);  // Augmenter la température
        }
        if(moveDown){    
            currentScale *= 0.99f;  // Diminuer de 1% par frame
            heatTriangle(shaderProgram, 0.01f);  // Diminuer la température
        }
        
        // Réinitialiser les uniforms à chaque frame
        float cx = (vertices[0] + vertices[3] + vertices[6]) / 3.0f;
        float cy = (vertices[1] + vertices[4] + vertices[7]) / 3.0f;
        float cz = (vertices[2] + vertices[5] + vertices[8]) / 3.0f;
        GLint loc_centroid = glGetUniformLocation(shaderProgram, "u_centroid");
        GLint loc_scale = glGetUniformLocation(shaderProgram, "u_scale");
        GLint loc_offset = glGetUniformLocation(shaderProgram, "offset");
        glUniform4f(loc_centroid, cx, cy, cz, 1.0f);
        glUniform1f(loc_scale, currentScale);
        glUniform4f(loc_offset, 0.0f, 0.0f, 0.0f, 0.0f);
        
        if(moveRight){    
            setTriangleColorRand(shaderProgram);
        }
        if(moveLeft){    
            currentScale = 1.0f;  // Annuler la dilatation quand on tourne
            makeTriangleSpin(shaderProgram, (float)glfwGetTime());
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        
    //P4 : fin render loop
        //met les pixels en couleur
        glfwSwapBuffers(window);
        //vérifie si un input a été trigger
        glfwPollEvents();
    }

    printf("fenêtre de fluides fermée\n");
    glfwTerminate();
    return 0;
}

