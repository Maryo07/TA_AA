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

using namespace std;

struct Futbolista {
    string nombre;
    string posicion;
    string club;
    string nacionalidad;
    int overall;
};

struct Formacion{
    vector<string> Defensores;
    vector<string> Delanteros;
    vector<string> MedioCampistas;
};

// Restriccion de la formacion. Por cada posicion cuántos jugadores necesito.
map<int, Formacion> formac;
// Los jugadores por índice.
vector<Futbolista> jugadores;
// Índice de los jugadores por posicion.
map<string, set<int>> posicion_jugadores;

int formacionDeseada;
double tmutacion;

typedef vector<int> Cromosoma;

Cromosoma generarCromosoma() {
    Cromosoma cromosoma;
    //elgir un portero al azar
    int r = rand()%posicion_jugadores["GK"].size();
    auto it = posicion_jugadores["GK"].begin();
    for(int i = 0; i < r; i++) it = next(it);
    cromosoma.push_back(*it);

    //elegir los delanteros
    for(int i=0; i<formac[formacionDeseada].Delanteros.size(); i++){
        string tipo=formac[formacionDeseada].Delanteros[i];
        r = rand() % posicion_jugadores[tipo].size();
        auto it = posicion_jugadores[tipo].begin();
        for(int k = 0; k < r; k++) it = next(it);
        // Añade el jugador aleatorio al cromosoma
        cromosoma.push_back(*it);
        // Lo elimina del mapa de índices por posicion para no volver a
        // seleccionarlo (*)
        posicion_jugadores[tipo].erase(it);
    }

    //elegir los mediocampistas
    for(int i=0; i<formac[formacionDeseada].MedioCampistas.size(); i++){
        string tipo=formac[formacionDeseada].MedioCampistas[i];
        r = rand() % posicion_jugadores[tipo].size();
        auto it = posicion_jugadores[tipo].begin();
        for(int k = 0; k < r; k++) it = next(it);
        // Añade el jugador aleatorio al cromosoma
        cromosoma.push_back(*it);
        // Lo elimina del mapa de índices por posicion para no volver a
        // seleccionarlo (*)
        posicion_jugadores[tipo].erase(it);
    }

    //elegir los defensas
    for(int i=0; i<formac[formacionDeseada].Defensores.size(); i++){
        string tipo=formac[formacionDeseada].Defensores[i];
        r = rand() % posicion_jugadores[tipo].size();
        auto it = posicion_jugadores[tipo].begin();
        for(int k = 0; k < r; k++) it = next(it);
        // Añade el jugador aleatorio al cromosoma
        cromosoma.push_back(*it);
        // Lo elimina del mapa de índices por posicion para no volver a
        // seleccionarlo (*)
        posicion_jugadores[tipo].erase(it);
    }

    // Añade nuevamente todos los jugadores al mapa de índices por posicion para
    // no alterarlo (*)
    for(int id: cromosoma) {
        posicion_jugadores[jugadores[id].posicion].insert(id);
    }

    return cromosoma;
}

double calcularFitness(const Cromosoma& cromosoma) {
    // Suma del overall de todos los jugadores del cromosoma
    int overallTotal = 0;
    for (int i : cromosoma) {
        overallTotal += jugadores[i].overall;
    }

    // Suma de la sinergia de cada jugador.
    double sinergiaTotal = 0;
    for (size_t i = 0; i < cromosoma.size(); i++) {
        // Para cada jugador, añade 0.5 si encuentra a otro que coincida con su
        // equipo y 0.5 adicional si coincide con su pais.
        for (size_t j = 0; j < cromosoma.size(); j++) {
            if(jugadores[i].nacionalidad == jugadores[j].nacionalidad)
                sinergiaTotal += 0.5;
            if(jugadores[i].club == jugadores[j].club)
                sinergiaTotal += 0.5;
        }
    }
    // Como máximo cada jugador obtiene 1 de sinergia, por lo que se divide
    // entre el cuadrado de la cantidad de jugadores para obtener un número del
    // 0 al 1.
    sinergiaTotal /= pow(cromosoma.size(), 2.0);

    // Fitness como multiplicacion de sinergia y overall
    return (overallTotal * sinergiaTotal);
}

