# Ecosystem Simulation

Simulación de un ecosistema con zorros, conejos y rocas sobre una matriz de R filas y C columnas
(proyecto de computación paralela). Incluye tres partes:

| Parte | Carpeta | Contenido |
|---|---|---|
| Núcleo secuencial | `src/`, `include/`, `tests/` | Simulador en C++17 que pasa todos los tests |
| Versión paralela | `paralelo/` | OpenMP: propuestas por celda en paralelo, resolución de conflictos por destino |
| Visualización y estadísticas | `visualizacion/` | Página web: grilla, gráficas, parámetros ajustables, pausa y velocidad |

Además: `scripts/` (generador de mundos, comparación secuencial/paralelo, benchmark).

## Compilar y probar
Requiere `g++` con C++17 y OpenMP, `make` y `python3`.

    make            # build/secuencial y build/paralelo
    make test       # tests de A, tests de B y comparación secuencial vs paralelo
    make clean

## Ejecutar
Entrada por stdin y resultado por stdout, con el formato del enunciado.

    build/secuencial < tests/ejemplo_pdf.in
    build/paralelo --hilos 4 < tests/ejemplo_pdf.in

Opciones comunes: `--estadisticas` (CSV `generacion,conejos,zorros,rocas` por stderr)
y `--traza archivo` (escribe cada generación, empezando en la 0, con el formato del output).
Solo el secuencial acepta `--solo-conejos`; solo el paralelo acepta `--hilos N`.

## Visualizar
1. Genera una traza con el motor que quieras (secuencial o paralelo):

       make traza MOTOR=paralelo ENTRADA=mundo.in      # o: build/paralelo --hilos 4 --traza trace.txt < mundo.in
2. Abre `visualizacion/index.html` en el navegador y usa **Cargar traza del simulador C++**.

También se puede cargar un input directamente, o generar mundos aleatorios con parámetros ajustables
(GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES, N_GEN, tamaño, poblaciones iniciales y semilla).
Controles: play/pausa, paso, reiniciar y velocidad (1 a 60 gen/s). La gráfica de población se exporta a CSV.
`visualizacion/index.html` incluye un simulador de referencia en JavaScript que reproduce exactamente el del C++.

## Informe y resultados
- `informe/Informe_Ecosystem_Simulation.pdf`: informe del proyecto.
- `resultados/`: CSV, gráficas y `resumen.json` de los experimentos. Se regeneran con `python3 scripts/experimentos.py` (requiere `numpy` y `matplotlib`, y `make` previo).
- Antes de entregar, completa en el informe los nombres, la URL del repositorio y la tabla de rendimiento (sección 7).

## Mundos de prueba y medición
    python3 scripts/gen_world.py 60 60 600 80 150 2 3 6 4 200 > mundo.in   # R C conejos zorros rocas semilla gpr gpf gff ngen
    sh scripts/benchmark.sh 1000 1000 100                                    # tiempos secuencial y paralelo

## Reglas de implementación que el enunciado deja abiertas
- En un conflicto se compara la edad de procreación ya reiniciada (quien procrea cuenta como edad 0); en empate de zorros gana el menos hambriento y después el de menor posición.
- Si un animal procrea pero pierde el conflicto en el destino, la cría queda igualmente en la celda de origen.
- Las crías no actúan en la generación en que nacen.

Coordenadas: X es la fila, Y es la columna, con origen en (0, 0).
