#include "ecosystem.h"


namespace ecosystem {

std::vector<Posicion> vecinosVacios(const Mundo& mundo, Posicion origen) {
    const Posicion direcciones[] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    std::vector<Posicion> vecinos;
    for (const auto& direccion : direcciones) {
        const int fila = origen.fila + direccion.fila;
        const int columna = origen.columna + direccion.columna;
        if (fila >= 0 && fila < mundo.config.filas
            && columna >= 0 && columna < mundo.config.columnas
            && mundo.matriz[fila][columna].tipo == Tipo::EMPTY) {
            vecinos.push_back({fila, columna});
        }
    }
    return vecinos;
}

}  // namespace ecosystem
