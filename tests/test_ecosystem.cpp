#include "ecosystem.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

int main() {
    try {
        std::istringstream entrada("2 4 3 6 2 3 3\nFOX 1 2\nROCK 0 0\nRABBIT 0 2\n");
        auto mundo = ecosystem::leerMundo(entrada);
        comprobar(mundo.generacion == 0, "Generacion inicial");
        comprobar(mundo.config.numGeneraciones == 6, "Lectura de N_GEN positivo");
        comprobar(mundo.matriz.size() == 2 && mundo.matriz[0].size() == 3, "Forma rectangular");
        comprobar(mundo.matriz[0][1].tipo == ecosystem::Tipo::EMPTY, "Celdas vacias");
        comprobar(mundo.matriz[0][0].tipo == ecosystem::Tipo::ROCK, "Roca");
        comprobar(mundo.matriz[0][2].tipo == ecosystem::Tipo::RABBIT, "Conejo");
        comprobar(mundo.matriz[1][2].tipo == ecosystem::Tipo::FOX, "Zorro");
        for (const auto& fila : mundo.matriz) {
            for (const auto& celda : fila) {
                comprobar(celda.edadReproduccion == 0 && celda.hambre == 0, "Contadores iniciales");
            }
        }
        std::ostringstream salida;
        ecosystem::imprimirMundo(mundo, salida);
        comprobar(salida.str() == "2 4 3 6 2 3 3\nROCK 0 0\nRABBIT 0 2\nFOX 1 2\n", "Orden de salida");
        std::istringstream copia(salida.str());
        auto releido = ecosystem::leerMundo(copia);
        std::ostringstream segunda;
        ecosystem::imprimirMundo(releido, segunda);
        comprobar(salida.str() == segunda.str(), "Ida y vuelta");

        // Comprueba el serializador; no representa una simulacion ejecutada.
        mundo.generacion = 6;
        mundo.matriz[1][2] = ecosystem::Celda{};
        std::ostringstream final;
        ecosystem::imprimirMundo(mundo, final);
        comprobar(final.str() == "2 4 3 0 2 3 2\nROCK 0 0\nRABBIT 0 2\n", "N recalculado y generaciones pendientes");
        std::cout << "OK: interfaz, matriz, contadores y serializacion\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
