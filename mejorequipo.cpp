#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <map>
#include <set>
#include <cmath>
#include <fstream>
#include <sstream>

#define ENDL (char)13

using namespace std;

struct Futbolista {
    string nombre;
    string posicion;
    string club;
    string nacionalidad;
    int overall;
};

// Restriccion de la formacion. Por cada posicion cuántos jugadores necesito.
map<string, int> formacion;
// Los jugadores por índice.
vector<Futbolista> jugadores;
// Índice de los jugadores por posicion.
map<string, set<int>> posicionJugadores;

double tasaMutacion, tasaCasamiento;
int tamanoPoblacion, numeroGeneraciones;

typedef vector<int> Cromosoma;

bool valido(Cromosoma cromosoma) {
    Cromosoma c = cromosoma;
    sort(c.begin(), c.end());
    // Si existen repetidos devuelve falso
    if(adjacent_find(c.begin(), c.end()) != c.end()) {
        return false;
    }
    else return true;
}

void poblacionInicial(vector<Cromosoma> &poblacion) {
    while(poblacion.size() < tamanoPoblacion) {
        Cromosoma cromosoma;
        // Itera entre todas las posiciones de la formacion
        for(pair<string, int> par: formacion) {
            // Repite por la cantidad de jugadores necesarias en la
            // posicion elegida
            for(int i = 0; i < par.second; i++) {
                // Elige aleatoriamente de los jugadores con la posicion elegida
                int r = rand() % posicionJugadores[par.first].size();
                auto it = posicionJugadores[par.first].begin();
                for(int j = 0; j < r; j++) it = next(it);

                // Añade el jugador aleatorio al cromosoma
                cromosoma.push_back(*it);
            }
        }
        if(valido(cromosoma)) poblacion.push_back(cromosoma);
    }
}

double fitness(const Cromosoma& cromosoma) {
    // Suma del overall de todos los jugadores del cromosoma
    int overallTotal = 0;
    for (int i : cromosoma) {
        overallTotal += jugadores[i].overall;
    }

    // Suma de la sinergia de cada jugador.
    double f = 0;
    for (size_t i = 0; i < cromosoma.size(); i++) {
        // Para cada jugador, añade 0.5 si encuentra a otro que coincida con su
        // equipo y 0.5 adicional si coincide con su pais.
        for (size_t j = 0; j < cromosoma.size(); j++) {
            if(jugadores[cromosoma[i]].club
                == jugadores[cromosoma[j]].club)
                f += (0.5*jugadores[cromosoma[i]].overall);

            if(jugadores[cromosoma[i]].nacionalidad
                == jugadores[cromosoma[j]].nacionalidad) 
                f += (0.5*jugadores[cromosoma[i]].overall);

            f += jugadores[cromosoma[i]].overall;
        }
    }
    // Como máximo cada jugador obtiene 1 de sinergia, por lo que se divide
    // entre el cuadrado de la cantidad de jugadores para obtener un número del
    // 0 al 1.
    //f /= pow(cromosoma.size(), 2.0);

    // Fitness como multiplicacion de sinergia y overall
    return (f);
}

Cromosoma mutarCromosoma(Cromosoma cromosoma) {
    int n  = round(cromosoma.size() * tasaMutacion);

    for(int i = 0; i < n; i++) {
        // selecciona aleatoriamente un índice j del cromosoma para reemplazar
        int j = rand() % cromosoma.size();
        string posicion = jugadores[cromosoma[j]].posicion;

        // Elige aleatoriamente de los jugadores con la posicion elegida
        int r = rand() % posicionJugadores[posicion].size();
        auto it = posicionJugadores[posicion].begin();
        for(int i = 0; i < r; i++) it = next(it);

        // Reemplazo en el cromosoma
        cromosoma[j] = *it;
    }

    return cromosoma;
}

Cromosoma cruzarCromosomas(const Cromosoma& padre1, const Cromosoma& padre2) {
    Cromosoma hijo = padre1;
    int puntoCorte = rand() % hijo.size();
    for (int i = puntoCorte; i < hijo.size(); i++) {
        hijo[i] = padre2[i];
    }
    return hijo;
}

void mutarPadres(vector<Cromosoma> &poblacion, const vector<Cromosoma> &padres) {
    for(int i = 0; i < padres.size(); i++) {
        Cromosoma mutado = mutarCromosoma(padres[i]);
        if(valido(mutado))
            poblacion.push_back(mutado);
    }
}

void generarHijos(vector<Cromosoma> &poblacion,
    const vector<Cromosoma> &padres) {
    for(int i = 0; i < padres.size(); i++)
        for(int j = 0; j < padres.size(); j++)
            if(i != j) {
                Cromosoma hijo = cruzarCromosomas(padres[i], padres[j]);
                if(valido(hijo))
                    poblacion.push_back(hijo);
            }
}

