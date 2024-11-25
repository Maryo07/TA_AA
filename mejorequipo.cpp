#include <iostream>
#include <vector>
#include <string>
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
map<string, vector<int>> posicion_jugadores;

// Representación de un cromosoma: un equipo
typedef vector<int> Cromosoma;

// Función para generar un cromosoma aleatorio
Cromosoma generarCromosoma() {
    Cromosoma cromosoma;

    for(pair<string, int> par: formacion) {
        for(int i = 0; i < par.second; i++) {
            int r = rand() % posicion_jugadores[par.first].size();
            cromosoma.push_back(posicion_jugadores[par.first][r]);
        }
    }

    return cromosoma;
}

// Función para calcular el fitness
double calcularFitness(const Cromosoma& cromosoma) {
    // Calcular el overall total
    int overallTotal = 0;
    for (int i : cromosoma) {
        overallTotal += jugadores[i].overall;
    }

    // Calcular la sinergia total
    double sinergiaTotal = 0;
    for (size_t i = 0; i < cromosoma.size(); i++) {
        for (size_t j = 0; j < cromosoma.size(); j++) {
            if(jugadores[i].nacionalidad == jugadores[j].nacionalidad or
                jugadores[i].club == jugadores[j].club) sinergiaTotal += 1;
        }
    }
    sinergiaTotal /= pow(cromosoma.size(), 2.0);

    // Fitness como multiplicacion de sinergia y overall
    return (overallTotal * sinergiaTotal);
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
        cout << jugadores[id].posicion << " - ID: " << id << " (Overall: " << jugadores[id].overall << ")";
        cout << jugadores[id].nacionalidad << " " << jugadores[id].club << endl;
    }
}

// limpieza de lectura
string limpiarString(const std::string& str) {
    string limpio;
    for (char c : str) {
        if (isalnum(c)) {
            limpio += c;
        }
    }
    return limpio;
}

// Lectura de la data en csv
void leerJugadores() {
    string nombreArchivo = "data.csv";
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error abriendo archivo" << endl;
        exit(1);
    }

    string linea, palabra, tmp;
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        bool mala_linea = false;

        // leer valores entre comas y limpieza
        vector<string> row;
        while (getline(ss, palabra, ',')) {
            row.push_back(palabra);
            if(palabra.empty()) mala_linea = true;
        }
        row[4] = limpiarString(row[4]);
        if(mala_linea or row[4].empty()) continue;

        // crear futbolista
        Futbolista futbolista;
        futbolista.nombre = row[0];
        futbolista.nacionalidad = row[1];
        futbolista.overall = stoi(row[2]);
        futbolista.club = row[3];
        futbolista.posicion = row[4];

        // añadir id del jugador a su posicion
        posicion_jugadores.emplace(futbolista.posicion, vector<int>());
        posicion_jugadores[futbolista.posicion].push_back(jugadores.size());
        jugadores.push_back(futbolista);
    }
    archivo.close();
}

int main() {
    srand(time(0));

    // Leer Data
    leerJugadores();

    // Formación (mock up)
    for(pair<string, vector<int>> par: posicion_jugadores) {
        if(par.second.size() < 1000) continue;
        formacion.emplace(par.first, 1);
    }

    // Generaciones y población predefinidas
    int generaciones = 100;
    int tamPoblacion = 20;

    // Ejecutar algoritmo genético
    //Cromosoma mejorEquipo = algoritmoGenetico(generaciones, tamPoblacion);

    // Imprimir el mejor equipo
    //cout << "Mejor equipo:" << endl;
    //imprimirEquipo(mejorEquipo);

    return 0;
}
