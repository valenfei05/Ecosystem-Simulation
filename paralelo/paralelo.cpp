#include "simulacion_paralela.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <omp.h>

using namespace std;

// Lee el numero de hilos y verifica que sea positivo.
bool leerNumeroHilos(const string& texto, int& hilos) {
    istringstream entrada(texto);
    bool valido = static_cast<bool>(entrada >> hilos);
    if (valido) {
        entrada >> ws;
        valido = entrada.eof() && hilos > 0;
    }
    return valido;
}

int main(int argc, char* argv[]) {
    int estado = 0;
    try {
        int hilos = omp_get_max_threads();
        bool estadisticas = false;
        bool argumentosValidos = true;
        bool hilosIndicados = false;

        for (int i = 1; i < argc; ++i) {
            string argumento = argv[i];
            if (argumento == "--hilos" && !hilosIndicados && i + 1 < argc) {
                ++i;
                hilosIndicados = true;
                argumentosValidos = leerNumeroHilos(argv[i], hilos) && argumentosValidos;
            } else if (argumento == "--estadisticas" && !estadisticas) {
                estadisticas = true;
            } else {
                argumentosValidos = false;
            }
        }

        if (!argumentosValidos) {
            cerr << "Uso: paralelo [--hilos N] [--estadisticas] < entrada.in\n";
            estado = 2;
        } else {
            ecosystem::Mundo mundo = ecosystem::leerMundo(cin);
            ecosystem::Observador observar;
            if (estadisticas) {
                cerr << "generacion,conejos,zorros,rocas\n";
                observar = [](const ecosystem::Mundo& actual, const ecosystem::Poblacion& p) {
                    cerr << actual.generacion << ',' << p.conejos << ','
                         << p.zorros << ',' << p.rocas << '\n';
                };
            }
            ecosystem::simularParalelo(mundo, hilos, observar);
            ecosystem::imprimirMundo(mundo, cout);
            if (!cout) {
                cerr << "Error al escribir el resultado.\n";
                estado = 1;
            }
        }
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        estado = 1;
    }
    return estado;
}
