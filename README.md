# Ecosystem Simulation

Simulación de un ecosistema con zorros, conejos y rocas sobre una matriz de R filas por C columnas. Los conejos se reproducen y sirven de alimento; los zorros cazan conejos, se reproducen y mueren si pasan demasiado tiempo sin comer. El proyecto incluye una versión secuencial, una versión paralela con OpenMP y una herramienta para visualizar la simulación.

## Contenido

| Carpeta | Descripción |
|---|---|
| `src/`, `include/` | Simulador secuencial en C++17 |
| `paralelo/` | Simulador paralelo con OpenMP |
| `visualizacion/` | Página web para ver la simulación y sus estadísticas |
| `tests/` | Pruebas automáticas y ejemplos de entrada |

## Requisitos

- `g++` con soporte para C++17 y OpenMP
- `make`
- `python3` (solo para ejecutar las pruebas)

## Compilar y probar

```
make          # genera build/secuencial y build/paralelo
make test     # ejecuta las pruebas
make clean    # borra los archivos compilados
```

## Ejecutar

El programa lee el ecosistema por la entrada estándar y escribe el estado final por la salida estándar.

```
build/secuencial < tests/ejemplo_pdf.in
build/paralelo --hilos 4 < tests/ejemplo_pdf.in
```

Ambas versiones producen exactamente el mismo resultado.

### Opciones

| Opción | Versión | Efecto |
|---|---|---|
| `--estadisticas` | ambas | Escribe por la salida de error la población de cada generación (CSV) |
| `--traza archivo` | ambas | Guarda cada generación en un archivo para visualizarla |
| `--hilos N` | paralela | Número de hilos a usar |
| `--solo-conejos` | secuencial | Simula únicamente los conejos |

### Formato de entrada y salida

La primera línea contiene los parámetros y las siguientes describen los objetos:

```
GEN_PROC_RABBITS GEN_PROC_FOXES GEN_FOOD_FOXES N_GEN R C N
OBJETO X Y          (N líneas; OBJETO es ROCK, FOX o RABBIT)
```

`X` es la fila e `Y` la columna, con origen en (0, 0). La salida usa el mismo formato.

## Visualización

1. Genera una traza: `build/paralelo --hilos 4 --traza trace.txt < tests/ejemplo_pdf.in > /dev/null`
2. Abre `visualizacion/index.html` en el navegador.
3. Pulsa **Cargar traza del simulador C++** y elige `trace.txt`.

La página también permite crear mundos aleatorios y cambiar los parámetros. Muestra la cuadrícula, la generación actual, el número de conejos, zorros y rocas, y una gráfica de población que se puede exportar a CSV. Tiene controles para iniciar, pausar, avanzar paso a paso, reiniciar y cambiar la velocidad.

## Decisiones de implementación

Donde el enunciado no es explícito, ambas versiones aplican las mismas reglas:

- En un conflicto, un animal que acaba de procrear cuenta como edad de procreación cero.
- Si un animal procrea pero pierde el conflicto en la celda destino, su cría igualmente queda en la celda de origen.
- Las crías no actúan en la generación en la que nacen.
