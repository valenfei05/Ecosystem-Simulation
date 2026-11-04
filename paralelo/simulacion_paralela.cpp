#include "simulacion_paralela.h"

#include <climits>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

namespace ecosystem {
namespace {

struct Propuesta {
    bool activa = false;
    bool reproduce = false;
    Posicion destino{0, 0};
    Celda animal;
};

// Busca las celdas vecinas que tengan el tipo indicado.
// Revisa norte, este, sur y oeste, en ese orden.
void buscarVecinos(const Mundo& mundo, int x, int y, Tipo buscado,
                   Posicion opciones[4], int& cantidad) {
    const int dx[4] = {-1, 0, 1, 0};
    const int dy[4] = {0, 1, 0, -1};
    cantidad = 0;
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < mundo.config.filas &&
            ny >= 0 && ny < mundo.config.columnas &&
            mundo.matriz[nx][ny].tipo == buscado) {
            opciones[cantidad] = {nx, ny};
            ++cantidad;
        }
    }
}

// Aumenta el contador y avisa si ya llego a su limite.
int incrementar(int valor, int& error) {
    int resultado = valor;
    if (valor < 0 || valor == INT_MAX) {
        error = 1;
    } else {
        resultado = valor + 1;
    }
    return resultado;
}

// Decide si el animal se mueve, come, se reproduce o muere.
void prepararPropuesta(const Mundo& mundo, Tipo especie, int x, int y,
                      Propuesta& propuesta, int& error) {
    Celda animal = mundo.matriz[x][y];
    if (animal.tipo == especie) {
        animal.edadReproduccion = incrementar(animal.edadReproduccion, error);

        Posicion opciones[4];
        int cantidad = 0;
        bool muere = false;
        if (especie == Tipo::RABBIT) {
            buscarVecinos(mundo, x, y, Tipo::EMPTY, opciones, cantidad);
        } else {
            buscarVecinos(mundo, x, y, Tipo::RABBIT, opciones, cantidad);
            if (cantidad == 0) {
                animal.hambre = incrementar(animal.hambre, error);
                if (animal.hambre >= mundo.config.genComidaZorros) {
                    muere = true;
                } else {
                    buscarVecinos(mundo, x, y, Tipo::EMPTY, opciones, cantidad);
                }
            }
        }

        if (!muere && error == 0) {
            propuesta.activa = true;
            propuesta.animal = animal;
            propuesta.destino = {x, y};
            int intervalo = mundo.config.genProcConejos;
            if (especie == Tipo::FOX) {
                intervalo = mundo.config.genProcZorros;
            }
            propuesta.reproduce = cantidad > 0 &&
                                    mundo.matriz[x][y].edadReproduccion >= intervalo;
            if (cantidad > 0) {
                unsigned long long suma = static_cast<unsigned long long>(mundo.generacion)
                                        + static_cast<unsigned long long>(x)
                                        + static_cast<unsigned long long>(y);
                int seleccion = static_cast<int>(suma % static_cast<unsigned long long>(cantidad));
                propuesta.destino = opciones[seleccion];
            }
        }
    }
}

// Elige el animal que gana cuando dos llegan al mismo sitio.
// Compara edad, luego hambre en zorros y por ultimo posicion.
bool preferir(const Propuesta& a, long long origenA,
              const Propuesta& b, long long origenB) {
    bool resultado = false;
    int edadA = a.animal.edadReproduccion;
    int edadB = b.animal.edadReproduccion;
    if (a.reproduce) {
        edadA = 0;
    }
    if (b.reproduce) {
        edadB = 0;
    }

    if (edadA != edadB) {
        resultado = edadA > edadB;
    } else if (a.animal.tipo == Tipo::FOX && a.animal.hambre != b.animal.hambre) {
        resultado = a.animal.hambre < b.animal.hambre;
    } else {
        resultado = origenA < origenB;
    }
    return resultado;
}

// Revisa si un animal quiere llegar a esta celda.
// Puede venir del mismo lugar o de una celda vecina.
void considerarOrigen(const Mundo& mundo, const vector<Propuesta>& propuestas,
                     int ox, int oy, int x, int y, long long& ganador) {
    if (ox >= 0 && ox < mundo.config.filas &&
        oy >= 0 && oy < mundo.config.columnas) {
        long long indice = static_cast<long long>(ox) * mundo.config.columnas + oy;
        const Propuesta& actual = propuestas[static_cast<size_t>(indice)];
        if (actual.activa && actual.destino.fila == x && actual.destino.columna == y) {
            if (ganador < 0 || preferir(actual, indice,
                                     propuestas[static_cast<size_t>(ganador)], ganador)) {
                ganador = indice;
            }
        }
    }
}

