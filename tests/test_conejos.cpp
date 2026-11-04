#include "ecosystem.h"

#include <climits>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace ecosystem;

void comprobar(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

Mundo leer(const std::string& texto) {
    std::istringstream entrada(texto);
    return leerMundo(entrada);
}

bool iguales(const Mundo& a, const Mundo& b) {
    if (a.generacion != b.generacion || a.matriz.size() != b.matriz.size()) return false;
    for (std::size_t x = 0; x < a.matriz.size(); ++x) {
        for (std::size_t y = 0; y < a.matriz[x].size(); ++y) {
            const auto& c = a.matriz[x][y];
            const auto& d = b.matriz[x][y];
            if (c.tipo != d.tipo || c.edadReproduccion != d.edadReproduccion
                || c.hambre != d.hambre) return false;
        }
    }
    return true;
}

int main() {
    try {
        const auto centro = leer("2 3 4 4 3 3 1\nRABBIT 1 1\n");
        const auto vecinos = vecinosVacios(centro, {1, 1});
        const Posicion orden[] = {{0, 1}, {1, 2}, {2, 1}, {1, 0}};
        comprobar(vecinos.size() == 4, "Cuatro vecinos");
        for (int i = 0; i < 4; ++i) {
            comprobar(vecinos[i].fila == orden[i].fila && vecinos[i].columna == orden[i].columna,
                      "Orden norte, este, sur, oeste");
            auto mundo = centro;
            mundo.generacion = i;
            moverConejos(mundo);
            const auto destino = orden[(i + 2) % 4];
            comprobar(mundo.matriz[destino.fila][destino.columna].tipo == Tipo::RABBIT,
                      "Seleccion dependiente de G");
            comprobar(mundo.matriz[destino.fila][destino.columna].edadReproduccion == 1,
                      "Edad aumenta antes de llegar al umbral");
            comprobar(mundo.matriz[1][1].tipo == Tipo::EMPTY && mundo.generacion == i,
                      "Origen vacio y reloj a cargo del llamador");
        }
        auto esquina = leer("2 3 4 1 2 3 1\nRABBIT 0 0\n");
        const auto borde = vecinosVacios(esquina, {0, 0});
        comprobar(borde.size() == 2 && borde[0].fila == 0 && borde[0].columna == 1
                  && borde[1].fila == 1 && borde[1].columna == 0, "Bordes sin envolver");
        moverConejos(esquina);
        comprobar(esquina.matriz[0][1].tipo == Tipo::RABBIT, "Movimiento una sola vez");

        auto encerrado = leer("2 3 4 1 3 3 5\nRABBIT 1 1\nROCK 0 1\nFOX 1 2\nROCK 2 1\nROCK 1 0\n");
        auto original = encerrado;
        original.matriz[1][1].edadReproduccion = 1;
        moverConejos(encerrado);
        comprobar(iguales(encerrado, original), "Sin espacio: no moverse a diagonales");
        auto unico = leer("2 3 4 1 1 1 1\nRABBIT 0 0\n");
        original = unico;
        original.matriz[0][0].edadReproduccion = 1;
        moverConejos(unico);
        comprobar(iguales(unico, original), "Matriz 1x1");
        auto vacio = leer("2 3 4 1 2 3 0\n");
        original = vacio;
        moverConejos(vacio);
        comprobar(iguales(vacio, original), "Mundo vacio");

        // El segundo conejo no puede usar una celda que acaba de desocuparse.
        auto simultaneo = leer("2 3 4 1 1 4 2\nRABBIT 0 1\nRABBIT 0 2\n");
        moverConejos(simultaneo);
        comprobar(simultaneo.matriz[0][0].tipo == Tipo::RABBIT
                  && simultaneo.matriz[0][3].tipo == Tipo::RABBIT
                  && simultaneo.matriz[0][1].tipo == Tipo::EMPTY
                  && simultaneo.matriz[0][2].tipo == Tipo::EMPTY, "Decisiones simultaneas");

        auto filtrado = leer("2 3 4 1 3 3 3\nRABBIT 1 1\nROCK 0 1\nFOX 1 2\n");
        moverConejos(filtrado);
        comprobar(filtrado.matriz[2][1].tipo == Tipo::RABBIT
                  && filtrado.matriz[0][1].tipo == Tipo::ROCK
                  && filtrado.matriz[1][2].tipo == Tipo::FOX, "Numerar solo opciones vacias");

        auto conflicto = leer("2 3 4 1 1 3 2\nRABBIT 0 0\nRABBIT 0 2\n");
        moverConejos(conflicto);
        comprobar(contarPoblacion(conflicto).conejos == 1
                  && conflicto.matriz[0][1].tipo == Tipo::RABBIT, "Conflicto conserva un ganador");

        auto grande = centro;
        grande.config.numGeneraciones = INT_MAX;
        grande.generacion = INT_MAX - 1;
        moverConejos(grande);
        comprobar(grande.matriz[0][1].tipo == Tipo::RABBIT, "Suma sin desbordamiento");
        auto reproduccion = leer("2 3 4 4 1 5 1\nRABBIT 0 0\n");
        moverConejos(reproduccion);
        comprobar(reproduccion.matriz[0][0].tipo == Tipo::EMPTY
                  && reproduccion.matriz[0][1].edadReproduccion == 1, "No nace antes del umbral");
        ++reproduccion.generacion;
        moverConejos(reproduccion);
        comprobar(contarPoblacion(reproduccion).conejos == 1
                  && reproduccion.matriz[0][2].edadReproduccion == 2,
                  "Primero transcurren dos generaciones completas");
        ++reproduccion.generacion;
        moverConejos(reproduccion);
        comprobar(reproduccion.matriz[0][2].tipo == Tipo::RABBIT
                  && reproduccion.matriz[0][3].tipo == Tipo::RABBIT
                  && reproduccion.matriz[0][2].edadReproduccion == 0
                  && reproduccion.matriz[0][3].edadReproduccion == 0,
                  "Cria y adulto reinician edad en el tercer movimiento");
        ++reproduccion.generacion;
        moverConejos(reproduccion);
        comprobar(reproduccion.matriz[0][1].tipo == Tipo::RABBIT
                  && reproduccion.matriz[0][4].tipo == Tipo::RABBIT
                  && reproduccion.matriz[0][1].edadReproduccion == 1
                  && reproduccion.matriz[0][4].edadReproduccion == 1
                  && contarPoblacion(reproduccion).conejos == 2,
                  "Cria actua despues y adulto espera otro intervalo");

        auto espera = leer("2 3 4 4 1 2 2\nRABBIT 0 0\nROCK 0 1\n");
        for (int paso = 0; paso < 3; ++paso) {
            moverConejos(espera);
            ++espera.generacion;
        }
        comprobar(espera.matriz[0][0].edadReproduccion == 3
                  && espera.matriz[0][1].tipo == Tipo::ROCK,
                  "Encerrado acumula edad sin reproducirse");
        espera.matriz[0][1] = Celda{};
        moverConejos(espera);
        comprobar(espera.matriz[0][0].tipo == Tipo::RABBIT
                  && espera.matriz[0][1].tipo == Tipo::RABBIT
                  && espera.matriz[0][0].edadReproduccion == 0
                  && espera.matriz[0][1].edadReproduccion == 0,
                  "Puede reproducirse despues de superar el umbral");

        for (int umbral : {0, 1}) {
            auto inmediato = leer("0 3 4 1 1 3 1\nRABBIT 0 0\n");
            inmediato.config.genProcConejos = umbral;
            inmediato.matriz[0][0].edadReproduccion = umbral;
            moverConejos(inmediato);
            comprobar(inmediato.matriz[0][0].tipo == Tipo::RABBIT
                      && inmediato.matriz[0][1].tipo == Tipo::RABBIT
                      && inmediato.matriz[0][2].tipo == Tipo::EMPTY
                      && inmediato.matriz[0][0].edadReproduccion == 0
                      && inmediato.matriz[0][1].edadReproduccion == 0,
                      "Umbrales cero y uno; un movimiento y un nacimiento por fase");
        }
        conflicto = leer("0 3 4 1 1 3 2\nRABBIT 0 0\nRABBIT 0 2\n");
        moverConejos(conflicto);
        comprobar(contarPoblacion(conflicto).conejos == 3,
                  "Los dos adultos dejan crias; solo uno sobrevive al destino");
        std::cout << "OK: movimiento y reproduccion de conejos\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
