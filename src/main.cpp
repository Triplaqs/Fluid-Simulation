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
#include "matrix.h"
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


//définition variables globales
// --- VAO/VBO + shader pour les flèches ---
unsigned int flecheVAO = 0;
unsigned int flecheVBO = 0;
unsigned int shaderProgramCellsFleche = 0;


//pour implémentation stam
/*int N = 100; 
float dt = 0.1f;
float diff = 0.0f;
float visc = 0.0f;

std::vector<float> u, v, u_prev, v_prev;
std::vector<float> dens, dens_prev;*/



//définition de la macro
#define IX(i,j) ((i) + (N+2) * (j))




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
    const char *vertexShaderSourceCellsTemp = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 vColor;\n"
    "void main()\n"
    "{\n"
    "    vColor = aColor;\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\0";
    
    //Idem que le vertex shader mais avec la couleur
    const char *fragmentShaderSourceCellsTemp = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 color;\n"
    "void main()\n"
    "{\n"
    "    FragColor = color;\n"
    "}\0";

    // --- SHADER FLECHE ---
    // Source du vertex shader flèche (positions uniquement)
    const char *vertexShaderSourceCellsFleche =
        "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "void main()\n"
        "{\n"
            "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}\n";

    
    // Source du fragment shader flèche (couleur uniforme)
    const char *fragmentShaderSourceCellsFleche =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec4 color;\n"
        "void main()\n"
        "{\n"
            "    FragColor = color;\n"
        "}\n";

    //création objet Shader
    unsigned int vertexShaderCellsTemp;
    unsigned int fragmentShaderCellsTemp;

    //Definition du type de Shader
    vertexShaderCellsTemp = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderCellsTemp = glCreateShader(GL_FRAGMENT_SHADER);
    //Association de l'objet et de notre shader
    glShaderSource(vertexShaderCellsTemp, 1, &vertexShaderSourceCellsTemp, NULL);
    glShaderSource(fragmentShaderCellsTemp, 1, &fragmentShaderSourceCellsTemp, NULL);
    //compilation
    glCompileShader(vertexShaderCellsTemp);
    glCompileShader(fragmentShaderCellsTemp);

    //Creer objet programme
    unsigned int shaderProgramCellsTemp;
    shaderProgramCellsTemp = glCreateProgram();

    //attache les objets au programme
    glAttachShader(shaderProgramCellsTemp, vertexShaderCellsTemp);
    glAttachShader(shaderProgramCellsTemp, fragmentShaderCellsTemp);
    glLinkProgram(shaderProgramCellsTemp);

    //appel au programme 
    glUseProgram(shaderProgramCellsTemp);

    //On supprime les objets après les avoir attaché
    glDeleteShader(vertexShaderCellsTemp);
    glDeleteShader(fragmentShaderCellsTemp);

    //On précise à OpenGL comment interpréter nos données pour les afficher
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),  (void*)0);
    glEnableVertexAttribArray(0);
    //------------------------------------------fin shader cellule-----------------------------



    //-----------------------------------------début shader fleche--------------------------------

    //création objet Shader
    unsigned int vertexShaderCellsFleche;
    unsigned int fragmentShaderCellsFleche;

    //Definition du type de Shader
    vertexShaderCellsFleche = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderCellsFleche = glCreateShader(GL_FRAGMENT_SHADER);
    //Association de l'objet et de notre shader
    glShaderSource(vertexShaderCellsFleche, 1, &vertexShaderSourceCellsFleche, NULL);
    glShaderSource(fragmentShaderCellsFleche, 1, &fragmentShaderSourceCellsFleche, NULL);
    //compilation
    glCompileShader(vertexShaderCellsFleche);
    glCompileShader(fragmentShaderCellsFleche);

    //Creer objet programme
    //unsigned int shaderProgramCellsFleche;
    shaderProgramCellsFleche = glCreateProgram();

    //attache les objets au programme
    glAttachShader(shaderProgramCellsFleche, vertexShaderCellsFleche);
    glAttachShader(shaderProgramCellsFleche, fragmentShaderCellsFleche);
    glLinkProgram(shaderProgramCellsFleche);

    //appel au programme 
    //glUseProgram(shaderProgramCellsFleche);

    //On supprime les objets après les avoir attaché
    glDeleteShader(vertexShaderCellsFleche);
    glDeleteShader(fragmentShaderCellsFleche);

    //On précise à OpenGL comment interpréter nos données pour les afficher
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),  (void*)0);
    //glEnableVertexAttribArray(0);
    //std::cout << "flecheVAO = " << flecheVAO << "\n";
    //std::cout << "flecheVBO = " << flecheVBO << "\n";
    //------------------------------------------fin shader fleche-----------------------------



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


    //essai en supprimant ces deux lignes (celles qui écrasent mon shader fleche)
    //glUseProgram(shaderProgramCellsTemp);
    //glBindVertexArray(VAO);




    // --- Création VAO/VBO pour les flèches ---
    glGenVertexArrays(1, &flecheVAO);
    glGenBuffers(1, &flecheVBO);

    glBindVertexArray(flecheVAO);
    glBindBuffer(GL_ARRAY_BUFFER, flecheVBO);

    // 2 floats par vertex (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);


    // Initialize cell grid for Game of Life-like animation
    initCellsGrid(std::stoi(prec), std::stoi(prec));
    gridCols = std::stoi(prec);
    gridRows = std::stoi(prec);
    //ON essaye de recup les dimensions de la fenêtre
    GLint m_viewport[4];
    glGetIntegerv( GL_VIEWPORT, m_viewport );
    //On set des valeurs au hasard pour la température des cellules
    randomizeCells();
    //lastStepTime = std::chrono::steady_clock::now();


