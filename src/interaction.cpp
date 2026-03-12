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
//Pour le Bluetooth
#include <iostream>
#include <string>
// Utilise une lib simple comme "serialib.h" pour Windows/Linux
#include "../serial/serialib.h" 


// On ouvre la connexion À appeler une seule fois avant ta boucle OpenGL (ex: dans ton main)
void initBluetooth(const char* portCOM) {
    // Tente d'ouvrir le port à 115200 bauds
    if (serialPort.openDevice(portCOM, 115200) != 1) {
        std::cerr << "Warning : Could not connect to Bluetooth on " << portCOM << std::endl;
    } else {
        std::cout << "Success : Bluetooth connected on " << portCOM << " !" << std::endl;
    }
}

void updateBluetooth() {
    char buffer[32];
    
    if (serialPort.readString(buffer, '\n', 31, 0) > 0) {
        std::string msg(buffer);
        size_t sep = msg.find(":");
        
        if (sep != std::string::npos) {
            std::string id = msg.substr(0, sep);
            try {
                float value = std::stof(msg.substr(sep + 1));

                // --- TU MODIFIES TES GLOBALES DIRECTEMENT ICI ---
                if (id == "01") {
                    glob = value;  // Met à jour la densité
                }
                else if (id == "02") {
                    angle = value; // Met à jour la direction
                }
            } catch (const std::exception& e) {
                std::cerr << "Erreur de conversion" << std::endl;
            }
        }
    }
}
