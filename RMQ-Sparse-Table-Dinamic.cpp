// RMQ-Sparse-Table-Dinamic.cpp
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

    // 1) Leer el arreglo desde archivo
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

    // 3) Construcción inicial del RMQ (sparse table) midiendo tiempo en ns
    auto t_build_start = chrono::high_resolution_clock::now();
    rmq_support_sparse_table<> rmq(&A);
    auto t_build_end = chrono::high_resolution_clock::now();

    auto build_ns =
        chrono::duration_cast<chrono::nanoseconds>(t_build_end - t_build_start).count();

    // Tamaño del RMQ en memoria (solo la estructura) en MB
    size_t rmq_bytes = size_in_bytes(rmq);
    double rmq_mb = static_cast<double>(rmq_bytes) / (1024.0 * 1024.0);

    cout << "Construcción del RMQ (sparse table dinámico) tomó "
         << build_ns << " ns\n";
    cout << "Tamaño del RMQ en memoria ~ " << rmq_mb << " MB\n";

    // 3.b) Guardar en CSV de construcción: size,rmq_mb,build_ns
    {
        ofstream csv("construccion-rmq-sparse-table-dinamic.csv", ios::app);
        if (!csv) {
            cerr << "Advertencia: no se pudo abrir construccion-rmq-sparse-table-dinamic.csv para escritura.\n";
        } else {
            csv << A.size() << "," << rmq_mb << "," << build_ns << "\n";
        }
    }

    cout << "Modo dinámico RMQ (Sparse Table)\n";
    cout << "Comandos:\n";
    cout << "  Q l r   -> consulta mínimo en [l, r]\n";
    cout << "  U i v   -> update: A[i] = v (reconstruye RMQ y mide tiempo)\n";
    cout << "  exit    -> salir\n\n";

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
        char op;
        ss >> op;

        if (!ss) {
            cout << "Entrada inválida. Usa: Q l r  o  U i v  o 'exit'.\n";
            continue;
        }

        if (op == 'Q' || op == 'q') {
            size_t a, b;
            if (!(ss >> a >> b)) {
                cout << "Formato de consulta inválido. Usa: Q l r\n";
                continue;
            }

            // Permitimos Q l r o Q r l, y ordenamos
            size_t l = min(a, b);
            size_t r = max(a, b);

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

            // Guardar en CSV de consultas: size,rango,tiempo_ns
            size_t rango = r - l + 1;
            ofstream csv("consultas-rmq-sparse-table.csv", ios::app);
            if (!csv) {
                cerr << "Advertencia: no se pudo abrir consultas-rmq-sparse-table.csv para escritura.\n";
            } else {
                csv << A.size() << "," << rango << "," << query_ns << "\n";
            }

        } else if (op == 'U' || op == 'u') {
            size_t i;
            long long v;
            if (!(ss >> i >> v)) {
                cout << "Formato de update inválido. Usa: U i v\n";
                continue;
            }

            if (i >= A.size()) {
                cout << "Índice fuera de límites. El arreglo tiene tamaño "
                     << A.size() << " (índices 0.." << (A.size() - 1) << ").\n";
                continue;
            }

            // 1) Hacemos el update y reconstruimos el RMQ midiendo tiempo
            auto t_update_start = chrono::high_resolution_clock::now();

            // Actualizar el valor en A[i]
            A[i] = static_cast<uint64_t>(v);
            // No llamamos bit_compress(A) aquí para no mezclar su costo con el de reconstrucción

            // Reconstruir la estructura RMQ (sparse table)
            rmq = rmq_support_sparse_table<>(&A);

            auto t_update_end = chrono::high_resolution_clock::now();
            auto update_ns =
                chrono::duration_cast<chrono::nanoseconds>(t_update_end - t_update_start).count();

            cout << "Update A[" << i << "] = " << v
                 << " completado. Tiempo de reconstrucción: "
                 << update_ns << " ns\n";

            // 2) Guardar resultado en CSV de updates:
            //    size,indice,valor,tiempo_ns
            ofstream csv("update-rmq-sparse-table-dinamic.csv", ios::app);
            if (!csv) {
                cerr << "Advertencia: no se pudo abrir update-rmq-sparse-table-dinamic.csv para escritura.\n";
            } else {
                csv << A.size() << ","
                    << i << ","
                    << v << ","
                    << update_ns << "\n";
            }

        } else {
            cout << "Comando no reconocido. Usa 'Q', 'U' o 'exit'.\n";
        }
    }

    cout << "Saliendo.\n";
    return 0;
}
