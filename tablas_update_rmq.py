#!/usr/bin/env python3
import csv
from collections import defaultdict
from statistics import mean, stdev

# Archivos de entrada: solo modelos dinámicos
FILES = {
    "ST-Dynamic":  "update-rmq-sparse-table-dinamic.csv",
    "Seg-Dynamic": "update-rmq-segment-tree-dinamic.csv",
}

# ---------- Lectura y resumen de datos ----------
def leer_updates(filename):
    """
    Lee archivo update-rmq-*.csv con formato:
        size_array,index,value,time_ns
    Devuelve dict[size_array] -> lista de tiempos (ns)
    (index y value son ignorados)
    """
    datos = defaultdict(list)
    with open(filename, newline="") as f:
        reader = csv.reader(f)
        header = next(reader, None)  # cabecera (se ignora)
        for row in reader:
            if len(row) < 4:
                continue
            try:
                size_arr = int(row[0])
                time_ns  = float(row[3])
            except ValueError:
                continue
            datos[size_arr].append(time_ns)
    return datos

def resumen_por_modelo_y_size():
    """
    Devuelve:
      resumen[modelo][size] = (mean_time, std_time)
    """
    resumen = {}
    for modelo, filename in FILES.items():
        try:
            datos = leer_updates(filename)
        except FileNotFoundError:
            print(f"% ⚠ Archivo no encontrado: {filename}")
            continue

        modelo_res = {}
        for size, tiempos in datos.items():
            if not tiempos:
                continue
            t_mean = mean(tiempos)
            t_std  = stdev(tiempos) if len(tiempos) > 1 else 0.0
            modelo_res[size] = (t_mean, t_std)
        resumen[modelo] = modelo_res
    return resumen

def todas_las_sizes(resumen):
    sizes = set()
    for modelo in resumen:
        sizes.update(resumen[modelo].keys())
    return sorted(sizes)

# ---------- Tabla en LaTeX ----------
def tabla_updates_latex(resumen):
    modelos = ["ST-Dynamic", "Seg-Dynamic"]
    sizes = todas_las_sizes(resumen)

    lines = []
    lines.append(r"% Paquetes requeridos: booktabs, float, multirow")
    lines.append(r"\begin{table}[H]")
    lines.append(r"\centering")
    lines.append(r"\caption{Tiempo promedio y desviación estándar (ns) de las operaciones de actualización.}")
    lines.append(r"\renewcommand{\arraystretch}{1.2}")
    lines.append(r"\setlength{\tabcolsep}{4pt}")
    # 1 columna tamaño + 2 modelos * 2 columnas = 5 columnas
    lines.append(r"\begin{tabular}{rcccc}")
    lines.append(r"\toprule")
    header1 = [r"\multirow{2}{*}{\textbf{Tamaño}}"]
    for modelo in modelos:
        header1.append(rf"\multicolumn{{2}}{{c}}{{\textbf{{{modelo}}}}}")
    lines.append(" & ".join(header1) + r"\\")
    lines.append(r"\cmidrule(lr){2-3}\cmidrule(lr){4-5}")
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
                t_mean, t_std = resumen[modelo][size]
                row.append(f"{t_mean:.2f}")
                row.append(f"{t_std:.2f}")
            else:
                row.append("--")
                row.append("--")
        lines.append(" & ".join(row) + r"\\")
    lines.append(r"\bottomrule")
    lines.append(r"\end{tabular}")
    lines.append(r"\end{table}")
    return "\n".join(lines)

# ---------- Gráfico en LaTeX con pgfplots (sin desviación estándar) ----------
def grafico_updates_pgfplots(resumen):
    modelos = ["ST-Dynamic", "Seg-Dynamic"]
    colores = {
        "ST-Dynamic": "cyan!70!black",
        "Seg-Dynamic": "red!80!black",
    }
    marcadores = {
        "ST-Dynamic": "square*",
        "Seg-Dynamic": "diamond*",
    }

    lines = []
    lines.append(r"% Paquetes requeridos: pgfplots, xcolor")
    lines.append(r"\begin{figure}[H]")
    lines.append(r"\centering")
    lines.append(r"\begin{tikzpicture}")
    lines.append(r"\begin{axis}[")
    lines.append(r"  width=\textwidth,")
    lines.append(r"  height=7cm,")
    lines.append(r"  xlabel={Tamaño del arreglo ($n$)},")
    lines.append(r"  ylabel={Tiempo promedio de actualización (ns)},")
    lines.append(r"  grid=major,")
    lines.append(r"  legend style={at={(0.5,-0.15)},anchor=north,legend columns=2},")
    lines.append(r"  yticklabel style={/pgf/number format/fixed},")
    lines.append(r"  enlargelimits=0.05")
    lines.append(r"]")

    for modelo in modelos:
        if modelo not in resumen or not resumen[modelo]:
            continue
        color = colores[modelo]
        marker = marcadores[modelo]
        lines.append(rf"\addplot+[mark={marker}, color={color}] coordinates {{")
        for size, (mean_t, _) in sorted(resumen[modelo].items()):
            lines.append(f"({size},{mean_t})")
        lines.append("};")
        lines.append(rf"\addlegendentry{{{modelo}}}")

    lines.append(r"\end{axis}")
    lines.append(r"\end{tikzpicture}")
    lines.append(r"\caption{Comparación del tiempo promedio de actualización para Sparse Table y Segment Tree dinámicos.}")
    lines.append(r"\end{figure}")
    return "\n".join(lines)

# ---------- Main ----------
def main():
    resumen = resumen_por_modelo_y_size()
    print("% ======= TABLA DE UPDATES =======")
    print(tabla_updates_latex(resumen))
    print("\n% ======= GRAFICO PGFPLOTS DE UPDATES =======")
    print(grafico_updates_pgfplots(resumen))

if __name__ == "__main__":
    main()

