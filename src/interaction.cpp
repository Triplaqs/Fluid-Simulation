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
#include "serialib.h" 

serial serialPort;
float fluidDensity = 1.0f;
float fluidDirection = 0.0f;

void updateBluetooth() {
    char buffer[32];
    // On lit si quelque chose arrive sur le port COM (ex: "COM3")
    if (serialPort.readString(buffer, '\n', 32, 0) > 0) {
        std::string msg(buffer);
        
        // On cherche le séparateur
        size_t sep = msg.find(":");
        if (sep != std::string::npos) {
            std::string id = msg.substr(0, sep);
            float value = std::stof(msg.substr(sep + 1));

            // On met à jour les variables de la simulation
            if (id == "01") fluidDensity = value;
            else if (id == "02") fluidDirection = value;
        }
    }
}
