# generar_comandos_rmq.py
import random

def generar_comandos_para_n(n, num_queries=100, num_updates=30,
                            valor_min=0, valor_max=9999):
    comandos = []

    # 100 consultas de rango
    for _ in range(num_queries):
        l = random.randint(0, n - 1)
        r = random.randint(l, n - 1)  # aseguramos l <= r
        comandos.append(f"Q {l} {r}")

    # 30 updates
    for _ in range(num_updates):
        i = random.randint(0, n - 1)
        v = random.randint(valor_min, valor_max)
        comandos.append(f"U {i} {v}")

    # Mezclar todo para intercalar Q y U
    random.shuffle(comandos)
    return comandos

def main():
    random.seed(0)  # opcional: para resultados reproducibles

    tamanos = [1000, 2000, 3000, 4000, 5000]
    for n in tamanos:
        nombre_archivo = f"comandos_{n}.txt"
        comandos = generar_comandos_para_n(n)

        with open(nombre_archivo, "w") as f:
            for linea in comandos:
                f.write(linea + "\n")

        print(f"✅ Archivo '{nombre_archivo}' creado con "
              f"{len(comandos)} comandos (100 Q + 30 U) para n={n}.")

if __name__ == "__main__":
    main()
