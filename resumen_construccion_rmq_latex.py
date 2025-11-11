#!/usr/bin/env python3
import csv
from collections import defaultdict
from statistics import mean, stdev

# Archivos de entrada: modelo -> archivo
FILES = {
    "ST-Static":  "construccion-rmq-sparse-table-static.csv",
    "ST-Dynamic": "construccion-rmq-sparse-table-dinamic.csv",
    "Seg-Static": "construccion-rmq-segment-tree-static.csv",
    "Seg-Dynamic":"construccion-rmq-segment-tree-dinamic.csv",
}

def leer_datos_construccion(filename):
    """
    Lee un archivo construccion-rmq-*.csv con formato:
        size,rmq_mb,build_ns
    y devuelve:
        dict[size] -> lista de (rmq_mb, build_ns)
    """
    datos = defaultdict(list)
    with open(filename, newline="") as f:
        reader = csv.reader(f)
        header = next(reader, None)  # saltar cabecera
        for row in reader:
            if len(row) < 3:
                continue
            try:
                size = int(row[0])
                rmq_mb = float(row[1])
                build_ns = float(row[2])
            except ValueError:
                continue
            datos[size].append((rmq_mb, build_ns))
    return datos

def resumen_por_modelo_y_size():
    """
    Devuelve una estructura:
      resumen[modelo][size] = (mem_mean, time_mean, time_std)
    """
    resumen = {}
    for modelo, filename in FILES.items():
        datos = leer_datos_construccion(filename)
        resumen[modelo] = {}
        for size, muestras in datos.items():
            mem_vals   = [m[0] for m in muestras]
            time_vals  = [m[1] for m in muestras]
            mem_mean   = mean(mem_vals)
            time_mean  = mean(time_vals)
            time_std   = stdev(time_vals) if len(time_vals) > 1 else 0.0
            resumen[modelo][size] = (mem_mean, time_mean, time_std)
    return resumen

def todas_las_sizes(resumen):
    """
    Devuelve el conjunto ordenado de todos los tamaños que aparecen
    en algún modelo.
    """
    sizes = set()
    for modelo in resumen:
        sizes.update(resumen[modelo].keys())
    return sorted(sizes)

def tabla_memoria_latex(resumen):
    """
    Genera una tabla LaTeX donde:
      - filas: tamaño del arreglo
      - columnas: memoria promedio (MB) de cada modelo
    """
    modelos = ["ST-Static", "ST-Dynamic", "Seg-Static", "Seg-Dynamic"]
    sizes = todas_las_sizes(resumen)

    lines = []
    lines.append(r"\begin{table}[H]")
    lines.append(r"\centering")
    lines.append(r"\caption{Memoria promedio utilizada por cada estructura RMQ (en MB).}")
    lines.append(r"\renewcommand{\arraystretch}{1.2}")
    lines.append(r"\setlength{\tabcolsep}{6pt}")
    lines.append(r"\begin{tabular}{rcccc}")
    lines.append(r"\toprule")
    lines.append(r"\textbf{Tamaño} & \textbf{ST-Static} & \textbf{ST-Dynamic} & \textbf{Seg-Static} & \textbf{Seg-Dynamic}\\")
    lines.append(r"\midrule")

    for size in sizes:
        row = [str(size)]
        for modelo in modelos:
            if size in resumen.get(modelo, {}):
                mem_mean = resumen[modelo][size][0]
                row.append(f"{mem_mean:.4f}")
            else:
                row.append("--")
        lines.append(" & ".join(row) + r"\\")
    lines.append(r"\bottomrule")
    lines.append(r"\end{tabular}")
    lines.append(r"\end{table}")
    return "\n".join(lines)

def tabla_tiempos_latex(resumen):
    """
    Genera una tabla LaTeX donde:
      - filas: tamaño del arreglo
      - por cada modelo: 2 columnas (promedio, desviación estándar)
    """
    modelos = ["ST-Static", "ST-Dynamic", "Seg-Static", "Seg-Dynamic"]
    sizes = todas_las_sizes(resumen)

    lines = []
    lines.append(r"\begin{table}[H]")
    lines.append(r"\centering")
    lines.append(r"\caption{Tiempo de construcción promedio y desviación estándar (ns) para cada estructura RMQ.}")
    lines.append(r"\renewcommand{\arraystretch}{1.2}")
    lines.append(r"\setlength{\tabcolsep}{4pt}")
    # 1 columna para tamaño + 2 por modelo = 1 + 8 = 9 columnas
    lines.append(r"\begin{tabular}{rcccccccc}")
    lines.append(r"\toprule")
    header1 = [r"\multirow{2}{*}{\textbf{Tamaño}}"]
    for modelo in modelos:
        header1.append(rf"\multicolumn{{2}}{{c}}{{\textbf{{{modelo}}}}}")
    lines.append(" & ".join(header1) + r"\\")
    lines.append(r"\cmidrule(lr){2-3}\cmidrule(lr){4-5}\cmidrule(lr){6-7}\cmidrule(lr){8-9}")
    header2 = [""]
    for _ in modelos:
        header2.append(r"\textbf{Prom.}")
        header2.append(r"\textbf{Desv.}")
    lines.append(" & ".join(header2) + r"\\")
    lines.append(r"\midrule")

    for size in sizes:
        row = [str(size)]
        for modelo in modelos:
            if size in resumen.get(modelo, {}):
                _, time_mean, time_std = resumen[modelo][size]
                row.append(f"{time_mean:.2f}")
                row.append(f"{time_std:.2f}")
            else:
                row.append("--")
                row.append("--")
        lines.append(" & ".join(row) + r"\\")
    lines.append(r"\bottomrule")
    lines.append(r"\end{tabular}")
    lines.append(r"\end{table}")
    return "\n".join(lines)

def main():
    resumen = resumen_por_modelo_y_size()

    print(r"% Paquetes recomendados en el preámbulo:")
    print(r"% \usepackage{booktabs}")
    print(r"% \usepackage{float}")
    print(r"% \usepackage{multirow}")
    print()
    print("% ===== Tabla de memoria =====")
    print(tabla_memoria_latex(resumen))
    print()
    print("% ===== Tabla de tiempos =====")
    print(tabla_tiempos_latex(resumen))

if __name__ == "__main__":
    main()

