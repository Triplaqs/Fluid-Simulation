//OpenGL et rendering
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <ostream>
//Les autres modules du projet (headers)
#include "utils.h"
#include "interaction.h"
#include "fluides.h"
#include "display.h"
//outils pour la géométrie c++
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
//autre
#include <random>
#include <cstdlib>
#include <ctime>
#include <chrono>

//ligne pour générer un aléatoire :
//float entre 0 et 1 :
//float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
//int entre 0 et N :
//int k = rand() % N+1;
//int entre a et b :
//int k = rand() % (b-a+1) + a;


//GESTION FENETRE
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


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


int main(int argc, char* argv[]){
    //ça c'est si on demande la précision :D
    /*std::string prec;
    std::cout << "Which precision for the display ? (from 50 to 200)\n";
    std::cin >> prec;*/
    //mais on la demande pas ;)
    std::string prec = "100";
    
    //glfwInit();
    //après erreur de génération : tests :
    if (!glfwInit()) {
        std::cerr << "GLFW init failed" << std::endl;
    return -1;
    }

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
    //Def de Shader (basique) à travers un C Strings
    const char *vertexShaderSourceCells = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 vColor;\n"
    "void main()\n"
    "{\n"
    "    vColor = aColor;\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\0";
    
    //Idem que le vertex shader mais avec la couleur
    const char *fragmentShaderSourceCells = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 color;\n"
    "void main()\n"
    "{\n"
    "    FragColor = color;\n"
    "}\0";

    //création objet Shader
    unsigned int vertexShaderCells;
    unsigned int fragmentShaderCells;
    //Definition du type de Shader
    vertexShaderCells = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderCells = glCreateShader(GL_FRAGMENT_SHADER);
    //Association de l'objet et de notre shader
    glShaderSource(vertexShaderCells, 1, &vertexShaderSourceCells, NULL);
    glShaderSource(fragmentShaderCells, 1, &fragmentShaderSourceCells, NULL);
    //compilation
    glCompileShader(vertexShaderCells);
    glCompileShader(fragmentShaderCells);

    //Creer objet programme
    unsigned int shaderProgramCells;
    shaderProgramCells = glCreateProgram();

    //attache les objets au programme
    glAttachShader(shaderProgramCells, vertexShaderCells);
    glAttachShader(shaderProgramCells, fragmentShaderCells);
    glLinkProgram(shaderProgramCells);

    //appel au programme 
    glUseProgram(shaderProgramCells);

    //On supprime les objets après les avoir attaché
    glDeleteShader(vertexShaderCells);
    glDeleteShader(fragmentShaderCells);

    //On précise à OpenGL comment interpréter nos données pour les afficher
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),  (void*)0);
    glEnableVertexAttribArray(0);

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

    // Initialize cell grid for Game of Life-like animation
    initCellsGrid(std::stoi(prec), std::stoi(prec));
    gridCols = std::stoi(prec);
    gridRows = std::stoi(prec);
    randomizeCells();
    //lastStepTime = std::chrono::steady_clock::now();


//render loop (maintient la fenêtre ouverte, une loop = une frame)
    //se divise en 4 parties : nettoyage, input, render puis cloture
    while(!glfwWindowShouldClose(window)){
    //P1 : nettoyage
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // Aussi GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT
        
        
    //P2 : gestion input clavier
        //Basiques
        bool moveRight = false;
        bool moveLeft = false;
        bool moveUp = false;
        bool moveDown = false;
        processInput(window, &moveRight, &moveLeft, &moveUp, &moveDown);
        //Simulation
        static bool lastSpacePressed = false;
        static bool lastRPressed = false;
        static bool lastNPressed = false;
        bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool rPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        bool nPressed = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;

    //P3 : gestion du render
        //Attention : au choix du programme Shader utilisé

        // Dessine les cellules
        glUseProgram(shaderProgramCells);
        for (const Cell& c : cells.grid) {
            glBindVertexArray(c.VAO1);
            //une des deux fonctions ne marche pas
            setTriangleColor(shaderProgramCells, c.temperature, 0.0f, 1.0f - c.temperature, 1.0f);
            //heatTriangle(shaderProgram, c.temperature);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            glBindVertexArray(c.VAO2);
            setTriangleColor(shaderProgramCells, c.temperature, 0.0f, 1.0f - c.temperature, 1.0f);
            //heatTriangle(shaderProgram, c.temperature);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        
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
        
        
    //P4 : fin render loop
        //met les pixels en couleur
        glfwSwapBuffers(window);
        //vérifie si un input a été trigger
        glfwPollEvents();

        // Contrôles de la simulation, à editer si besoin
        if(spacePressed && !lastSpacePressed){ simRunning = !simRunning; }
        if(rPressed && !lastRPressed){ randomizeCells(); }
        if(nPressed && !lastNPressed){ updateSimulation(shaderProgramCells); }
        lastSpacePressed = spacePressed;
        lastRPressed = rPressed;
        lastNPressed = nPressed;

        // Simulation stepping
        if(simRunning){
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> diff = now - lastStepTime;
            if(diff.count() >= simStepSeconds){
                updateSimulation(shaderProgramCells);
                lastStepTime = now;
            }
        }
    }

    printf("fenêtre de fluides fermée\n");
    glfwTerminate();
    return 0;
}
