#include "ecosystem.h"

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <climits>

using namespace ecosystem;

void exigir(bool valor, const char* mensaje) {
    if (!valor) throw std::runtime_error(mensaje);
}
Mundo leer(const std::string& texto) {
    std::istringstream entrada(texto);
    return leerMundo(entrada);
}
std::string grilla(const Mundo& mundo) {
    std::string s;
    for (const auto& fila : mundo.matriz) {
        for (const auto& c : fila) {
            s += c.tipo == Tipo::ROCK ? '*' : c.tipo == Tipo::RABBIT ? 'R'
               : c.tipo == Tipo::FOX ? 'F' : ' ';
        }
        s += '\n';
    }
    return s;
}

int main() {
    try {
        std::ifstream entrada("tests/ejemplo_pdf.in");
        auto mundo = leerMundo(entrada);
        const std::array<std::string, 7> estados = {
            "* R F\nF   F\n    *\nR    \nR   F\n",
            "*R F \n   F \nF   *\n R  F\n R   \n",
            "* F  \n  F  \n    *\nF    \n  R F\n",
            "*  F \n     \n    *\n     \nFRR  \n",
            "*   F\n     \n    *\n R   \n F R \n",
            "*    \n     \n    *\nR    \n FF  \n",
            "*    \n     \n    *\nR    \nF  F \n"
        };
        int llamadas = 0;
        simular(mundo, [&](const Mundo& estado, const Poblacion& poblacion) {
            exigir(estado.generacion == llamadas, "Observador una vez por generacion");
            if (grilla(estado) != estados.at(llamadas)) {
                std::cerr << "Generacion " << llamadas << "\n" << grilla(estado);
                throw std::runtime_error("Secuencia completa del PDF");
            }
            exigir(poblacion.rocas == 2, "Rocas inmutables");
            ++llamadas;
        });
        exigir(llamadas == 7 && mundo.generacion == 6, "Exactamente N_GEN pasos");
        avanzarGeneracion(mundo);
        exigir(mundo.generacion == 6 && grilla(mundo) == estados[6], "No excede N_GEN");

        auto cero = leer("2 3 4 0 1 1 1\nRABBIT 0 0\n");
        llamadas = 0;
        simular(cero, [&](const Mundo&, const Poblacion& p) {
            ++llamadas;
            exigir(p.conejos == 1 && p.zorros == 0 && p.rocas == 0, "Conteos con N_GEN=0");
        });
        exigir(llamadas == 1 && cero.matriz[0][0].edadReproduccion == 0, "Cero pasos");

        auto caza = leer("9 9 2 1 3 3 2\nFOX 1 1\nRABBIT 1 2\n");
        caza.matriz[1][1].hambre = 1;
        moverZorros(caza);
        exigir(caza.matriz[1][2].tipo == Tipo::FOX && caza.matriz[1][2].hambre == 0
               && contarPoblacion(caza).conejos == 0, "Caza antes de morir y antes de elegir vacios");

        auto hambre = leer("9 9 2 2 1 3 1\nFOX 0 0\n");
        moverZorros(hambre);
        exigir(hambre.matriz[0][1].tipo == Tipo::FOX && hambre.matriz[0][1].hambre == 1,
               "Aumenta hambre al moverse sin comer");
        moverZorros(hambre);
        exigir(contarPoblacion(hambre).zorros == 0, "Muere exactamente al alcanzar hambre maxima");
        auto quieto = leer("9 0 1 1 1 1 1\nFOX 0 0\n");
        moverZorros(quieto);
        exigir(contarPoblacion(quieto).zorros == 0, "Encerrado tambien muere; no se reproduce");
        auto ceroHambre = leer("9 0 0 1 1 2 1\nFOX 0 0\n");
        moverZorros(ceroHambre);
        exigir(contarPoblacion(ceroHambre).zorros == 0, "Limite de hambre cero sin presa");
        ceroHambre = leer("9 0 0 1 1 2 2\nFOX 0 0\nRABBIT 0 1\n");
        moverZorros(ceroHambre);
        exigir(contarPoblacion(ceroHambre).zorros == 2, "Limite cero permite cazar y reproducirse");

        auto cria = leer("9 2 9 1 1 3 1\nFOX 0 0\n");
        cria.matriz[0][0].edadReproduccion = 2;
        cria.matriz[0][0].hambre = 3;
        moverZorros(cria);
        exigir(cria.matriz[0][0].tipo == Tipo::FOX && cria.matriz[0][1].tipo == Tipo::FOX
               && cria.matriz[0][0].edadReproduccion == 0 && cria.matriz[0][1].edadReproduccion == 0
               && cria.matriz[0][0].hambre == 0 && cria.matriz[0][1].hambre == 4
               && cria.matriz[0][2].tipo == Tipo::EMPTY, "Reproduccion no alimenta al padre ni mueve a cria");

        auto choque = leer("9 9 9 1 1 3 2\nFOX 0 0\nFOX 0 2\n");
        choque.matriz[0][0].edadReproduccion = 1;
        choque.matriz[0][2].edadReproduccion = 4;
        choque.matriz[0][2].hambre = 3;
        moverZorros(choque);
        exigir(contarPoblacion(choque).zorros == 1 && choque.matriz[0][1].edadReproduccion == 5
               && choque.matriz[0][1].hambre == 4, "Mayor edad gana aun con mas hambre");
        choque = leer("9 9 9 1 1 3 2\nFOX 0 0\nFOX 0 2\n");
        choque.matriz[0][0].hambre = 3;
        choque.matriz[0][2].hambre = 1;
        moverZorros(choque);
        exigir(choque.matriz[0][1].hambre == 2, "Empate edad favorece menos hambre");

        auto conejos = leer("4 9 9 1 1 3 2\nRABBIT 0 0\nRABBIT 0 2\n");
        conejos.matriz[0][0].edadReproduccion = 8;
        conejos.matriz[0][2].edadReproduccion = 2;
        moverConejos(conejos);
        exigir(contarPoblacion(conejos).conejos == 2
               && conejos.matriz[0][1].edadReproduccion == 0,
               "Prioridad usa edad anterior al reinicio por nacimiento");

        auto simultaneo = leer("9 9 9 1 1 4 2\nFOX 0 1\nFOX 0 2\n");
        moverZorros(simultaneo);
        exigir(grilla(simultaneo) == "F  F\n", "Zorros no usan espacios recien liberados");
        auto orden = leer("9 9 9 1 1 3 2\nRABBIT 0 1\nFOX 0 2\n");
        avanzarGeneracion(orden);
        exigir(grilla(orden) == "RF \n", "Conejos se mueven antes de la caza");

        std::ifstream entradaOtra("tests/ejemplo_pdf.in");
        auto incompleto = leerMundo(entradaOtra);
        avanzarGeneracion(incompleto);
        simular(incompleto);
        exigir(grilla(incompleto) == estados[6], "Reanudar en memoria conserva edades y reloj");

        auto error = leer("9 9 9 1 1 3 2\nRABBIT 0 0\nFOX 0 2\n");
        error.matriz[0][2].edadReproduccion = INT_MAX;
        const auto antes = grilla(error);
        bool fallo = false;
        try { avanzarGeneracion(error); } catch (const std::runtime_error&) { fallo = true; }
        exigir(fallo && error.generacion == 0 && grilla(error) == antes
               && error.matriz[0][0].edadReproduccion == 0, "Error no publica media generacion");
        std::cout << "OK: secuencia PDF, zorros, hambre, conflictos e integracion\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
