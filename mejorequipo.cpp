#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

// Estructura para representar un futbolista
struct Futbolista {
    int id;
    string posicion;
    int overall;
};

// Representación de un cromosoma: un equipo
typedef vector<int> Cromosoma;

// Matriz de sinergias (simétrica)
//cambiar del 1 al 10
vector<vector<int>> sinergias = {
    { 0,  5,  3, -2,  1,  4,  0,  2, -1,  3,  1},
    { 5,  0,  4,  1, -1,  2,  3, -2,  0,  1,  2},
    { 3,  4,  0,  2,  1, -3,  1,  4,  0,  2, -2},
    {-2,  1,  2,  0,  5,  3,  1,  0, -1,  4,  1},
    { 1, -1,  1,  5,  0,  4,  3,  2,  0, -1,  3},
    { 4,  2, -3,  3,  4,  0,  2,  1,  3,  2,  1},
    { 0,  3,  1,  1,  3,  2,  0,  4, -2,  1,  2},
    { 2, -2,  4,  0,  2,  1,  4,  0,  3, -1,  1},
    {-1,  0,  0, -1,  0,  3, -2,  3,  0,  1,  4},
    { 3,  1,  2,  4, -1,  2,  1, -1,  1,  0,  5},
    { 1,  2, -2,  1,  3,  1,  2,  1,  4,  5,  0}
};

// Función para generar un cromosoma aleatorio
Cromosoma generarCromosoma(const vector<Futbolista>& jugadores) {
    vector<int> porteros, defensas, medios, delanteros;

    for (const auto& jugador : jugadores) {
        if (jugador.posicion == "Portero") porteros.push_back(jugador.id);
        else if (jugador.posicion == "Defensa") defensas.push_back(jugador.id);
        else if (jugador.posicion == "Medio") medios.push_back(jugador.id);
        else if (jugador.posicion == "Delantero") delanteros.push_back(jugador.id);
    }

    Cromosoma cromosoma;

    random_shuffle(porteros.begin(), porteros.end());
    cromosoma.push_back(porteros[0]);

    random_shuffle(defensas.begin(), defensas.end());
    cromosoma.insert(cromosoma.end(), defensas.begin(), defensas.begin() + 4);

    random_shuffle(medios.begin(), medios.end());
    cromosoma.insert(cromosoma.end(), medios.begin(), medios.begin() + 4);

    random_shuffle(delanteros.begin(), delanteros.end());
    cromosoma.insert(cromosoma.end(), delanteros.begin(), delanteros.begin() + 2);

    return cromosoma;
}

// Función para calcular el fitness
int calcularFitness(const Cromosoma& cromosoma, const vector<Futbolista>& jugadores) {
    int overallTotal = 0;
    int sinergiaTotal = 0;

    // Calcular el overall total
    for (int id : cromosoma) {
        overallTotal += jugadores[id].overall;
    }

    // Calcular la sinergia total
    for (size_t i = 0; i < cromosoma.size(); i++) {
        for (size_t j = i + 1; j < cromosoma.size(); j++) {
            sinergiaTotal += sinergias[cromosoma[i]][cromosoma[j]];
        }
    }

    return overallTotal + sinergiaTotal; // Fitness combina overall y sinergia
}

// Función para realizar una mutación
void mutarCromosoma(Cromosoma& cromosoma, const vector<Futbolista>& jugadores) {
    int pos = rand() % cromosoma.size();
    string tipo;

    if (pos == 0) tipo = "Portero";
    else if (pos <= 4) tipo = "Defensa";
    else if (pos <= 8) tipo = "Medio";
    else tipo = "Delantero";

    vector<int> posibles;
    for (const auto& jugador : jugadores) {
        if (jugador.posicion == tipo && find(cromosoma.begin(), cromosoma.end(), jugador.id) == cromosoma.end()) {
            posibles.push_back(jugador.id);
        }
    }

    if (!posibles.empty()) {
        cromosoma[pos] = posibles[rand() % posibles.size()];
    }
}

// Función para realizar el cruce
Cromosoma cruzarCromosomas(const Cromosoma& padre1, const Cromosoma& padre2) {
    Cromosoma hijo = padre1;
    int puntoCorte = rand() % hijo.size();

    for (int i = puntoCorte; i < hijo.size(); i++) {
        hijo[i] = padre2[i];
    }

    return hijo;
}

// Algoritmo genético
Cromosoma algoritmoGenetico(const vector<Futbolista>& jugadores, int generaciones, int tamPoblacion) {
    vector<Cromosoma> poblacion;

    // Generar población inicial
    for (int i = 0; i < tamPoblacion; i++) {
        poblacion.push_back(generarCromosoma(jugadores));
    }

    Cromosoma mejorEquipo;
    int mejorFitness = 0;

    for (int gen = 0; gen < generaciones; gen++) {
        vector<pair<int, Cromosoma>> fitnessPoblacion;

        for (const auto& cromosoma : poblacion) {
            int fitness = calcularFitness(cromosoma, jugadores);
            fitnessPoblacion.push_back({fitness, cromosoma});
            if (fitness > mejorFitness) {
                mejorFitness = fitness;
                mejorEquipo = cromosoma;
            }
        }

        sort(fitnessPoblacion.begin(), fitnessPoblacion.end(), greater<>());

        vector<Cromosoma> nuevaPoblacion;

        for (int i = 0; i < tamPoblacion / 2; i++) {
            nuevaPoblacion.push_back(fitnessPoblacion[i].second);
        }

        while (nuevaPoblacion.size() < tamPoblacion) {
            int padre1 = rand() % (tamPoblacion / 2);
            int padre2 = rand() % (tamPoblacion / 2);
            nuevaPoblacion.push_back(cruzarCromosomas(nuevaPoblacion[padre1], nuevaPoblacion[padre2]));
        }

        for (int i = 1; i < nuevaPoblacion.size(); i++) {
            if (rand() % 100 < 10) {
                mutarCromosoma(nuevaPoblacion[i], jugadores);
            }
        }

        poblacion = nuevaPoblacion;
    }

    return mejorEquipo;
}

// Imprimir el mejor equipo
void imprimirEquipo(const Cromosoma& equipo, const vector<Futbolista>& jugadores) {
    for (int id : equipo) {
        for (const auto& jugador : jugadores) {
            if (jugador.id == id) {
                cout << jugador.posicion << " - ID: " << jugador.id << " (Overall: " << jugador.overall << ")" << endl;
            }
        }
    }
}

int main() {
    srand(time(0));

    // Lista de futbolistas
    vector<Futbolista> jugadores = {
        {0, "Portero", 85}, {1, "Defensa", 90}, {2, "Defensa", 88}, {3, "Defensa", 86},
        {4, "Defensa", 87}, {5, "Medio", 92}, {6, "Medio", 89}, {7, "Medio", 88},
        {8, "Medio", 87}, {9, "Delantero", 95}, {10, "Delantero", 93}
    };

    int generaciones = 100;
    int tamPoblacion = 20;

    // Ejecutar algoritmo genético
    Cromosoma mejorEquipo = algoritmoGenetico(jugadores, generaciones, tamPoblacion);

    // Imprimir el mejor equipo
    cout << "Mejor equipo encontrado:" << endl;
    imprimirEquipo(mejorEquipo, jugadores);

    return 0;
}
