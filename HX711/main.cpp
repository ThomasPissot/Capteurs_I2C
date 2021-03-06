/**
 *  @file     main.cpp
 *  @author   Philippe SIMIER (Touchard Wahington le Mans)
 *  @license  BSD (see license.txt)
 *  @date     15 avril 2018
 *  @brief    programme balance
 *            compilation: g++ main.cpp hx711.cpp  spi.c -o main
*/

#include <iostream>
#include <iomanip>
#include "hx711.h"


using namespace std;

int main()
{

    hx711 balance;
    float xn;
    float yn   = 0.0;
    float yn_1 = 0.0;
    char  stable;

    balance.effectuerTarage();
    while(1)
    {
	xn = balance.obtenirPoids();
	// Filtrage passe bas du premier ordre constante de temps 2.Te  (20ms)
        // Filtre passe bas du premier ordre constante de temps 10.Te (100ms) 
        // yn = 0,091.xn + 0,91.yn-1;
	yn = 0.33 * xn + 0.66 * yn_1;

	// calcul de la dérivée
	if ((yn_1 - yn) < 0.1 && (yn_1 - yn) > - 0.1)
            stable = '*';
        else stable = ' ';
        yn_1 = yn;

        cout << stable << " " << yn << fixed << setprecision (1) << " g" << endl;
        usleep(100000);
        system("clear");
    }
}
