#include "ecosystem.h"

#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace ecosystem {
namespace {

int incrementar(int valor) {
    if (valor < 0 || valor == std::numeric_limits<int>::max()) {
        throw std::runtime_error("Contador de animal fuera de rango.");
    }
    return valor + 1;
}

std::vector<Posicion> vecinosConTipo(const Mundo& mundo, Posicion origen, Tipo tipo) {
    const Posicion direcciones[] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    std::vector<Posicion> resultado;
    for (const auto& d : direcciones) {
        const int x = origen.fila + d.fila;
        const int y = origen.columna + d.columna;
        if (x >= 0 && x < mundo.config.filas && y >= 0 && y < mundo.config.columnas
            && mundo.matriz[x][y].tipo == tipo) resultado.push_back({x, y});
    }
    return resultado;
}

struct Propuesta {
    Posicion origen;
    Posicion destino;
    Celda animal;
    bool reproduce;
};

// Se compara la edad antes de reiniciarla por reproduccion y el hambre antes
// de comer. El PDF no fija el empate total: se elige el menor origen (fila,col).
bool preferir(const Propuesta& a, const Propuesta& b) {
    if (a.animal.edadReproduccion != b.animal.edadReproduccion)
        return a.animal.edadReproduccion > b.animal.edadReproduccion;
    if (a.animal.tipo == Tipo::FOX && a.animal.hambre != b.animal.hambre)
        return a.animal.hambre < b.animal.hambre;
    return std::tie(a.origen.fila, a.origen.columna)
         < std::tie(b.origen.fila, b.origen.columna);
}

void moverEspecie(Mundo& mundo, Tipo especie) {
    if (mundo.generacion < 0) throw std::runtime_error("Generacion negativa.");
    std::vector<Propuesta> propuestas;
    auto siguiente = mundo.matriz;
    const int intervalo = especie == Tipo::RABBIT
                        ? mundo.config.genProcConejos : mundo.config.genProcZorros;
    // Primera fase: decisiones independientes sobre una instantanea inmutable.
    for (int x = 0; x < mundo.config.filas; ++x) {
        for (int y = 0; y < mundo.config.columnas; ++y) {
            auto animal = mundo.matriz[x][y];
            if (animal.tipo != especie) continue;
            siguiente[x][y] = Celda{};
            animal.edadReproduccion = incrementar(animal.edadReproduccion);
            auto opciones = especie == Tipo::FOX
                          ? vecinosConTipo(mundo, {x, y}, Tipo::RABBIT)
                          : vecinosVacios(mundo, {x, y});
            if (especie == Tipo::FOX && opciones.empty()) {
                animal.hambre = incrementar(animal.hambre);
                // Muere antes de intentar ocupar un espacio vacio o reproducirse.
                if (animal.hambre >= mundo.config.genComidaZorros) continue;
                opciones = vecinosVacios(mundo, {x, y});
            }
            Posicion destino{x, y};
            if (!opciones.empty()) {
                const auto indice = (static_cast<unsigned long long>(mundo.generacion)
                                     + x + y) % opciones.size();
                destino = opciones[indice];
            }
            propuestas.push_back({{x, y}, destino, animal,
                !opciones.empty() && mundo.matriz[x][y].edadReproduccion >= intervalo});
        }
    }
    // Segunda fase: elegir un ganador por destino, sin depender del orden.
    const auto ninguno = std::numeric_limits<std::size_t>::max();
    std::vector<std::vector<std::size_t>> ganadores(
        mundo.config.filas, std::vector<std::size_t>(mundo.config.columnas, ninguno));
    for (std::size_t i = 0; i < propuestas.size(); ++i) {
        const auto& p = propuestas[i];
        auto& ganador = ganadores[p.destino.fila][p.destino.columna];
        if (ganador == ninguno || preferir(p, propuestas[ganador])) ganador = i;
        // Cada movimiento elegible deja una cria antes de resolver las llegadas;
        // la muerte del adulto en destino no elimina la cria que dejo en origen.
        if (p.reproduce) siguiente[p.origen.fila][p.origen.columna] = Celda{especie, 0, 0};
    }
    for (std::size_t i = 0; i < propuestas.size(); ++i) {
        const auto& p = propuestas[i];
        if (ganadores[p.destino.fila][p.destino.columna] != i) continue;
        auto animal = p.animal;
        if (p.reproduce) animal.edadReproduccion = 0;
        if (especie == Tipo::FOX
            && mundo.matriz[p.destino.fila][p.destino.columna].tipo == Tipo::RABBIT)
            animal.hambre = 0;
        siguiente[p.destino.fila][p.destino.columna] = animal;
    }
    mundo.matriz = std::move(siguiente);
}

}  // namespace

void moverConejos(Mundo& mundo) { moverEspecie(mundo, Tipo::RABBIT); }
void moverZorros(Mundo& mundo) { moverEspecie(mundo, Tipo::FOX); }

Poblacion contarPoblacion(const Mundo& mundo) {
    Poblacion poblacion;
    for (const auto& fila : mundo.matriz) {
        for (const auto& celda : fila) {
            if (celda.tipo == Tipo::RABBIT) ++poblacion.conejos;
            else if (celda.tipo == Tipo::FOX) ++poblacion.zorros;
            else if (celda.tipo == Tipo::ROCK) ++poblacion.rocas;
        }
    }
    return poblacion;
}

void avanzarGeneracion(Mundo& mundo) {
    if (mundo.generacion < 0 || mundo.generacion > mundo.config.numGeneraciones)
        throw std::runtime_error("Reloj fuera del intervalo de simulacion.");
    if (mundo.generacion == mundo.config.numGeneraciones) return;
    // Publicar solo una generacion completa, incluso si una fase lanza un error.
    auto siguiente = mundo;
    moverConejos(siguiente);
    moverZorros(siguiente);
    ++siguiente.generacion;
    mundo = std::move(siguiente);
}

void simular(Mundo& mundo, const Observador& observar) {
    if (mundo.generacion < 0 || mundo.generacion > mundo.config.numGeneraciones)
        throw std::runtime_error("Reloj fuera del intervalo de simulacion.");
    if (observar) observar(mundo, contarPoblacion(mundo));
    while (mundo.generacion < mundo.config.numGeneraciones) {
        avanzarGeneracion(mundo);
        if (observar) observar(mundo, contarPoblacion(mundo));
    }
}

}  // namespace ecosystem
