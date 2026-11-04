#include "ecosystem.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    try {
        bool soloConejos = false;
        bool estadisticas = false;
        std::string rutaTraza;
        for (int i = 1; i < argc; ++i) {
            const std::string argumento = argv[i];
            if (argumento == "--solo-conejos" && !soloConejos) soloConejos = true;
            else if (argumento == "--estadisticas" && !estadisticas) estadisticas = true;
            else if (argumento == "--traza" && rutaTraza.empty() && i + 1 < argc) rutaTraza = argv[++i];
            else {
                std::cerr << "Uso: secuencial [--solo-conejos] [--estadisticas] [--traza archivo] < entrada.in\n";
                return 2;
            }
        }
        auto mundo = ecosystem::leerMundo(std::cin);
        // --traza: escribe cada generacion (la 0 incluida) con el formato del output,
        // para que la visualizacion (visualizacion/index.html) pueda reproducirla.
        std::ofstream traza;
        if (!rutaTraza.empty()) {
            traza.open(rutaTraza);
            if (!traza) throw std::runtime_error("No se pudo abrir el archivo de traza.");
        }
        ecosystem::Observador observar;
        if (estadisticas || traza.is_open()) {
            if (estadisticas) std::cerr << "generacion,conejos,zorros,rocas\n";
            observar = [&](const ecosystem::Mundo& estado, const ecosystem::Poblacion& p) {
                if (estadisticas)
                    std::cerr << estado.generacion << ',' << p.conejos << ','
                              << p.zorros << ',' << p.rocas << '\n';
                if (traza.is_open()) ecosystem::imprimirMundo(estado, traza);
            };
        }
        if (soloConejos) {
            if (observar) observar(mundo, ecosystem::contarPoblacion(mundo));
            while (mundo.generacion < mundo.config.numGeneraciones) {
                ecosystem::moverConejos(mundo);
                ++mundo.generacion;
                if (observar) observar(mundo, ecosystem::contarPoblacion(mundo));
            }
        } else {
            ecosystem::simular(mundo, observar);
        }
        ecosystem::imprimirMundo(mundo, std::cout);
        if (!std::cout) {
            std::cerr << "Error al escribir la salida.\n";
            return 1;
        }
        if (traza.is_open() && (traza.flush(), !traza)) {
            std::cerr << "Error al escribir la traza.\n";
            return 1;
        }
        return 0;
    } catch (const std::bad_alloc&) {
        std::cerr << "Error: memoria insuficiente para el ecosistema.\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
    }
    return 1;
}