void generarPadres(const vector<Cromosoma> &poblacion,
    vector<Cromosoma> &padres) {
    vector<Cromosoma> ruleta = poblacion;
    double fitnessTotal = 0.0;
    for(Cromosoma c: poblacion) fitnessTotal += fitness(c);

    // Giramos la ruleta
    for(int i = 0; i < round(poblacion.size() * tasaCasamiento); i++) {
        // elegimos un double aleatorio de 0 a fitnessTotal
        double r = ((double)rand() / (RAND_MAX)) * fitnessTotal;
        double fitnessActual = 0.0;
        for(int j = 0; j < ruleta.size(); j++) {
            // Voy sumando fitness por cada elemento de la ruleta
            fitnessActual += fitness(ruleta[j]);
            // Si mi aleatorio es menor o igual que el fitness actual entonces
            // tomo el elemento actual, lo guardo en los padres y lo quito de la
            // ruleta. El fitness total habŕa cambiado también.
            if(r <= fitnessActual) {
                padres.push_back(ruleta[j]);
                fitnessTotal -= fitness(ruleta[j]);
                ruleta.erase(ruleta.begin()+j);
                break;
            }
        }
    }
}

bool cmp(const Cromosoma a, const Cromosoma b){
    return fitness(a) > fitness(b);
}

void eliminarDuplicados(vector<vector<int>> &poblacion) {
    // Para igualar vectores es necesario ordernarlos primero, pero como
    // no queremos alterar la poblacion, los copiamos
    for(int i = 0; i < poblacion.size(); i++) {
        Cromosoma ivec = poblacion[i];
        sort(ivec.begin(), ivec.end());
        for(int j = i+1; j < poblacion.size(); j++) {
            Cromosoma jvec = poblacion[j];
            sort(jvec.begin(), jvec.end());
            // Si el cromosoma en i es igual que el j, elimina el j
            if (ivec == jvec) {
                poblacion.erase(poblacion.begin() + j);
                j--;
            }
        }   
    }
}

void reducirPoblacion(vector<Cromosoma> &poblacion){
    sort(poblacion.begin(),poblacion.end(),cmp);
    poblacion.erase(poblacion.begin()+tamanoPoblacion,poblacion.end());
}

Cromosoma algoritmoGenetico() {
    srand(time(NULL));
    vector<Cromosoma> poblacion, padres;
    poblacionInicial(poblacion);

    for (int _ = 0; _ < numeroGeneraciones; _++) {
        vector<Cromosoma> padres;
        generarPadres(poblacion, padres);
        generarHijos(poblacion, padres);
        mutarPadres(poblacion, padres);
        eliminarDuplicados(poblacion);
        reducirPoblacion(poblacion);
    }

    return poblacion[0];
}

void imprimirEquipo(const Cromosoma& equipo) {
    for(int id: equipo) {
        cout << jugadores[id].posicion << " - : " << jugadores[id].nombre;
        cout << " (Overall: " << jugadores[id].overall << ")";
        cout << " (Nacionalidad: " << jugadores[id].nacionalidad << ")";
        cout << " (Club: " << jugadores[id].club << ")" << endl;
    }
}

void leerJugadores() {
    string nombreArchivo = "data.csv";
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error abriendo archivo csv" << endl;
        exit(1);
    }

    string linea, palabra, tmp;
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        bool mala_linea = false;

        // leer valores entre comas y limpieza
        vector<string> row;
        for(int i = 0; i < 5; i++) {
            if(i==4) getline(ss, palabra, ENDL);
            else getline(ss, palabra, ',');

            if(palabra.empty()) {
                mala_linea=true;
                break;
            }
            row.push_back(palabra);
        }
        if(mala_linea) {
            continue;
        }

        // crear futbolista
        Futbolista futbolista;
        futbolista.nombre = row[0];
        futbolista.nacionalidad = row[1];
        futbolista.overall = stoi(row[2]);
        futbolista.club = row[3];
        futbolista.posicion = row[4];

        // añadir id del jugador a su posicion
        posicionJugadores.emplace(futbolista.posicion, set<int>());
        posicionJugadores[futbolista.posicion].insert(jugadores.size());
        jugadores.push_back(futbolista);
    }
    archivo.close();
}

void leerFormacion() {
    string nombreArchivo = "Formaciones.txt";
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error abriendo archivo txt" << endl;
        exit(1);
    }

    string linea, palabra;
    getline(archivo, linea);
    
    stringstream ss(linea);
    getline(ss, palabra, ',');
    int n = stoi(palabra);
    for(int i = 0; i < n; i++) {
        if(i == n-1) getline(ss, palabra, ENDL);
        else getline(ss, palabra, ',');

        if(formacion.find(palabra) != formacion.end()) {
            formacion[palabra] = formacion[palabra] + 1;
        } else {
            formacion.emplace(palabra, 1);
        }
    }
    archivo.close();
}

int main() {
    leerJugadores();
    leerFormacion();

    numeroGeneraciones = 100;
    tamanoPoblacion = 20;
    tasaMutacion = 0.5;
    tasaCasamiento = 0.5;

    Cromosoma mejorEquipo;
    for(int i = 0; i < 10; i++) {
        Cromosoma c = algoritmoGenetico();
        if(mejorEquipo.size() == 0 or fitness(c) > fitness(mejorEquipo))
            mejorEquipo = c;
    }

    // Imprimir el mejor equipo
    cout << "Mejor equipo:" << endl;
    imprimirEquipo(mejorEquipo);
    cout << fitness(mejorEquipo) << endl;
    
    return 0;
}