// Calcula los movimientos de una especie con varios hilos.
void moverEspecieParalelo(Mundo& mundo, Tipo especie, int hilos) {
    if (mundo.generacion < 0) {
        throw runtime_error("Generacion negativa.");
    }
    int filas = mundo.config.filas;
    int columnas = mundo.config.columnas;
    long long total = static_cast<long long>(filas) * columnas;
    vector<Propuesta> propuestas(static_cast<size_t>(total));
    int error = 0;

    // Paso 1: cada hilo calcula a donde quiere ir un animal.
    // Nadie cambia la matriz mientras se deciden los movimientos.
    #pragma omp parallel for schedule(static) num_threads(hilos) reduction(|:error)
    for (long long indice = 0; indice < total; ++indice) {
        int x = static_cast<int>(indice / columnas);
        int y = static_cast<int>(indice % columnas);
        int errorLocal = 0;
        prepararPropuesta(mundo, especie, x, y,
                         propuestas[static_cast<size_t>(indice)], errorLocal);
        error |= errorLocal;
    }
    if (error != 0) {
        throw runtime_error("Contador de animal fuera de rango.");
    }

    Mundo siguiente;
    siguiente.config = mundo.config;
    siguiente.generacion = mundo.generacion;
    siguiente.matriz.assign(filas, vector<Celda>(columnas));

    // Paso 2: cada hilo prepara una celda de la nueva matriz.
    // Si varios animales llegan, solo deja al ganador.
    #pragma omp parallel for schedule(static) num_threads(hilos)
    for (long long indice = 0; indice < total; ++indice) {
        int x = static_cast<int>(indice / columnas);
        int y = static_cast<int>(indice % columnas);
        Celda celda = mundo.matriz[x][y];
        const Propuesta& propia = propuestas[static_cast<size_t>(indice)];
        if (celda.tipo == especie) {
            celda = Celda{};
            if (propia.activa && propia.reproduce) {
                celda = Celda{especie, 0, 0};
            }
        }

        long long ganador = -1;
        considerarOrigen(mundo, propuestas, x, y, x, y, ganador);
        considerarOrigen(mundo, propuestas, x - 1, y, x, y, ganador);
        considerarOrigen(mundo, propuestas, x, y + 1, x, y, ganador);
        considerarOrigen(mundo, propuestas, x + 1, y, x, y, ganador);
        considerarOrigen(mundo, propuestas, x, y - 1, x, y, ganador);
        if (ganador >= 0) {
            const Propuesta& elegida = propuestas[static_cast<size_t>(ganador)];
            celda = elegida.animal;
            if (elegida.reproduce) {
                celda.edadReproduccion = 0;
            }
            if (especie == Tipo::FOX && mundo.matriz[x][y].tipo == Tipo::RABBIT) {
                celda.hambre = 0;
            }
        }
        siguiente.matriz[x][y] = celda;
    }
    mundo.matriz = move(siguiente.matriz);
}

// Cuenta los animales y las rocas del mundo.
Poblacion contarParalelo(const Mundo& mundo) {
    Poblacion total;
    for (int x = 0; x < mundo.config.filas; ++x) {
        for (int y = 0; y < mundo.config.columnas; ++y) {
            Tipo tipo = mundo.matriz[x][y].tipo;
            if (tipo == Tipo::ROCK) {
                ++total.rocas;
            } else if (tipo == Tipo::RABBIT) {
                ++total.conejos;
            } else if (tipo == Tipo::FOX) {
                ++total.zorros;
            }
        }
    }
    return total;
}

}  // namespace

// Mueve primero los conejos y despues los zorros.
void avanzarGeneracionParalela(Mundo& mundo, int hilos) {
    if (hilos <= 0) {
        throw runtime_error("El numero de hilos debe ser mayor que cero.");
    }
    if (mundo.generacion < 0 || mundo.generacion > mundo.config.numGeneraciones) {
        throw runtime_error("Reloj fuera del intervalo de simulacion.");
    }
    if (mundo.generacion < mundo.config.numGeneraciones) {
        // Guarda la generacion solo si los dos grupos terminan bien.
        Mundo siguiente = mundo;
        moverEspecieParalelo(siguiente, Tipo::RABBIT, hilos);
        moverEspecieParalelo(siguiente, Tipo::FOX, hilos);
        ++siguiente.generacion;
        mundo = move(siguiente);
    }
}

// Ejecuta las generaciones que faltan.
void simularParalelo(Mundo& mundo, int hilos, const Observador& observar) {
    if (hilos <= 0) {
        throw runtime_error("El numero de hilos debe ser mayor que cero.");
    }
    if (mundo.generacion < 0 || mundo.generacion > mundo.config.numGeneraciones) {
        throw runtime_error("Reloj fuera del intervalo de simulacion.");
    }
    if (observar) {
        observar(mundo, contarParalelo(mundo));
    }
    while (mundo.generacion < mundo.config.numGeneraciones) {
        avanzarGeneracionParalela(mundo, hilos);
        if (observar) {
            observar(mundo, contarParalelo(mundo));
        }
    }
}

}  // namespace ecosystem
