#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>

#include <sdsl/int_vector.hpp>
#include <sdsl/rmq_support.hpp>
#include <sdsl/util.hpp>

using namespace std;
using namespace sdsl;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo_enteros\n";
        cerr << "El archivo debe contener enteros separados por espacios o saltos de línea.\n";
        return 1;
    }

    // 1) Leer el arreglo desde archivo normal
    ifstream in(argv[1]);
    if (!in) {
        cerr << "Error: no se pudo abrir el archivo " << argv[1] << "\n";
        return 1;
    }

    vector<uint64_t> vec;
    long long x;
    while (in >> x) {
        vec.push_back(static_cast<uint64_t>(x));
    }

    if (vec.empty()) {
        cerr << "Error: el archivo no contiene enteros válidos.\n";
        return 1;
    }

    // 2) Pasar a int_vector<> de SDSL y comprimir el ancho de bits
    int_vector<> A(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        A[i] = vec[i];
    }
    util::bit_compress(A);  // ajusta el ancho mínimo necesario

    cout << "Arreglo cargado (" << A.size() << " elementos):\n";
    cout << "A = " << A << "\n\n";

    // 3) Construir la Sparse Table de RMQ (mínimo) midiendo el tiempo en ns
    auto t_build_start = chrono::high_resolution_clock::now();
    rmq_support_sparse_table<> rmq(&A);
    auto t_build_end = chrono::high_resolution_clock::now();

    auto build_ns =
        chrono::duration_cast<chrono::nanoseconds>(t_build_end - t_build_start).count();

    // Tamaño de la estructura RMQ en memoria (solo sparse table) en MB
    size_t rmq_bytes = size_in_bytes(rmq);
    double rmq_mb = static_cast<double>(rmq_bytes) / (1024.0 * 1024.0);

    cout << "Construcción del RMQ (sparse table) tomó " << build_ns << " ns\n";
    cout << "Tamaño del RMQ en memoria ~ " << rmq_mb << " MB\n";

    // 3.b) Guardar en CSV de construcción:
    //      tamaño_arreglo, tamaño_rmq_MB, tiempo_ns
    {
        ofstream csv("construccion-rmq-sparse-table-static.csv", ios::app);
        if (!csv) {
            cerr << "Advertencia: no se pudo abrir construccion-rmq-sparse-table-static.csv para escritura.\n";
        } else {
            csv << A.size() << "," << rmq_mb << "," << build_ns << "\n";
        }
    }

    // 4) Loop interactivo de consultas
    cout << "\nListo para consultas RMQ.\n";
    cout << "Formato: i j (rango (con base 0) de la i a la j separados por espacio)\n";
    cout << "Escribe 'exit' para salir.\n\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) {
            // EOF o error de entrada
            break;
        }

        if (line == "exit" || line == "EXIT" || line == "Exit") {
            break;
        }
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        size_t l, r;
        if (!(ss >> l >> r)) {
            cout << "Entrada inválida. Usa: i j  o 'exit'.\n";
            continue;
        }

        if (l > r || r >= A.size()) {
            cout << "Rango fuera de límites. El arreglo tiene tamaño "
                 << A.size() << " (índices 0.." << (A.size() - 1) << ").\n";
            continue;
        }

        // Medir tiempo de la consulta en ns
        auto t_query_start = chrono::high_resolution_clock::now();
        auto min_idx = rmq(l, r);
        auto t_query_end = chrono::high_resolution_clock::now();

        auto query_ns =
            chrono::duration_cast<chrono::nanoseconds>(t_query_end - t_query_start).count();

        cout << "Mínimo en [" << l << ", " << r << "] está en índice "
             << min_idx << " y vale A[" << min_idx << "] = " << A[min_idx] << "\n";
        cout << "Tiempo de consulta: " << query_ns << " ns\n";

        // Guardar en CSV de consultas: tamaño_arreglo, tamaño_rango, tiempo_ns
        size_t rango = r - l + 1;
        ofstream csv("consultas-rmq-sparse-table-static.csv", ios::app);
        if (!csv) {
            cerr << "Advertencia: no se pudo abrir consultas-rmq-sparse-table-static.csv para escritura.\n";
        } else {
            // Formato: size,rango,tiempo_ns
            csv << A.size() << "," << rango << "," << query_ns << "\n";
        }
    }

    cout << "Saliendo.\n";
    return 0;
}
