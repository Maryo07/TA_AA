#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace std;

// Estructura para representar un futbolista
struct Futbolista {
    string nombre;
    string posicion;
    string club;
    string nacionalidad;
    int overall;
};

// Variables globales
map<string,int> formacion;
vector<Futbolista> jugadores;

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
Cromosoma generarCromosoma() {
    vector<int> porteros, defensas, medios, delanteros;

    for(int i = 0; i < jugadores.size(); i++) {
        if (jugadores[i].posicion == "Portero") porteros.push_back(i);
        else if (jugadores[i].posicion == "Defensa") defensas.push_back(i);
        else if (jugadores[i].posicion == "Medio") medios.push_back(i);
        else if (jugadores[i].posicion == "Delantero") delanteros.push_back(i);
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
int calcularFitness(const Cromosoma& cromosoma) {
    // Calcular el overall total
    int overallTotal = 0;
    for (int id : cromosoma) {
        overallTotal += jugadores[id].overall;
    }

    // Calcular la sinergia total
    int sinergiaTotal = 0;
    for (size_t i = 0; i < cromosoma.size(); i++) {
        for (size_t j = 0; j < cromosoma.size(); j++) {
            if(jugadores[i].nacionalidad == jugadores[j].nacionalidad) sinergiaTotal += 0.5;
            if(jugadores[i].club == jugadores[j].club) sinergiaTotal += 0.5;
        }
    }
    sinergiaTotal /= pow(cromosoma.size(), 2.0);

    // Fitness como multiplicacion de sinergia y overall
    return overallTotal + sinergiaTotal;
}

// Función para realizar una mutación
void mutarCromosoma(Cromosoma& cromosoma) {
    int pos = rand() % cromosoma.size();
    string tipo;

    if (pos == 0) tipo = "Portero";
    else if (pos <= 4) tipo = "Defensa";
    else if (pos <= 8) tipo = "Medio";
    else tipo = "Delantero";

    vector<int> posibles;
    for(int i = 0; i < jugadores.size(); i++) {
        if (jugadores[i].posicion == tipo && find(cromosoma.begin(), cromosoma.end(), i) == cromosoma.end()) {
            posibles.push_back(i);
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
Cromosoma algoritmoGenetico(int generaciones, int tamPoblacion) {
    vector<Cromosoma> poblacion;

    // Generar población inicial
    for (int i = 0; i < tamPoblacion; i++) {
        poblacion.push_back(generarCromosoma());
    }

    Cromosoma mejorEquipo;
    int mejorFitness = 0;

    for (int gen = 0; gen < generaciones; gen++) {
        vector<pair<int, Cromosoma>> fitnessPoblacion;

        for (const auto& cromosoma : poblacion) {
            int fitness = calcularFitness(cromosoma);
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
                mutarCromosoma(nuevaPoblacion[i]);
            }
        }

        poblacion = nuevaPoblacion;
    }

    return mejorEquipo;
}

// Imprimir el mejor equipo
void imprimirEquipo(const Cromosoma& equipo) {
    for(int id: equipo) {
        cout << jugadores[id].posicion << " - ID: " << id << " (Overall: " << jugadores[id].overall << ")" << endl;
    }
}

// Lectura de la data en csv
void leerData() {
    string nombreArchivo = "data.csv";
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error abriendo archivo" << endl;
        exit(1);
    }

    vector<string> row;
    string linea, palabra;
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        bool mala_linea = false;

        // leer valores entre comas
        while (getline(ss, palabra, ',')) {
            row.push_back(palabra);
            if(palabra.empty()) mala_linea = true;
        }
        if(mala_linea) continue;

        // añadir valor
        Futbolista futbolista;
        futbolista.nombre = row[0];
        futbolista.nacionalidad = row[1];
        futbolista.overall = stoi(row[2]);
        futbolista.club = row[3];
        futbolista.posicion = row[4];
        jugadores.push_back(futbolista);
    }
    archivo.close();
}

int main() {
    srand(time(0));

    // Leer Data
    leerData();

    // Generaciones y población predefinidas
    int generaciones = 100;
    int tamPoblacion = 20;

    // Ejecutar algoritmo genético
    Cromosoma mejorEquipo = algoritmoGenetico(generaciones, tamPoblacion);

    // Imprimir el mejor equipo
    cout << "Mejor equipo:" << endl;
    imprimirEquipo(mejorEquipo);

    return 0;
}
