import random

def generar_comandos_estaticos_para_n(n, num_queries=100):
    comandos = []
    for _ in range(num_queries):
        l = random.randint(0, n - 1)
        r = random.randint(l, n - 1)  # aseguramos l <= r
        comandos.append(f"{l} {r}")
    return comandos

def main():
    random.seed(0)  # opcional: para reproducibilidad

    tamanos = [1000, 2000, 3000, 4000, 5000]
    for n in tamanos:
        nombre_archivo = f"comandos_static_{n}.txt"
        comandos = generar_comandos_estaticos_para_n(n)

        with open(nombre_archivo, "w") as f:
            for linea in comandos:
                f.write(linea + "\n")

        print(f"✅ Archivo '{nombre_archivo}' creado con {len(comandos)} consultas para n={n}.")

if __name__ == "__main__":
    main()

