#!/usr/bin/env bash
set -euo pipefail

# ==========================
# 1) Limpiar CSVs y cabeceras
# ==========================

echo "Inicializando CSVs..."

# Static: Sparse Table
rm -f construccion-rmq-sparse-table-static.csv
rm -f consultas-rmq-sparse-table-static.csv

echo "size,rmq_mb,build_ns"  > construccion-rmq-sparse-table-static.csv
echo "size,range,query_ns"   > consultas-rmq-sparse-table-static.csv

# Static: Segment Tree
rm -f construccion-rmq-segment-tree-static.csv
rm -f consultas-rmq-segment-tree-static.csv

echo "size,rmq_mb,build_ns"  > construccion-rmq-segment-tree-static.csv
echo "size,range,query_ns"   > consultas-rmq-segment-tree-static.csv

# Dynamic: Sparse Table
rm -f construccion-rmq-sparse-table-dinamic.csv
rm -f consultas-rmq-sparse-table.csv
rm -f update-rmq-sparse-table-dinamic.csv

echo "size,rmq_mb,build_ns"  > construccion-rmq-sparse-table-dinamic.csv
echo "size,range,query_ns"   > consultas-rmq-sparse-table.csv
echo "size,index,value,update_ns" > update-rmq-sparse-table-dinamic.csv

# Dynamic: Segment Tree
rm -f construccion-rmq-segment-tree-dinamic.csv
rm -f consultas-rmq-segment-tree.csv
rm -f update-rmq-segment-tree-dinamic.csv

echo "size,rmq_mb,build_ns"  > construccion-rmq-segment-tree-dinamic.csv
echo "size,range,query_ns"   > consultas-rmq-segment-tree.csv
echo "size,index,value,update_ns" > update-rmq-segment-tree-dinamic.csv

echo "CSV listos."
echo

# ==========================
# 2) Experimentos estáticos
# ==========================

echo "Ejecutando experimentos ESTÁTICOS..."

STATIC_BINARIES=("RMQ-Sparse-Table-Static" "RMQ-Segment-Tree-Static")
SIZES=(1000 2000 3000 4000 5000)
REPS=30

for bin in "${STATIC_BINARIES[@]}"; do
    if [[ ! -x "./$bin" ]]; then
        echo "⚠️  Advertencia: ejecutable ./$bin no existe o no es ejecutable."
        continue
    fi

    for n in "${SIZES[@]}"; do
        dataset="dataset_${n}.txt"
        cmds="comandos_static_${n}.txt"

        if [[ ! -f "$dataset" ]]; then
            echo "⚠️  Dataset $dataset no encontrado, se omite."
            continue
        fi
        if [[ ! -f "$cmds" ]]; then
            echo "⚠️  Archivo de comandos $cmds no encontrado, se omite."
            continue
        fi

        echo "==> [STATIC] $bin con n=$n (30 repeticiones)..."
        for ((rep=1; rep<=REPS; rep++)); do
            ./"$bin" "$dataset" < "$cmds" > /dev/null
        done
    done
done

echo "Experimentos estáticos completados."
echo

# ==========================
# 3) Experimentos dinámicos
# ==========================

echo "Ejecutando experimentos DINÁMICOS..."

DYNAMIC_BINARIES=("RMQ-Sparse-Table-Dinamic" "RMQ-Segment-Tree-Dinamic")

for bin in "${DYNAMIC_BINARIES[@]}"; do
    if [[ ! -x "./$bin" ]]; then
        echo "⚠️  Advertencia: ejecutable ./$bin no existe o no es ejecutable."
        continue
    fi

    for n in "${SIZES[@]}"; do
        dataset="dataset_${n}.txt"
        cmds="comandos_${n}.txt"

        if [[ ! -f "$dataset" ]]; then
            echo "⚠️  Dataset $dataset no encontrado, se omite."
            continue
        fi
        if [[ ! -f "$cmds" ]]; then
            echo "⚠️  Archivo de comandos $cmds no encontrado, se omite."
            continue
        fi

        echo "==> [DINAMIC] $bin con n=$n (30 repeticiones)..."
        for ((rep=1; rep<=REPS; rep++)); do
            ./"$bin" "$dataset" < "$cmds" > /dev/null
        done
    done
done

echo
echo "✅ Todos los experimentos han terminado."