Cromosoma mutarCromosoma(Cromosoma cromosoma) {
    int n  = round(cromosoma.size() * tmutacion);

    // Elimina del mapa de índices por posicion a los jugadores del cromosoma
    // para no seleccionarlos nuevamente (*)
    vector<int> recuperar = cromosoma;
    for(int id: cromosoma) {
        recuperar.push_back(id);
        posicion_jugadores[jugadores[id].posicion].erase(id);
    }

    for(int i = 0; i < n; i++) {
        // selecciona aleatoriamente un índice j del cromosoma para reemplazar
        int j = rand() % cromosoma.size();
        string posicion = jugadores[cromosoma[j]].posicion;

        // Elige aleatoriamente de los jugadores con la posicion elegida
        int r;
        r = rand() % posicion_jugadores[posicion].size();
        auto it = posicion_jugadores[posicion].begin();
        for(int i = 0; i < r; i++) it = next(it);

        // Reemplazo en el cromosoma
        cromosoma[j] = *it;
        // Lo elimina del mapa de índices por posicion para no volver a
        // seleccionarlo (*)
        recuperar.push_back(*it);
        posicion_jugadores[posicion].erase(it);
    }

    // Añade nuevamente todos los jugadores al mapa de índices por posicion para
    // no alterarlo (*)
    for(int id: recuperar) {
        posicion_jugadores[jugadores[id].posicion].insert(id);
    }

    return cromosoma;
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
        //cout<<poblacion[i].size()<<endl;
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
        cout << jugadores[id].posicion << " - ID: " << id << " (Overall: "
            << jugadores[id].overall << ")";
        cout << jugadores[id].nacionalidad << " " << jugadores[id].club << endl;
    }
}
// Lectura de la data en csv
void leerJugadores() {
    string nombreArchivo = "C:/AA/TA_AA/data.csv";
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error abriendo archivo csv" << endl;
        exit(1);
    }
    int i=0, j=0;
    string linea, palabra, tmp;
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        bool mala_linea = false;

        // leer valores entre comas y limpieza
        vector<string> row;
        for(int i=0; i<5; i++){
            if(i==4)
                getline(ss, palabra, '\n');
            else
                getline(ss, palabra, ',');
            if((i==0 or i==2 or i==4) and palabra.empty()){
                mala_linea=true;
                break;
            }
            if(palabra.empty()) 
                row.push_back("NE");
            else
                row.push_back(palabra);
        }
        if(mala_linea){
            j++;
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
        posicion_jugadores.emplace(futbolista.posicion, set<int>());
        posicion_jugadores[futbolista.posicion].insert(jugadores.size());
        jugadores.push_back(futbolista);
        i++;
    }
    //cout<<i<<'-'<<j<<'-'<<i+j<<endl;
    archivo.close();
}
void cargarFormaciones(){
    string nombreArchivo = "C:/AA/TA_AA/Formaciones.txt";
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error abriendo archivo txt" << endl;
        exit(1);
    }
    int defensa,medioCam1,medioCam2,delan, llave;
    char c;
    string linea, pos, ss;
    while(1){
        archivo>>defensa;
        if(archivo.eof()) break;
        archivo>>c>>medioCam1>>c>>delan>>c;
        if(c=='-'){
            medioCam2=delan;
            archivo>>delan>>c;
            llave=defensa*1000+medioCam1*100+medioCam2*10+delan;
            medioCam1+=medioCam2;
        }else
            llave=defensa*100+medioCam1*10+delan;
        getline(archivo, linea);
        stringstream ss(linea);
        Formacion aux;
        for(int i=0; i<defensa; i++){
            getline(ss, pos, ',');
            aux.Defensores.push_back(pos);
        }
        for(int i=0; i<medioCam1; i++){
            getline(ss, pos, ',');
            aux.MedioCampistas.push_back(pos);
        }
        for(int i=0; i<delan; i++){
            if(i==delan-1)
                getline(ss, pos, '\n');
            else
                getline(ss, pos, ',');
            aux.Delanteros.push_back(pos);
        }
        formac.emplace(llave, aux);
    }
}
int main() {
    srand(time(0));

    // Leer Data
    leerJugadores();
    cargarFormaciones();

    // Generaciones y población predefinidas
    int generaciones = 100;
    int tamPoblacion = 20;
    tmutacion = 0.5;
    //Las formaciones disponibles son
    //4-4-2 Defensores(LB,RB,CB,CB), Mediocampistas(LM,RM,CM,CM), Delanteros(ST,ST)
    //4-3-3 Defensores(LB,RB,CB,CB), Mediocampistas(CDM,LCM,RCM), Delanteros(LW,RW,ST)
    //3-5-2 Defensores(LCB,CB,RCB), Mediocampistas(LWB,RWB,LCM,RCM), Delanteros(ST,ST)
    //4-2-3-1 Defensores(LB,RB,CB,CB), Mediocampistas(CDM,CDM,LAM,CAM,RAM), Delanteros(ST)
    formacionDeseada=442;//4-4-2
    // Ejecutar algoritmo genético
    Cromosoma mejorEquipo = algoritmoGenetico(generaciones, tamPoblacion);

    // Imprimir el mejor equipo
    cout << "Mejor equipo:" << endl;
    imprimirEquipo(mejorEquipo);
    
    return 0;
}
