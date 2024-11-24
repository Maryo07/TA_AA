#include <iostream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#define NITERACIONES 5
#define NIND 5
#define Tseleccion 0.5
#define Pcasamiento 0.5
#define Tmutacion 0.2


using namespace std;

struct Cursos{
    char *id;//id del curso
    char *nombre; //nombre del curso
    int numSesiones; //cant de sesiones por semana 
    int horasDeSesion;// horas que tiene cada sesion
    int cantAlumnos; //cant de alumnos por horario
    //vector<string> profes; //id de profesores que enseñan ese curso
};

struct Profesor{
    char *id;
    char *nombre;
    vector<char *> cursos;//id de los cursos que puede dictar
};

struct Salon{
    char *id;
    int capacidad;
};

struct Clase{
    char *idCurso;
    char *idProfesor;
    char *idSalon;
    int horaInicio;
    int horaFin;
};

vector<Profesor> profesores;
vector<Salon> salones;

char *leerCadExacta(ifstream &arch, char lim){
    char buffer[100], *cad;
    arch.getline(buffer, 100, lim);
    if(arch.eof()) return nullptr;
    cad= new char[strlen(buffer)+1]{};
    strcpy(cad, buffer);
}
void cargarCursos(vector<Cursos> &cursos){
    ifstream arch("Cursos.txt", ios::in);
    if(not arch.is_open()){
        cout<<"No se pudo abrir Cursos.txt";
        exit(1);
    }
    Cursos aux;
    char *cad, c;
    while(1){
        cad=leerCadExacta(arch, ',');
        if(arch.eof()) break;
        aux.id=cad;
        cad=leerCadExacta(arch, ',');
        aux.nombre=cad;
        arch>>aux.numSesiones>>c>>aux.horasDeSesion>>c>>aux.cantAlumnos>>c;
        cursos.push_back(aux);
    }
}
void cargarProfesores(){
    ifstream arch("Profesores.txt", ios::in);
    if(not arch.is_open()){
        cout<<"No se pudo abrir Profesores.txt";
        exit(1);
    }
    Profesor aux;
    char *cad, c;
    int n;
    while(1){
        cad=leerCadExacta(arch, ',');
        if(arch.eof()) break;
        aux.id=cad;
        cad=leerCadExacta(arch, ',');
        aux.nombre=cad;
        arch>>n>>c;
        for(int i=0; i<n; i++){
            if(i==n-1)
                cad=leerCadExacta(arch, '\n');
            else
                cad=leerCadExacta(arch, ',');
            aux.cursos.push_back(cad);
        }
        profesores.push_back(aux);
    }
}
void cargarSalones(){
    ifstream arch("Salones.txt", ios::in);
    if(not arch.is_open()){
        cout<<"No se pudo abrir Salones.txt";
        exit(1);
    }
    Salon aux;
    char *cad;
    while(1){
        cad=leerCadExacta(arch, ',');
        if(arch.eof()) break;
        aux.id=cad;
        arch>>aux.capacidad;
        arch.get();
        salones.push_back(aux);
    }
}

bool aberracion(const vector<vector<Clase>>& horario, const vector<Cursos>& cursos) {
    // Iteramos sobre cada día de la semana
    for (int dia = 0; dia < 6; dia++) {
        // Iteramos sobre las clases programadas para ese día
        for (int i = 0; i < horario[dia].size(); i++) {
            Clase claseActual = horario[dia][i];

            // Verificar que el salón tenga capacidad suficiente
            bool salonCapacidadSuficiente = false;
            for (const auto& salon : salones) {
                if (strcmp(salon.id, claseActual.idSalon) == 0) {
                    if (salon.capacidad >= cursos[claseActual.idCurso].cantAlumnos) {//agregar funcion buscar posCurso
                        salonCapacidadSuficiente = true;
                        break;
                    }
                }
            }
            if (!salonCapacidadSuficiente) {
                return true;  // El salón no tiene suficiente capacidad
            }

            // Verificar que el salón no esté ocupado en el mismo horario
            for (int j = i + 1; j < horario[dia].size(); j++) {
                if (strcmp(horario[dia][j].idSalon, claseActual.idSalon) == 0 &&
                    (claseActual.horaInicio < horario[dia][j].horaFin &&
                    horario[dia][j].horaInicio < claseActual.horaFin)) {//esto está mal
                    return true;  // El salón está ocupado a la misma hora
                }
            }

            // Verificar que el profesor esté capacitado para dictar el curso
            bool profesorApto = false;
            for (const auto& curso : cursos) {
                if (strcmp(curso.id, claseActual.idCurso) == 0) {
                    for (const auto& profesor : profesores) {
                        if (find(profesor.cursos.begin(), profesor.cursos.end(), claseActual.idCurso) != profesor.cursos.end()) {
                            profesorApto = true;
                            break;
                        }
                    }
                    if (profesorApto) break;
                }
            }
            if (!profesorApto) {
                return true;  // El profesor no está capacitado para este curso
            }

            // Verificar que el profesor no esté dictando dos clases a la vez
            for (int j = i + 1; j < horario[dia].size(); j++) {
                if (strcmp(horario[dia][j].idProfesor, claseActual.idProfesor) == 0 &&
                    (claseActual.horaInicio < horario[dia][j].horaFin &&
                    horario[dia][j].horaInicio < claseActual.horaFin)) {//esto está mal
                    return true;  // El profesor está dictando dos clases a la vez
                }
            }
        }
    }

    // Si no se encontró ninguna aberración
    return false;
}