//render loop (maintient la fenêtre ouverte, une loop = une frame)
    //se divise en 4 parties : nettoyage, input, render puis cloture
    while(!glfwWindowShouldClose(window)){
        //mesure du temps
        float currentTime = glfwGetTime();
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
        //Choisir le shader d'affichage à utiliser grâce à cells.aff
        if(cells.aff_mode==0){
            glUseProgram(shaderProgramCellsTemp);
            for (const Cell& c : cells.grid) {
                glBindVertexArray(c.VAO1);
                setTriangleColor(shaderProgramCellsTemp, c.temperature, 0.0f, 1.0f - c.temperature, 1.0f);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            
                glBindVertexArray(c.VAO2);
                setTriangleColor(shaderProgramCellsTemp, c.temperature, 0.0f, 1.0f - c.temperature, 1.0f);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
        else if (cells.aff_mode == 1) {
            srand(time(nullptr));

            randomizeVecs();
            // Activer le shader flèche
            glUseProgram(shaderProgramCellsFleche);

            // Activer le VAO flèche
            glBindVertexArray(flecheVAO);

            for (const Cell& c : cells.grid) {
                //affichagefleche(c);
                affichagefleche_aleatoire(c);
            }
        }

        //Gestion de pression des touches :
        if(start_press >= 0.0f){
            //Fragmentation du cooldown
            t_press = (currentTime - start_press)/duree_cooldown;
            //Fin du cooldown
            if (t_press >= 1.0f){
                t_press = 1.0f;
                start_press = -1.0f;
            }
        }

        
        
        // Gestion inputs
        if(moveUp && (start_press<0.0f)){    
            start_press = currentTime;
            if(cells.aff_mode==cells.mode_max){
                cells.aff_mode=0;
            } else{
                cells.aff_mode+=1;
            }
        }
        if(moveDown && (start_press<0.0f)){    
            start_press = currentTime;
            if(cells.aff_mode==0){
                //randomizeVecs();
                cells.aff_mode=cells.mode_max;
            } else{
                //randomizeVecs();
                cells.aff_mode-=1;
            }
        }
        
        
//P4 : fin render loop
        //met les pixels en couleur
        glfwSwapBuffers(window);
        //vérifie si un input a été trigger
        glfwPollEvents();

        // Contrôles de la simulation, à editer si besoin
        if(spacePressed && !lastSpacePressed){ simRunning = !simRunning; }
        if(rPressed && !lastRPressed){
            start_press = -1;
            randomizeCells(); 
            //randomizeVecs();
        }
        if(nPressed && !lastNPressed){ updateSimulation(shaderProgramCellsTemp); }
        lastSpacePressed = spacePressed;
        lastRPressed = rPressed;
        lastNPressed = nPressed;

        // Simulation stepping
        if(simRunning){
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> diff = now - lastStepTime;
            if(diff.count() >= simStepSeconds){
                updateSimulation(shaderProgramCellsTemp);
                lastStepTime = now;
            }
        }
    }

    printf("fenêtre de fluides fermée\n");
    glfwTerminate();
    return 0;
}
