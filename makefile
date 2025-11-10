# ===============================
#  Makefile para RMQ Experimentos
# ===============================

# Compilador y flags comunes
CXX     = g++
CXXFLAGS = -std=c++11 -O3 -DNDEBUG -I $(HOME)/include
LDFLAGS  = -L $(HOME)/lib
LDLIBS   = -lsdsl -ldivsufsort -ldivsufsort64

# Archivos fuente
SRCS = RMQ-Sparse-Table-Static.cpp \
       RMQ-Sparse-Table-Dinamic.cpp \
       RMQ-Segment-Tree-Static.cpp \
       RMQ-Segment-Tree-Dinamic.cpp

# Ejecutables (mismo nombre sin .cpp)
EXECS = $(SRCS:.cpp=)

# Regla por defecto: compilar todos
all: $(EXECS)

# Regla genérica para compilar cada archivo
%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)
	@echo "Compilado: $@"

# Limpieza
clean:
	rm -f $(EXECS)
	@echo "Ejecutables eliminados."

# Limpieza total (opcional)
distclean: clean
	rm -f *.csv
	@echo "CSVs eliminados también."

