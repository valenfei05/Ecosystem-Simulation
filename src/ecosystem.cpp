#include "ecosystem.h"

#include <cstddef>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ecosystem {
namespace {

bool finDeLinea(std::istringstream& linea) {
    linea >> std::ws;
    return linea.eof();
}

Tipo leerTipo(const std::string& nombre) {
    if (nombre == "ROCK") return Tipo::ROCK;
    if (nombre == "RABBIT") return Tipo::RABBIT;
    if (nombre == "FOX") return Tipo::FOX;
    throw std::runtime_error("Objeto desconocido: " + nombre);
}

const char* nombreTipo(Tipo tipo) {
    switch (tipo) {
        case Tipo::ROCK: return "ROCK";
        case Tipo::RABBIT: return "RABBIT";
        case Tipo::FOX: return "FOX";
        case Tipo::EMPTY: break;
    }
    throw std::runtime_error("No se puede escribir este tipo de celda.");
}

}  // namespace

Mundo leerMundo(std::istream& entrada) {
    Mundo mundo;
    auto& c = mundo.config;
    int n = 0;
    std::string texto;
    if (!std::getline(entrada, texto)) {
        throw std::runtime_error("Falta la cabecera de siete enteros.");
    }
    std::istringstream cabecera(texto);
    if (!(cabecera >> c.genProcConejos >> c.genProcZorros >> c.genComidaZorros
                   >> c.numGeneraciones >> c.filas >> c.columnas >> n)
        || !finDeLinea(cabecera)) {
        throw std::runtime_error("La cabecera debe contener exactamente siete enteros de tipo int.");
    }
    if (c.genProcConejos < 0 || c.genProcZorros < 0 || c.genComidaZorros < 0
        || c.numGeneraciones < 0 || n < 0) {
        throw std::runtime_error("Los intervalos, N_GEN y N no pueden ser negativos.");
    }
    if (c.filas <= 0 || c.columnas <= 0) {
        throw std::runtime_error("R y C deben ser mayores que cero.");
    }
    // La conversion antes de multiplicar evita desbordar un int.
    const auto capacidad = static_cast<unsigned long long>(c.filas)
                         * static_cast<unsigned long long>(c.columnas);
    if (static_cast<unsigned long long>(n) > capacidad) {
        throw std::runtime_error("N supera el numero de celdas disponibles.");
    }
    mundo.matriz.assign(static_cast<std::size_t>(c.filas),
                        std::vector<Celda>(static_cast<std::size_t>(c.columnas)));
    for (int i = 0; i < n; ++i) {
        if (!std::getline(entrada, texto)) {
            throw std::runtime_error("Faltan objetos: se esperaban N lineas.");
        }
        std::istringstream linea(texto);
        std::string nombre;
        int x = 0;
        int y = 0;
        if (!(linea >> nombre >> x >> y) || !finDeLinea(linea)) {
            throw std::runtime_error("Objeto " + std::to_string(i + 1)
                                     + ": se esperaba OBJECT X Y.");
        }
        const Tipo tipo = leerTipo(nombre);
        if (x < 0 || x >= c.filas || y < 0 || y >= c.columnas) {
            throw std::runtime_error("Coordenadas fuera de la matriz.");
        }
        auto& celda = mundo.matriz[x][y];
        if (celda.tipo != Tipo::EMPTY) {
            throw std::runtime_error("Dos objetos ocupan la misma celda.");
        }
        celda.tipo = tipo;
    }
    // Se admiten espacios/lineas vacias finales, pero no objetos sobrantes.
    entrada >> std::ws;
    if (!entrada.eof() || entrada.bad()) {
        throw std::runtime_error("Hay datos adicionales o un error de lectura.");
    }
    return mundo;
}

void imprimirMundo(const Mundo& mundo, std::ostream& salida) {
    std::size_t n = 0;
    for (const auto& fila : mundo.matriz) {
        for (const auto& celda : fila) {
            if (celda.tipo != Tipo::EMPTY) ++n;
        }
    }
    const auto& c = mundo.config;
    salida << c.genProcConejos << ' ' << c.genProcZorros << ' '
           << c.genComidaZorros << ' ' << c.numGeneraciones - mundo.generacion
           << ' ' << c.filas << ' ' << c.columnas << ' ' << n << '\n';
    // Orden determinista: primero filas, despues columnas.
    for (int x = 0; x < c.filas; ++x) {
        for (int y = 0; y < c.columnas; ++y) {
            const auto tipo = mundo.matriz[x][y].tipo;
            if (tipo != Tipo::EMPTY) {
                salida << nombreTipo(tipo) << ' ' << x << ' ' << y << '\n';
            }
        }
    }
}

}  // namespace ecosystem
