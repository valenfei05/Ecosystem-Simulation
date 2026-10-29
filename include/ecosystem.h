#pragma once

#include <iosfwd>
#include <functional>
#include <cstddef>
#include <vector>

namespace ecosystem {

enum class Tipo { EMPTY, ROCK, RABBIT, FOX };

struct Celda {
    Tipo tipo = Tipo::EMPTY;
    int edadReproduccion = 0;
    int hambre = 0;
};

struct Config {
    int genProcConejos = 0;
    int genProcZorros = 0;
    int genComidaZorros = 0;
    int numGeneraciones = 0;
    int filas = 0;
    int columnas = 0;
};

struct Mundo {
    Config config;
    // matriz[X][Y]: X es fila, Y es columna; origen (0, 0).
    std::vector<std::vector<Celda>> matriz;
    int generacion = 0;
};

// Lee un estado completo. Lanza std::runtime_error ante una entrada invalida.
Mundo leerMundo(std::istream& entrada);

// Recibe un Mundo valido (misma forma que config, contadores no negativos).
// Escribe generaciones pendientes y cuenta los objetos desde la matriz.
void imprimirMundo(const Mundo& mundo, std::ostream& salida);

struct Posicion {
    int fila;
    int columna;
};

// Devuelve solo celdas vacias, en orden norte, este, sur, oeste.
std::vector<Posicion> vecinosVacios(const Mundo& mundo, Posicion origen);

// Una fase de movimiento y reproduccion; comprueba la edad al inicio del paso.
// El llamador avanza el reloj. Las crias participan desde la siguiente fase.
void moverConejos(Mundo& mundo);
void moverZorros(Mundo& mundo);

struct Poblacion {
    std::size_t conejos = 0;
    std::size_t zorros = 0;
    std::size_t rocas = 0;
};

Poblacion contarPoblacion(const Mundo& mundo);
// Un paso completo: conejos, zorros, incremento del reloj. No supera N_GEN.
void avanzarGeneracion(Mundo& mundo);
// Se notifica el estado actual y cada generacion completada. El receptor puede
// copiar Mundo para guardar historia; no debe retener referencias a sus celdas.
using Observador = std::function<void(const Mundo&, const Poblacion&)>;
void simular(Mundo& mundo, const Observador& observar = {});

}  // namespace ecosystem
