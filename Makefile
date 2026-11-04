CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude
COMMON = src/ecosystem.cpp src/conejos.cpp src/simulacion.cpp
PAR = paralelo/simulacion_paralela.cpp paralelo/paralelo.cpp

all: build/secuencial build/paralelo

build:
	mkdir -p build

build/secuencial: $(COMMON) src/secuencial.cpp include/ecosystem.h | build
	$(CXX) $(CXXFLAGS) $(COMMON) src/secuencial.cpp -o $@

build/paralelo: $(COMMON) $(PAR) paralelo/simulacion_paralela.h include/ecosystem.h | build
	$(CXX) $(CXXFLAGS) -fopenmp -Iparalelo $(COMMON) $(PAR) -o $@

build/test_paralelo: $(COMMON) paralelo/simulacion_paralela.cpp paralelo/tests/test_paralelo.cpp | build
	$(CXX) $(CXXFLAGS) -fopenmp -Iparalelo $(COMMON) paralelo/simulacion_paralela.cpp paralelo/tests/test_paralelo.cpp -o $@

test: all build/test_paralelo
	python3 tests/run_tests.py
	build/test_paralelo
	sh scripts/comparar.sh

MOTOR ?= secuencial
ENTRADA ?= tests/ejemplo_pdf.in

# Elige el motor (secuencial o paralelo) y genera trace.txt para visualizacion/index.html
traza: all
	build/$(MOTOR) --traza trace.txt < $(ENTRADA) > /dev/null
	@echo "Traza escrita en trace.txt: cargala en visualizacion/index.html"

clean:
	rm -rf build

.PHONY: all test traza clean
