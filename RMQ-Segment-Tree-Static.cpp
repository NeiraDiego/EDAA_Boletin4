// basado en codigo de https://www.geeksforgeeks.org/dsa/sparse-table/
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>

#include <sdsl/int_vector.hpp>
#include <sdsl/util.hpp>

using namespace std;
using namespace sdsl;

// Segment Tree estilo "rmq_*" que trabaja sobre un int_vector<> externo
// y entrega el índice del mínimo en [l, r].
struct rmq_segment_tree {
    const int_vector<>* A;   // puntero al arreglo original (no se modifica)
    int n;
    vector<int> st;          // st[p] guarda el índice del mínimo en ese nodo

    rmq_segment_tree() : A(nullptr), n(0) {}

    rmq_segment_tree(const int_vector<>* a) {
        build(a);
    }

    void build(const int_vector<>* a) {
        A = a;
        n = static_cast<int>(A->size());
        if (n == 0) {
            st.clear();
            return;
        }
        st.assign(4 * n, 0);
        build_rec(1, 0, n - 1);
    }

    int build_rec(int p, int l, int r) {
        if (l == r) {
            st[p] = l;
            return l;
        }
        int mid = (l + r) / 2;
        int left_idx  = build_rec(p * 2,     l,     mid);
        int right_idx = build_rec(p * 2 + 1, mid+1, r);
        st[p] = combine(left_idx, right_idx);
        return st[p];
    }

    // Combina dos índices devolviendo el índice del mínimo (empate: menor índice).
    int combine(int i, int j) const {
        if (i == -1) return j;
        if (j == -1) return i;
        auto vi = (*A)[i];
        auto dj = (*A)[j];
        if (vi < dj) return i;
        if (dj < vi) return j;
        return (i < j ? i : j);
    }

    // Query interna
    int query_rec(int p, int l, int r, int ql, int qr) const {
        if (qr < l || r < ql) {
            return -1; // índice inválido
        }
        if (ql <= l && r <= qr) {
            return st[p];
        }
        int mid = (l + r) / 2;
        int left_idx  = query_rec(p * 2,     l,     mid, ql, qr);
        int right_idx = query_rec(p * 2 + 1, mid+1, r,   ql, qr);
        return combine(left_idx, right_idx);
    }

    // Query pública: devuelve índice del mínimo en [l, r] (0-based, inclusivo)
    int query(int l, int r) const {
        if (!A || n == 0) return -1;
        if (l < 0) l = 0;
        if (r >= n) r = n - 1;
        if (l > r) return -1;
        return query_rec(1, 0, n - 1, l, r);
    }

    // Para que se use igual que rmq_support_* de SDSL: rmq(l, r)
    int operator()(size_t l, size_t r) const {
        return query(static_cast<int>(l), static_cast<int>(r));
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo_enteros\n";
        cerr << "El archivo debe contener enteros separados por espacios o saltos de línea.\n";
        return 1;
    }

    // 1) Leer el arreglo desde archivo en un vector temporal
    ifstream in(argv[1]);
    if (!in) {
        cerr << "Error: no se pudo abrir el archivo " << argv[1] << "\n";
        return 1;
    }

    vector<uint64_t> tmp;
    long long x;
    while (in >> x) {
        tmp.push_back(static_cast<uint64_t>(x));
    }

    if (tmp.empty()) {
        cerr << "Error: el archivo no contiene enteros válidos.\n";
        return 1;
    }

    // 2) Pasar a int_vector<> y comprimir ancho de bits
    int_vector<> A(tmp.size());
    for (size_t i = 0; i < tmp.size(); ++i) {
        A[i] = tmp[i];
    }
    util::bit_compress(A);

    cout << "Arreglo cargado (" << A.size() << " elementos):\n";
    cout << "A = " << A << "\n\n";

    // 3) Construcción del Segment Tree (RMQ) midiendo tiempo en ns
    auto t_build_start = chrono::high_resolution_clock::now();
    rmq_segment_tree rmq(&A);
    auto t_build_end = chrono::high_resolution_clock::now();

    auto build_ns =
        chrono::duration_cast<chrono::nanoseconds>(t_build_end - t_build_start).count();

    // Calcular tamaño aproximado del RMQ en memoria (solo el árbol) en MB
    size_t rmq_bytes = rmq.st.size() * sizeof(int);
    double rmq_mb = static_cast<double>(rmq_bytes) / (1024.0 * 1024.0);

    cout << "Construcción del RMQ (segment tree estático) tomó "
         << build_ns << " ns\n";
    cout << "Tamaño del RMQ en memoria ~ " << rmq_mb << " MB\n";

    // 3.b) Guardar en CSV de construcción:
    //      tamaño_arreglo, tamaño_rmq_MB, tiempo_ns
    {
        ofstream csv("construccion-rmq-segment-tree-static.csv", ios::app);
        if (!csv) {
            cerr << "Advertencia: no se pudo abrir construccion-rmq-segment-tree-static.csv para escritura.\n";
        } else {
            csv << A.size() << "," << rmq_mb << "," << build_ns << "\n";
        }
    }

    // 4) Loop interactivo de consultas
    cout << "\nListo para consultas RMQ con Segment Tree.\n";
    cout << "Formato: l r  (índices 0-based, inclusive)\n";
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
            cout << "Entrada inválida. Usa: l r  o 'exit'.\n";
            continue;
        }

        if (l > r || r >= A.size()) {
            cout << "Rango fuera de límites. El arreglo tiene tamaño "
                 << A.size() << " (índices 0.." << (A.size() - 1) << ").\n";
            continue;
        }

        // Medir tiempo de la consulta en ns
        auto t_query_start = chrono::high_resolution_clock::now();
        int min_idx = rmq(l, r);
        auto t_query_end = chrono::high_resolution_clock::now();

        auto query_ns =
            chrono::duration_cast<chrono::nanoseconds>(t_query_end - t_query_start).count();

        if (min_idx == -1) {
            cout << "Error interno: no se encontró mínimo en el rango.\n";
        } else {
            cout << "Mínimo en [" << l << ", " << r << "] está en índice "
                 << min_idx << " y vale A[" << min_idx << "] = " << A[min_idx] << "\n";
            cout << "Tiempo de consulta: " << query_ns << " ns\n";
        }

        // Guardar en CSV de consultas: size, tamaño del rango y tiempo en ns
        size_t rango = r - l + 1;
        ofstream csv("consultas-rmq-segment-tree-static.csv", ios::app);
        if (!csv) {
            cerr << "Advertencia: no se pudo abrir consultas-rmq-segment-tree-static.csv para escritura.\n";
        } else {
            // Formato: size,rango,tiempo_ns
            csv << A.size() << "," << rango << "," << query_ns << "\n";
        }
    }

    cout << "Saliendo.\n";
    return 0;
}
