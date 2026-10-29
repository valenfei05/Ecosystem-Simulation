"""Compila C++17 y prueba la interfaz y el ejecutable real (sin dependencias Python)."""
from pathlib import Path
import os
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
BUILD.mkdir(exist_ok=True)
SUFFIX = ".exe" if os.name == "nt" else ""
PROGRAM = BUILD / ("secuencial" + SUFFIX)
UNIT = BUILD / ("test_ecosystem" + SUFFIX)
MOVEMENT = BUILD / ("test_conejos" + SUFFIX)
FULL = BUILD / ("test_simulacion" + SUFFIX)
FLAGS = ["g++", "-std=c++17", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Iinclude"]
for source, target in [("src/secuencial.cpp", PROGRAM), ("tests/test_ecosystem.cpp", UNIT), ("tests/test_conejos.cpp", MOVEMENT), ("tests/test_simulacion.cpp", FULL)]:
    # MinGW puede fallar con rutas absolutas que contienen tildes.
    subprocess.run(FLAGS + ["src/ecosystem.cpp", "src/conejos.cpp", "src/simulacion.cpp", source, "-o", str(target.relative_to(ROOT))], cwd=ROOT, check=True)
subprocess.run([str(UNIT)], cwd=ROOT, check=True)
subprocess.run([str(MOVEMENT)], cwd=ROOT, check=True)
subprocess.run([str(FULL)], cwd=ROOT, check=True)

checks = 0


def check(name, data, code=0, expected=None, args=(), stderr=""):
    global checks
    result = subprocess.run([str(PROGRAM), *args], input=data, text=True, capture_output=True, cwd=ROOT)
    if result.returncode != code:
        raise AssertionError(f"{name}: codigo {result.returncode}, esperado {code}: {result.stderr}")
    if code == 0:
        if result.stdout != expected or result.stderr != stderr:
            raise AssertionError(f"{name}: stdout={result.stdout!r}, stderr={result.stderr!r}")
    elif result.stdout or not result.stderr.strip():
        raise AssertionError(f"{name}: el error debe estar solo en stderr")
    checks += 1
    print(f"OK: {name}")


initial = (ROOT / "tests/initial.in").read_text(encoding="utf-8")
check("N_GEN=0 conserva el estado", initial, expected=initial)
check("objetos desordenados", "2 3 4 0 3 4 3\nFOX 2 3\nROCK 0 0\nRABBIT 1 2\n", expected=initial)
check("mundo vacio 1x1", "2 3 4 0 1 1 0\n", expected="2 3 4 0 1 1 0\n")
check("matriz llena y bordes", "2 3 4 0 1 3 3\nFOX 0 2\nROCK 0 0\nRABBIT 0 1\n",
      expected="2 3 4 0 1 3 3\nROCK 0 0\nRABBIT 0 1\nFOX 0 2\n")
check("intervalos cero", "0 0 0 0 1 1 0", expected="0 0 0 0 1 1 0\n")
check("CRLF, espacios y sin salto final", "2 3 4 0 1 1 1  \r\nROCK 0 0   ", expected="2 3 4 0 1 1 1\nROCK 0 0\n")
check("espacios finales", initial + "\n \t\n", expected=initial)

invalid = {
    "entrada vacia": "",
    "cabecera incompleta": "2 3 4 0 3 4\n",
    "cabecera sobrante": "2 3 4 0 3 4 0 9\n",
    "cabecera no numerica": "a 3 4 0 3 4 0\n",
    "decimal": "2 3 4 0 3 4 0.5\n",
    "entero fuera de rango": "2147483648 3 4 0 1 1 0\n",
    "tipo desconocido": "2 3 4 0 1 1 1\nWOLF 0 0\n",
    "tipo en minusculas": "2 3 4 0 1 1 1\nrock 0 0\n",
    "fila fuera de rango": "2 3 4 0 2 3 1\nFOX 2 0\n",
    "columna fuera de rango": "2 3 4 0 2 3 1\nFOX 0 3\n",
    "coordenada negativa": "2 3 4 0 2 3 1\nFOX -1 0\n",
    "coordenada enorme": "2 3 4 0 2 3 1\nFOX 0 999999999999999\n",
    "coordenada decimal": "2 3 4 0 2 3 1\nFOX 0 1.5\n",
    "ocupacion duplicada": "2 3 4 0 2 3 2\nFOX 0 0\nRABBIT 0 0\n",
    "N mayor que capacidad": "2 3 4 0 1 1 2\n",
    "faltan objetos": "2 3 4 0 2 3 2\nFOX 0 0\n",
    "sobran objetos": "2 3 4 0 2 3 0\nFOX 0 0\n",
    "objeto incompleto": "2 3 4 0 2 3 1\nFOX 0\n",
    "objeto con campos extra": "2 3 4 0 2 3 1\nFOX 0 0 extra\n",
    "linea de objeto vacia": "2 3 4 0 2 3 1\n\n",
}
for index, field in enumerate(["GEN_PROC_RABBITS", "GEN_PROC_FOXES", "GEN_FOOD_FOXES", "N_GEN", "R", "C", "N"]):
    values = [2, 3, 4, 0, 2, 3, 0]
    values[index] = -1
    invalid[field + " negativo"] = " ".join(map(str, values)) + "\n"
for index, field in [(4, "R"), (5, "C")]:
    values = [2, 3, 4, 0, 2, 3, 0]
    values[index] = 0
    invalid[field + " cero"] = " ".join(map(str, values)) + "\n"
for name, data in invalid.items():
    check(name, data, code=1)
check("modo desconocido", initial, code=2, args=("--otro",))
check("cero generaciones en modo conejos", initial, expected=initial, args=("--solo-conejos",))
movement = (ROOT / "tests/conejos.in").read_text(encoding="utf-8")
check("movimiento con obstaculos", movement,
      expected="2 3 4 0 3 4 3\nROCK 0 0\nRABBIT 1 1\nFOX 2 3\n", args=("--solo-conejos",))
check("dos pasos usan generaciones distintas", "9 3 4 2 3 3 1\nRABBIT 1 1\n",
      expected="9 3 4 0 3 3 1\nRABBIT 2 2\n", args=("--solo-conejos",))
check("conflicto resuelto", "2 3 4 1 1 3 2\nRABBIT 0 0\nRABBIT 0 2\n",
      expected="2 3 4 0 1 3 1\nRABBIT 0 1\n", args=("--solo-conejos",))
reproduction = (ROOT / "tests/reproduccion.in").read_text(encoding="utf-8")
check("nacimiento actualiza N y salida", reproduction,
      expected="2 3 4 0 1 5 2\nRABBIT 0 2\nRABBIT 0 3\n", args=("--solo-conejos",))
check("sin espacio no hay nacimiento", "1 3 4 3 1 1 1\nRABBIT 0 0\n",
      expected="1 3 4 0 1 1 1\nRABBIT 0 0\n", args=("--solo-conejos",))
check("crias no actuan al nacer", "0 3 4 1 1 4 1\nRABBIT 0 0\n",
      expected="0 3 4 0 1 4 2\nRABBIT 0 0\nRABBIT 0 1\n", args=("--solo-conejos",))
check("cero generaciones no produce nacimientos", "0 3 4 0 1 3 1\nRABBIT 0 0\n",
      expected="0 3 4 0 1 3 1\nRABBIT 0 0\n", args=("--solo-conejos",))
pdf = (ROOT / "tests/ejemplo_pdf.in").read_text(encoding="utf-8")
final = "2 4 3 0 5 5 5\nROCK 0 0\nROCK 2 4\nRABBIT 3 0\nFOX 4 0\nFOX 4 3\n"
check("ejemplo oficial completo", pdf, expected=final)
check("estadisticas separadas de stdout", pdf, expected=final, args=("--estadisticas",),
      stderr="generacion,conejos,zorros,rocas\n0,3,4,2\n1,3,4,2\n2,1,4,2\n3,2,2,2\n4,2,2,2\n5,1,2,2\n6,1,2,2\n")
check("opcion repetida", initial, code=2, args=("--estadisticas", "--estadisticas"))
check("extincion completa", "2 4 1 1 1 1 1\nFOX 0 0\n", expected="2 4 1 0 1 1 0\n")
print(f"PASS: {checks} casos de consola y pruebas de interfaz C++.")
