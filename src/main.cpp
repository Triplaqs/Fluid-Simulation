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



//créait une grille affichée dans la fenêtre (affichage seulement)
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

// Helper: initialize the cell grid -- create quad vertices and buffers
void initCellsGrid(int cols = 100, int rows = 100){
    gridCols = cols;
    gridRows = rows;
    cells.assign(cols * rows, 0);
    cellsNext.assign(cols * rows, 0);

    float cellW = 2.0f / (float)cols;  // in NDC (-1..1)
    float cellH = 2.0f / (float)rows;

    cellVertices.clear();
    cellColors.clear();
    cellVertices.reserve(cols * rows * 6 * 3);
    cellColors.reserve(cols * rows * 6 * 3);

    for(int y = 0; y < rows; ++y){
        for(int x = 0; x < cols; ++x){
            float left = -1.0f + x * cellW;
            float right = left + cellW;
            float bottom = -1.0f + y * cellH;
            float top = bottom + cellH;
            // two triangles per cell
            // tri 1
            cellVertices.push_back(left);  cellVertices.push_back(bottom); cellVertices.push_back(0.0f);
            cellVertices.push_back(right); cellVertices.push_back(bottom); cellVertices.push_back(0.0f);
            cellVertices.push_back(right); cellVertices.push_back(top);    cellVertices.push_back(0.0f);
            // tri 2
            cellVertices.push_back(left);  cellVertices.push_back(bottom); cellVertices.push_back(0.0f);
            cellVertices.push_back(right); cellVertices.push_back(top);    cellVertices.push_back(0.0f);
            cellVertices.push_back(left);  cellVertices.push_back(top);    cellVertices.push_back(0.0f);

            // default color black (dead)
            for(int v = 0; v < 6; ++v){
                cellColors.push_back(0.0f);
                cellColors.push_back(0.0f);
                cellColors.push_back(0.0f);
            }
        }
    }

    cellsVertexCount = (int)cellVertices.size() / 3;

    if(cellsVAO == 0) glGenVertexArrays(1, &cellsVAO);
    if(cellsVBO == 0) glGenBuffers(1, &cellsVBO);
    if(cellsCBO == 0) glGenBuffers(1, &cellsCBO);

    glBindVertexArray(cellsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    glBufferData(GL_ARRAY_BUFFER, cellVertices.size() * sizeof(float), cellVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, cellsCBO);
    glBufferData(GL_ARRAY_BUFFER, cellColors.size() * sizeof(float), cellColors.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// Fill cells randomly
void randomizeCells(float aliveProbability=0.2f){
    std::mt19937 rng((unsigned int)time(NULL));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for(size_t i = 0; i < cells.size(); ++i){
        cells[i] = (dist(rng) < aliveProbability) ? 1 : 0;
    }
    // update colors buffer
    for(int i = 0; i < gridCols * gridRows; ++i){
        int base = i * 6 * 3;
        float r = cells[i] ? 1.0f : 0.0f;
        float g = cells[i] ? 1.0f : 0.0f;
        float b = cells[i] ? 1.0f : 0.0f;
        for(int v = 0; v < 6; ++v){
            cellColors[base + v*3 + 0] = r;
            cellColors[base + v*3 + 1] = g;
            cellColors[base + v*3 + 2] = b;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, cellsCBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cellColors.size() * sizeof(float), cellColors.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// update Game of Life rules (conway-like)
void updateSimulation(){
    for(int y = 0; y < gridRows; ++y){
        for(int x = 0; x < gridCols; ++x){
            int idx = y * gridCols + x;
            int neighbors = 0;
            for(int oy = -1; oy <= 1; ++oy){
                for(int ox = -1; ox <= 1; ++ox){
                    if(ox == 0 && oy == 0) continue;
                    int nx = x + ox;
                    int ny = y + oy;
                    // wrap around (toroidal) or clamp; let's wrap
                    if(nx < 0) nx = gridCols - 1; if(nx >= gridCols) nx = 0;
                    if(ny < 0) ny = gridRows - 1; if(ny >= gridRows) ny = 0;
                    neighbors += cells[ny * gridCols + nx];
                }
            }
            if(cells[idx]){
                // alive -> survive only with 2 or 3 neighbors
                cellsNext[idx] = //moyenne de toutes les cases voisines
            } else {
                // dead -> become alive with exactly 3 neighbors
                cellsNext[idx] = (neighbors == 3) ? 1 : 0;
            }
        }
    }
    // swap and update colors
    cells.swap(cellsNext);
    for(int i = 0; i < gridCols * gridRows; ++i){
        int base = i * 6 * 3;
        float r = cells[i] ? 1.0f : 0.0f;
        float g = cells[i] ? 1.0f : 0.0f;
        float b = cells[i] ? 1.0f : 0.0f;
        for(int v = 0; v < 6; ++v){
            cellColors[base + v*3 + 0] = r;
            cellColors[base + v*3 + 1] = g;
            cellColors[base + v*3 + 2] = b;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, cellsCBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cellColors.size() * sizeof(float), cellColors.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}



/*Cell createCellOld(float x0, float y0, float x1, float y1) {
    Cell cell;
    cell.x = (x0 + x1) / 2.0f;
    cell.y = (y0 + y1) / 2.0f;
    cell.temperature = 0.0f;
    
    // Triangle 1 :  bas-gauche, bas-droit, haut-droit
    float vertices1[] = {
        x0, y0, 0.0f,
        x1, y0, 0.0f,
        x1, y1, 0.0f
    };
    
    // Triangle 2 : bas-gauche, haut-droit, haut-gauche
    float vertices2[] = {
        x0, y0, 0.0f,
        x1, y1, 0.0f,
        x0, y1, 0.0f
    };
    
    // === Création Triangle 1 ===
    glGenVertexArrays(1, &cell.VAO1);
    glGenBuffers(1, &cell.VBO1);
    
    glBindVertexArray(cell.VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, cell.VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // === Création Triangle 2 ===
    glGenVertexArrays(1, &cell.VAO2);
    glGenBuffers(1, &cell.VBO2);
    
    glBindVertexArray(cell.VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, cell.VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return cell;
}*/




//Met à jour la simulation (diffusion)
void updateSimulation(unsigned int shaderProgram){
    //float newVals[gridRows][gridCols]; (pas besoin de matrice)
    for(int y = 0; y < gridRows; ++y){
        for(int x = 0; x < gridCols; ++x){
            //ordonnancement linéaire de la grille
            int idx = y * gridCols + x;

            //calcul de la moyenne des voisins
            float mean = 0.0f;
            for(int oy = -1; oy <= 1; ++oy){
                for(int ox = -1; ox <= 1; ++ox){
                    if(ox == 0 && oy == 0) continue;
                    if(cells[idx].bh() && oy == -1) continue;
                    if(cells[idx].bb() && oy == 1) continue;
                    if(cells[idx].bg() && ox == -1) continue;
                    if(cells[idx].bd() && ox == 1) continue;
                    int nx = x + ox;
                    int ny = y + oy;
                    mean += cells[nx + ny * gridCols].temperature;
                }
            }
            //Ici aux bords on prend de l'autre coté, par soucis de simplicité (on modifira après hein)
            mean /= cells[idx].nbVoisins();
            //update
            cellsNext[idx].temperature = mean; //moyenne de toutes les cases voisines
            //changement de couleur
            //heatCells(shaderProgram, cells[idx], mean);
        
            //Askip le rendemendent je dois le faire dans la boucle de rendue... ici seulement calculs
        }
    }
    // passe le contenu de CellsNext dans Cells
    cells.swap(cellsNext);
    /*for(int i = 0; i < gridCols * gridRows; ++i){
        
    }*/
    /*glBindBuffer(GL_ARRAY_BUFFER, cellsCBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cellColors.size() * sizeof(float), cellColors.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    */
}


int main(int argc, char* argv[]){
    //ça c'est si on demande la précision :D
    /*std::string prec;
    std::cout << "Which precision for the display ? (from 50 to 200)\n";
    std::cin >> prec;*/
    //mais on la demande pas ;)
    std::string prec = "100";
    
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

    const char *vertexShaderSourceCells = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 vColor;\n"
    "void main()\n"
    "{\n"
    "    vColor = aColor;\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\0";

    
    //FS rudimentaire pour cellules
    /*const char *fragmentShaderSourceCells = "#version 330 core\n"
    "in vec3 vColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(vColor, 1.0);\n"
    "}\0";*/

    const char *fragmentShaderSourceCells = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 color;\n"
    "void main()\n"
    "{\n"
    "    FragColor = color;\n"
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
    
    // Compile cells shaders
    unsigned int vertexShaderCells;
    unsigned int fragmentShaderCells;
    vertexShaderCells = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderCells = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShaderCells, 1, &vertexShaderSourceCells, NULL);
    glShaderSource(fragmentShaderCells, 1, &fragmentShaderSourceCells, NULL);
    glCompileShader(vertexShaderCells);
    glCompileShader(fragmentShaderCells);

    //Creer objet programme
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    unsigned int shaderProgramGrid;
    shaderProgramGrid = glCreateProgram();
    unsigned int shaderProgramCells;
    shaderProgramCells = glCreateProgram();

    //attache les objets au programme
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgramGrid, vertexShaderGrid);
    glAttachShader(shaderProgramGrid, fragmentShaderGrid);
    glAttachShader(shaderProgramCells, vertexShaderCells);
    glAttachShader(shaderProgramCells, fragmentShaderCells);
    glLinkProgram(shaderProgram);
    glLinkProgram(shaderProgramGrid);
    glLinkProgram(shaderProgramCells);

    //appel au programme 
    glUseProgram(shaderProgram);
    glUseProgram(shaderProgramGrid);
    glUseProgram(shaderProgramCells);

    //On supprime les objets après les avoir attaché
    glDeleteShader(vertexShader);
    glDeleteShader(vertexShaderGrid);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShaderGrid);
    glDeleteShader(vertexShaderCells);
    glDeleteShader(fragmentShaderCells);

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
    //generate_grid(10, 15);

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
        for (const Cell& c : cells) {
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
        //old method
        /*
        glBindVertexArray(cellsVAO);
        if(cellsVertexCount > 0){
            glDrawArrays(GL_TRIANGLES, 0, cellsVertexCount);
        }*/

        //on s'en blc de la grille déssinée
        /*
        //Dessine la grille
        glUseProgram(shaderProgramGrid);
        if (gridVAO != 0 && gridVertexCount > 0) {
            glLineWidth(1.5f); //épaisseur des lignes
            glBindVertexArray(gridVAO);
            glDrawArrays(GL_LINES, 0, gridVertexCount);
        }*/

        //dessin du triangle
        //Plus besoin
        //glUseProgram(shaderProgram);
        //glBindVertexArray(VAO);
        
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
        
        //ancien tests triangle
        /*
        if(moveRight){    
            setTriangleColorRand(shaderProgram);
        }
        if(moveLeft){    
            currentScale = 1.0f;  // Annuler la dilatation quand on tourne
            makeTriangleSpin(shaderProgram, (float)glfwGetTime());
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);*/
        
        
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