void generapoblacioninicial(vector<vector<vector<Clase>>> &poblacion, vector<Cursos> cursos){
    int cont=0;
    
    while(cont<NIND){
        vector<vector<Clase>> horario (6, vector<Clase>{});
        for(int i=0;i<cursos.size();i++){
            Clase aux;
            aux.idCurso=cursos[i].id;
            int indProf=rand()%profesores.size();
            int indSalon=rand()%salones.size();
            aux.idProfesor=profesores[indProf].id;
            aux.idSalon=salones[indSalon].id;
            for(int k=0; k<cursos[i].numSesiones; k++){
                int dia=rand()%6;
                int cantHoras=cursos[i].horasDeSesion;
                int hora=rand()%15;
                while(hora+cantHoras>=15)
                    hora=rand()%15;
                aux.horaInicio=hora+7;
                aux.horaFin=hora+7+cantHoras;
                horario[dia].push_back(aux);
            }
        }
        //el salon no puede estar ocupado a la misma hora y debe tener la capacidad para todos los alumnos
        //el profesor debe estar capacitado para dictar ese curso y solo puede dictar un curso el mismo dia durante determinado intervalo de tiempo
        if(not aberracion(horario,cursos)){
            poblacion.push_back(horario);
            cont++;
        }
    }
}
void seleccion(vector<vector<vector<Clase>>>&padres, vector<vector<vector<Clase>>>poblacion, vector<Cursos> cursos){
    int ruleta[100]{-1};
    vector<int>supervivencia;
    calculasupervivencia(poblacion,supervivencia,cursos);
    cargaruleta(supervivencia,ruleta);
    int nseleccionados= poblacion.size()*Tseleccion;        
    for(int i=0;i<nseleccionados;i++){
        int ind=rand()%100;
        if(ruleta[ind]>-1)
            padres.push_back(poblacion[ruleta[ind]]);
            
    } 

}
void solve(vector<Cursos> cursos){
    int cont=0;
    vector<vector<vector<Clase>>> poblacion;
    srand(time(NULL));
    generapoblacioninicial(poblacion,cursos);
    //muestrapoblacion(poblacion,paq);    
    cout << endl<<endl;
    while(1){
        vector<vector<int>> padres;
        seleccion(padres,poblacion,cursos);
        //muestrapoblacion(padres,paq);
        //casamiento(padres,poblacion,paq,peso);
        //cout << endl<<endl;
        //inversion(poblacion,padres,paq,peso);
        //mutacion(poblacion,padres,paq,peso);
        //muestrapoblacion(poblacion,paq);    
        //cout << endl<<endl;
        //eliminaaberraciones(poblacion,paq);
        //muestrapoblacion(poblacion,paq);    
        //cout << endl<<endl;
        //generarpoblacion(poblacion,paq,peso);
        //muestrapoblacion(poblacion,paq); 
        //muestramejor(poblacion,paq,peso);
        //cout << endl<<endl;
        cont++;
        if(cont==NITERACIONES) break;
    }
}
int main(int argc, char** argv) {
    vector<Cursos> cursos;
    cargarCursos(cursos);
    cargarProfesores();
    cargarSalones();
    
    solve(cursos);

    return 0;
}