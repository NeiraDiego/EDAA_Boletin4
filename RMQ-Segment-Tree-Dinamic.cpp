// RMQ-Segment-Tree-Dinamic.cpp
// RMQ-Segment-Tree-Dinamic.cpp
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

// Segment Tree estilo rmq_*: trabaja sobre un int_vector<> externo
// y entrega el índice del mínimo en [l, r]. Además permite updates O(log n).
struct rmq_segment_tree {
    const int_vector<>* A;  // puntero al arreglo original
    int n;
    vector<int> st;         // st[p] guarda índice del mínimo en el nodo

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

    // Combina dos índices devolviendo el índice del mínimo (empate: menor índice)
    int combine(int i, int j) const {
        if (i == -1) return j;
        if (j == -1) return i;
        auto vi = (*A)[i];
        auto vj = (*A)[j];
        if (vi < vj) return i;
        if (vj < vi) return j;
        return (i < j ? i : j);
    }

    // Query interna
    int query_rec(int p, int l, int r, int ql, int qr) const {
        if (qr < l || ql > r) {
            return -1;
        }
        if (ql <= l && r <= qr) {
            return st[p];
        }
        int mid = (l + r) / 2;
        int left_idx  = query_rec(p * 2,     l,     mid, ql, qr);
        int right_idx = query_rec(p * 2 + 1, mid+1, r,   ql, qr);
        return combine(left_idx, right_idx);
    }

    // Query pública: índice del mínimo en [l, r]
    int query(int l, int r) const {
        if (!A || n == 0) return -1;
        if (l < 0) l = 0;
        if (r >= n) r = n - 1;
        if (l > r) return -1;
        return query_rec(1, 0, n - 1, l, r);
    }

    int operator()(size_t l, size_t r) const {
        return query(static_cast<int>(l), static_cast<int>(r));
    }

    // Update interna: recalcula la rama que contiene idx
    void update_rec(int p, int l, int r, int idx) {
        if (l == r) {
            st[p] = l;
            return;
        }
        int mid = (l + r) / 2;
        if (idx <= mid)
            update_rec(p * 2, l, mid, idx);
        else
            update_rec(p * 2 + 1, mid + 1, r, idx);
        st[p] = combine(st[p * 2], st[p * 2 + 1]);
    }

    // Update pública: ya se actualizó A[idx] afuera; aquí solo se actualiza el árbol
    void update(int idx) {
        if (!A || n == 0) return;
        if (idx < 0 || idx >= n) return;
        update_rec(1, 0, n - 1, idx);
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

    // 3) Construcción inicial del Segment Tree (RMQ) midiendo tiempo y memoria
    auto t_build_start = chrono::high_resolution_clock::now();
    rmq_segment_tree rmq(&A);
    auto t_build_end = chrono::high_resolution_clock::now();

    auto build_ns =
        chrono::duration_cast<chrono::nanoseconds>(t_build_end - t_build_start).count();

    // Tamaño del RMQ en memoria (solo el árbol) en MB
    size_t rmq_bytes = rmq.st.size() * sizeof(int);
    double rmq_mb = static_cast<double>(rmq_bytes) / (1024.0 * 1024.0);

    cout << "Construcción del RMQ (segment tree dinámico) tomó "
         << build_ns << " ns\n";
    cout << "Tamaño del RMQ en memoria ~ " << rmq_mb << " MB\n";

    // Guardar construcción en CSV: tamaño_arreglo, tamaño_rmq_MB, tiempo_ns
    {
        ofstream csv("construccion-rmq-segment-tree-dinamic.csv", ios::app);
        if (!csv) {
            cerr << "Advertencia: no se pudo abrir construccion-rmq-segment-tree-dinamic.csv para escritura.\n";
        } else {
            csv << A.size() << "," << rmq_mb << "," << build_ns << "\n";
        }
    }

    cout << "Modo dinámico RMQ (Segment Tree)\n";
    cout << "Comandos:\n";
    cout << "  Q l r   -> consulta mínimo en [l, r]\n";
    cout << "  U i v   -> update: A[i] = v (update O(log n) en el árbol)\n";
    cout << "  exit    -> salir\n\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) {
            break; // EOF
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

            size_t l = min(a, b);
            size_t r = max(a, b);

            if (l > r || r >= A.size()) {
                cout << "Rango fuera de límites. El arreglo tiene tamaño "
                     << A.size() << " (índices 0.." << (A.size() - 1) << ").\n";
                continue;
            }

            // Medir tiempo de la consulta
            auto t_query_start = chrono::high_resolution_clock::now();
            int min_idx = rmq(l, r);
            auto t_query_end = chrono::high_resolution_clock::now();

            auto query_ns =
                chrono::duration_cast<chrono::nanoseconds>(t_query_end - t_query_start).count();

            if (min_idx == -1) {
                cout << "Error interno en la consulta.\n";
            } else {
                cout << "Mínimo en [" << l << ", " << r << "] está en índice "
                     << min_idx << " y vale A[" << min_idx << "] = " << A[min_idx] << "\n";
                cout << "Tiempo de consulta: " << query_ns << " ns\n";
            }

            // Guardar en CSV de consultas: size,rango,tiempo_ns
            size_t rango = r - l + 1;
            ofstream csv("consultas-rmq-segment-tree.csv", ios::app);
            if (!csv) {
                cerr << "Advertencia: no se pudo abrir consultas-rmq-segment-tree.csv para escritura.\n";
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

            // Medimos el tiempo total del update: escribir en A y actualizar árbol
            auto t_update_start = chrono::high_resolution_clock::now();

            // Actualizar valor en A[i]
            A[i] = static_cast<uint64_t>(v);
            // No llamamos bit_compress aquí para no mezclar su costo

            // Actualizar el árbol en O(log n)
            rmq.update(static_cast<int>(i));

            auto t_update_end = chrono::high_resolution_clock::now();
            auto update_ns =
                chrono::duration_cast<chrono::nanoseconds>(t_update_end - t_update_start).count();

            cout << "Update A[" << i << "] = " << v
                 << " completado. Tiempo de update (árbol): "
                 << update_ns << " ns\n";

            // Guardar en CSV: size,indice,valor,tiempo_ns
            ofstream csv("update-rmq-segment-tree-dinamic.csv", ios::app);
            if (!csv) {
                cerr << "Advertencia: no se pudo abrir update-rmq-segment-tree-dinamic.csv para escritura.\n";
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
