#include "simulacion_paralela.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
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
        string rutaTraza;

        for (int i = 1; i < argc; ++i) {
            string argumento = argv[i];
            if (argumento == "--hilos" && !hilosIndicados && i + 1 < argc) {
                ++i;
                hilosIndicados = true;
                argumentosValidos = leerNumeroHilos(argv[i], hilos) && argumentosValidos;
            } else if (argumento == "--estadisticas" && !estadisticas) {
                estadisticas = true;
            } else if (argumento == "--traza" && rutaTraza.empty() && i + 1 < argc) {
                rutaTraza = argv[++i];
            } else {
                argumentosValidos = false;
            }
        }

        if (!argumentosValidos) {
            cerr << "Uso: paralelo [--hilos N] [--estadisticas] [--traza archivo] < entrada.in\n";
            estado = 2;
        } else {
            ecosystem::Mundo mundo = ecosystem::leerMundo(cin);
            // --traza: escribe cada generacion (la 0 incluida) con el formato del output,
            // para reproducirla en visualizacion/index.html.
            ofstream traza;
            if (!rutaTraza.empty()) {
                traza.open(rutaTraza);
                if (!traza) throw runtime_error("No se pudo abrir el archivo de traza.");
            }
            ecosystem::Observador observar;
            if (estadisticas || traza.is_open()) {
                if (estadisticas) cerr << "generacion,conejos,zorros,rocas\n";
                observar = [&](const ecosystem::Mundo& actual, const ecosystem::Poblacion& p) {
                    if (estadisticas)
                        cerr << actual.generacion << ',' << p.conejos << ','
                             << p.zorros << ',' << p.rocas << '\n';
                    if (traza.is_open()) ecosystem::imprimirMundo(actual, traza);
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
