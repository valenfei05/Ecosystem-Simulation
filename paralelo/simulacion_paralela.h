#pragma once

#include "ecosystem.h"

namespace ecosystem {

// Avanza una generacion, primero conejos, despues zorros.
// Si ocurre un error, el mundo anterior se conserva.
void avanzarGeneracionParalela(Mundo& mundo, int hilos);

// Ejecuta todas las generaciones y permite mostrar estadisticas.
void simularParalelo(Mundo& mundo, int hilos, const Observador& observar = Observador());

}
