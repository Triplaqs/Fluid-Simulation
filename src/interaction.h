#ifndef INTERACTION_H
#define INTERACTION_H

//On associe chaque paramètre à un numéro !

/*
01 : Densité
02 : Direction
*/

//tente de se connecter au Bluetooth
void initBluetooth(const char* portCOM);
//mets à jour les données bluetooth
void updateBluetooth();

#endif 