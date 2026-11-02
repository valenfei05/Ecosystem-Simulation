#include "ecosystem.h"
#include "simulacion_paralela.h"

#include <climits>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace ecosystem;

// Detiene la prueba si una condicion no se cumple.
void exigir(bool condicion, const string& mensaje) {
    if (!condicion) {
        throw runtime_error(mensaje);
    }
}

Mundo leer(const string& texto) {
    istringstream entrada(texto);
    Mundo resultado = leerMundo(entrada);
    return resultado;
}

// Compara tipos, edad y hambre en todas las celdas.
bool mundosIguales(const Mundo& a, const Mundo& b) {
    bool iguales = a.generacion == b.generacion &&
                   a.config.filas == b.config.filas &&
                   a.config.columnas == b.config.columnas;
    if (iguales) {
        for (int x = 0; x < a.config.filas; ++x) {
            for (int y = 0; y < a.config.columnas; ++y) {
                const Celda& ca = a.matriz[x][y];
                const Celda& cb = b.matriz[x][y];
                if (ca.tipo != cb.tipo ||
                    ca.edadReproduccion != cb.edadReproduccion ||
                    ca.hambre != cb.hambre) {
                    iguales = false;
                }
            }
        }
    }
    return iguales;
}

// Comprueba que ambas versiones coincidan en cada generacion.
void compararGeneraciones(const Mundo& inicial, const string& descripcion, int hilos) {
    Mundo referencia = inicial;
    Mundo paralelo = inicial;
    for (int paso = 0; paso <= inicial.config.numGeneraciones; ++paso) {
        if (!mundosIguales(referencia, paralelo)) {
            throw runtime_error(descripcion + ": estados diferentes con " +
                                to_string(hilos) + " hilos en generacion " + to_string(paso));
        }
        if (paso < inicial.config.numGeneraciones) {
            avanzarGeneracion(referencia);
            avanzarGeneracionParalela(paralelo, hilos);
        }
    }
    ostringstream refSalida;
    ostringstream parSalida;
    imprimirMundo(referencia, refSalida);
    imprimirMundo(paralelo, parSalida);
    exigir(refSalida.str() == parSalida.str(), descripcion + ": stdout diferente");
}

void compararHilos(const Mundo& mundo, const string& descripcion) {
    compararGeneraciones(mundo, descripcion, 1);
    compararGeneraciones(mundo, descripcion, 2);
    compararGeneraciones(mundo, descripcion, 4);
}

// Crea un mundo de prueba con posiciones variadas.
Mundo mundoAleatorio(mt19937& generador, int filas, int columnas, int generaciones) {
    Mundo mundo;
    mundo.config.filas = filas;
    mundo.config.columnas = columnas;
    mundo.config.numGeneraciones = generaciones;
    mundo.config.genProcConejos = static_cast<int>(generador() % 5);
    mundo.config.genProcZorros = static_cast<int>(generador() % 6);
    mundo.config.genComidaZorros = static_cast<int>(generador() % 6);
    mundo.matriz.assign(filas, vector<Celda>(columnas));
    for (int x = 0; x < filas; ++x) {
        for (int y = 0; y < columnas; ++y) {
            int aleatorio = static_cast<int>(generador() % 100);
            Celda& c = mundo.matriz[x][y];
            if (aleatorio < 11) {
                c.tipo = Tipo::ROCK;
            } else if (aleatorio < 43) {
                c.tipo = Tipo::RABBIT;
                c.edadReproduccion = static_cast<int>(generador() % 5);
            } else if (aleatorio < 63) {
                c.tipo = Tipo::FOX;
                c.edadReproduccion = static_cast<int>(generador() % 6);
                c.hambre = static_cast<int>(generador() % 6);
            }
        }
    }
    return mundo;
}

// Ejecuta todas las pruebas de la version paralela.
int main() {
    int codigo = 0;
    try {
        ifstream archivo("tests/ejemplo_pdf.in");
        exigir(static_cast<bool>(archivo), "No existe tests/ejemplo_pdf.in");
        compararHilos(leerMundo(archivo), "Ejemplo oficial del PDF");
        compararHilos(leer("2 3 4 0 1 1 1\nRABBIT 0 0\n"), "Cero generaciones");
        compararHilos(leer("2 3 4 5 1 1 1\nFOX 0 0\n"), "Zorro sin alimento");
        compararHilos(leer("1 3 4 4 1 5 2\nRABBIT 0 0\nRABBIT 0 4\n"), "Reproduccion");
        compararHilos(leer("2 3 4 3 1 3 2\nRABBIT 0 0\nRABBIT 0 2\n"), "Colision de conejos");
        compararHilos(leer("9 9 2 3 3 3 2\nFOX 1 1\nRABBIT 1 2\n"), "Caza");
        compararHilos(leer("0 0 0 3 1 2 2\nFOX 0 0\nRABBIT 0 1\n"), "Intervalos cero");

        mt19937 generador(20261102);
        for (int caso = 0; caso < 32; ++caso) {
            int filas = 1 + static_cast<int>(generador() % 12);
            int columnas = 1 + static_cast<int>(generador() % 15);
            Mundo mundo = mundoAleatorio(generador, filas, columnas, 12);
            compararHilos(mundo, "Aleatorio " + to_string(caso));
        }
        compararHilos(mundoAleatorio(generador, 70, 90, 8), "Matriz grande");

        Mundo error = leer("2 4 3 3 1 2 1\nFOX 0 0\n");
        error.matriz[0][0].edadReproduccion = INT_MAX;
        Mundo original = error;
        bool fallo = false;
        try {
            avanzarGeneracionParalela(error, 4);
        } catch (const runtime_error&) {
            fallo = true;
        }
        exigir(fallo && mundosIguales(error, original),
               "El error no debe publicar una generacion incompleta");

        int observaciones = 0;
        Mundo estadisticas = leer("2 4 3 3 2 2 2\nFOX 0 0\nRABBIT 1 1\n");
        simularParalelo(estadisticas, 2, [&](const Mundo& actual, const Poblacion& p) {
            exigir(actual.generacion == observaciones, "Orden del observador");
            exigir(p.conejos + p.zorros + p.rocas <= 4, "Conteo fuera de capacidad");
            ++observaciones;
        });
        exigir(observaciones == 4, "Se notifica el estado inicial y todos los pasos");

        bool hiloInvalido = false;
        try {
            avanzarGeneracionParalela(estadisticas, 0);
        } catch (const runtime_error&) {
            hiloInvalido = true;
        }
        exigir(hiloInvalido, "No se permiten cero hilos");
        cout << "OK: ejemplo PDF, casos limite, 32 ecosistemas aleatorios, "
             << "matriz 70x90, estados internos, estadisticas y errores (1/2/4 hilos).\n";
    } catch (const exception& error) {
        cerr << "Fallo de prueba: " << error.what() << '\n';
        codigo = 1;
    }
    return codigo;
}
