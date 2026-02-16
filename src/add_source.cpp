#include <vector>
#include <iostream>
#include "matrix.h"
#include <math.h>

//fonction pour ajouter une source à une frame
//s[] est un tableau qui contient les sources d'une frame
void add_source ( int N, float * x, float * s, float dt )
{
    int i, size=(N+2)*(N+2);
    for ( i=0 ; i<size ; i++ ) x[i] += dt*s[i];
}
