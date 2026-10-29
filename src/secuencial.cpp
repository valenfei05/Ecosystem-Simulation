#include "ecosystem.h"

#include <exception>
#include <iostream>
#include <new>
#include <string>

int main(int argc, char* argv[]) {
    try {
        bool soloConejos = false;
        bool estadisticas = false;
        for (int i = 1; i < argc; ++i) {
            const std::string argumento = argv[i];
            if (argumento == "--solo-conejos" && !soloConejos) soloConejos = true;
            else if (argumento == "--estadisticas" && !estadisticas) estadisticas = true;
            else {
                std::cerr << "Uso: secuencial [--solo-conejos] [--estadisticas] < entrada.in\n";
                return 2;
            }
        }
        auto mundo = ecosystem::leerMundo(std::cin);
        ecosystem::Observador observar;
        if (estadisticas) {
            std::cerr << "generacion,conejos,zorros,rocas\n";
            observar = [](const ecosystem::Mundo& estado, const ecosystem::Poblacion& p) {
                std::cerr << estado.generacion << ',' << p.conejos << ','
                          << p.zorros << ',' << p.rocas << '\n';
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
        return 0;
    } catch (const std::bad_alloc&) {
        std::cerr << "Error: memoria insuficiente para el ecosistema.\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
    }
    return 1;
}
