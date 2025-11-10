import random

def generar_archivo(nombre, n, minimo=0, maximo=9999):
    """Genera un archivo con n enteros aleatorios entre [minimo, maximo]."""
    with open(nombre, "w") as f:
        numeros = [str(random.randint(minimo, maximo)) for _ in range(n)]
        f.write(" ".join(numeros))
    print(f"✅ Archivo '{nombre}' creado con {n} enteros aleatorios.")


def main():
    tamanos = [1000, 2000, 3000, 4000, 5000]
    for n in tamanos:
        nombre = f"dataset_{n}.txt"
        generar_archivo(nombre, n)


if __name__ == "__main__":
    main()

