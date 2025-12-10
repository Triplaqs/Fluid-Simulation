//OpenGL et rendering
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <iostream>
#include <ostream>
//outils pour la géométrie c++
#include <vector>
#include <iostream>
#include <glm/glm.hpp>


// Ajout : handles pour la grille
unsigned int gridVAO = 0;
unsigned int gridVBO = 0;
int gridVertexCount = 0;


//GESTION FENETRE
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


//GESTION INPUTS
//gestion input clavier : ici, si KEY_ESCAPE préssée
void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
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
    float vertices[] = {
        -0.75f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

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
    "void main()\n"
    "{\n"
    " gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

    //création objet Shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER); // -> type de shader

    //Association de l'objet et de notre shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    //compilation
    glCompileShader(vertexShader);

    //Idem que le vertex shader mais avec la couleur
    //Def de Shader 
    const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    " FragColor = vec4(1.0f, 0.2f, 0.2f, 1.0f);\n"
    "}\0";

    const char *fragmentShaderSourceGrid = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    " FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
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
    glAttachShader(shaderProgramGrid, vertexShader);
    glAttachShader(shaderProgramGrid, fragmentShaderGrid);
    glLinkProgram(shaderProgram);
    glLinkProgram(shaderProgramGrid);

    //appel au programme 
    glUseProgram(shaderProgram);
    glUseProgram(shaderProgramGrid);

    //On supprime les objets après les avoir attaché
    glDeleteShader(vertexShader);
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

    generate_grid(10, 10);


    //render loop (maintient la fenêtre ouverte, une loop = une frame)
    //se divise en 4 parties : nettoyage, input, render puis cloture
    while(!glfwWindowShouldClose(window)){
    //P1 : nettoyage
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // Aussi GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT
        
        
    //P2 : gestion input clavier
        processInput(window);

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

