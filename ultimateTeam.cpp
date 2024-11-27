#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#define ENDL (char)13

struct Futbolista {
	std::string nombre;
	std::string posicion;
	std::string club;
	std::string nacionalidad;
	int overall;
	int precio;
};

std::random_device rd;   // Semilla para el generador
std::mt19937 gen(rd());  // Generador Mersenne Twister

const double T_MUTACION = 0.1;
const double T_CASAMIENTO = 0.5;
const int TAM_POBLACION = 20;
const int NUM_GENERACIONES = 10000;
const int NUM_SEMILLAS = 1; 

// Restricciones.
// Por cada posicion cuántos jugadores son necesarios.
std::map<std::string, int> formacion;
// Precio máximo del equipo
int maxPrecio;
// Datos
// Jugadores ist por índice
std::vector<Futbolista> jugadores;
// Índice de los jugadores por posicion.
std::map<std::string, std::set<int>> posicionJugadores;

typedef std::vector<int> Cromosoma;

bool valido(const Cromosoma &cromosoma) {
	int precioTotal = 0;
	// Verifica que el valor del cromosoma sea menor al valor máximo
	for(int i = 0; i < cromosoma.size(); i++) {
		precioTotal += jugadores[cromosoma[i]].precio;
	}
	if(precioTotal >= maxPrecio) {
		//std::cout << "exceeds" << precioTotal << std::endl;
		return false;
	}
	// Si existen repetidos devuelve falso
	Cromosoma c = cromosoma;
	sort(c.begin(), c.end());
	if (adjacent_find(c.begin(), c.end()) != c.end()) {
		return false;
	} else {
		return true;
	}
}

void poblacionInicial(std::vector<Cromosoma> &poblacion) {
	while (poblacion.size() < TAM_POBLACION) {
		Cromosoma cromosoma;
		// Itera entre todas las posiciones de la formacion
		for (auto &par : formacion) {
			// Repite por la cantidad de jugadores necesarias en la
			// posicion elegida
			for (int i = 0; i < par.second; i++) {
				// Elige aleatoriamente de los jugadores con la posicion elegida
				std::uniform_int_distribution<int> distribution(
					0,
					posicionJugadores[par.first].size() - 1);
				int random = distribution(gen);
				auto it = posicionJugadores[par.first].begin();
				for (int j = 0; j < random; j++)
					it++;

				// Añade el jugador aleatorio al cromosoma
				cromosoma.push_back(*it);
			}
		}
		if (valido(cromosoma))
			poblacion.push_back(cromosoma);
	}
}

double sinergia(const Cromosoma &cromosoma, const int i) {
	double s = 0;
	// Si no tiene club entonces no tiene sinergia con nadie
	if(jugadores[cromosoma[i]].club.empty()) return -2;

	// Para cada jugador, añade 1 si encuentra a otro que coincida con su
	// pais y equipo. Y -1 si no coincide con ninguno.
	for (int j = 0; j < cromosoma.size(); j++) {
		if(i == j) {
			continue;
		}
		if (jugadores[cromosoma[i]].club == jugadores[cromosoma[j]].club
			and jugadores[cromosoma[i]].nacionalidad
				== jugadores[cromosoma[j]].nacionalidad) {
			s += 1;
		}
		if(jugadores[cromosoma[i]].club != jugadores[cromosoma[j]].club
			and jugadores[cromosoma[i]].nacionalidad
				!= jugadores[cromosoma[j]].nacionalidad) {
			s -= 1;
		}
	}
	// s dividido entre el máximo multiplicado por 2 (bonus)
	return (s*2.0) / (cromosoma.size() - 1);
}

double fitness(const Cromosoma &cromosoma) {
	// Suma del overall y bonus por sinergia
	double f = 0;
	for (int i = 0; i < cromosoma.size(); i++) {
		f += sinergia(cromosoma, i)
			+ jugadores[cromosoma[i]].overall;
	}
	return (f);
}

Cromosoma mutarCromosoma(Cromosoma cromosoma) {
	int n = round(cromosoma.size() * T_MUTACION);
	for (int i = 0; i < n; i++) {
		// selecciona aleatoriamente un índice j del cromosoma para reemplazar
		std::uniform_int_distribution<int> distribution1(
			0,
			cromosoma.size() - 1);
		int j = distribution1(gen);
		std::string posicion = jugadores[cromosoma[j]].posicion;

		// Elige aleatoriamente de los jugadores con la posicion elegida
		std::uniform_int_distribution<int> distribution2(
			0,
			posicionJugadores[posicion].size() - 1);
		int r = distribution2(gen);
		auto it = posicionJugadores[posicion].begin();
		for (int i = 0; i < r; i++) {
			it++;
		}

		// Reemplazo en el cromosoma
		cromosoma[j] = *it;
	}

	return cromosoma;
}

Cromosoma cruzarCromosomas(const Cromosoma &padre1, const Cromosoma &padre2) {
	Cromosoma hijo = padre1;
	std::uniform_int_distribution<int> distribution(0, hijo.size() - 1);
	int puntoCorte = distribution(gen);
	for (int i = puntoCorte; i < hijo.size(); i++) {
		hijo[i] = padre2[i];
	}
	return hijo;
}

void mutarPadres(
	std::vector<Cromosoma> &poblacion,
	const std::vector<Cromosoma> &padres
) {
	for (int i = 0; i < padres.size(); i++) {
		Cromosoma mutado = mutarCromosoma(padres[i]);
		if (valido(mutado)) {
			poblacion.push_back(mutado);	
		}
	}
}

void generarHijos(
	std::vector<Cromosoma> &poblacion,
	const std::vector<Cromosoma> &padres
) {
	for (int i = 0; i < padres.size(); i++) {
		for (int j = 0; j < padres.size(); j++) {
			if (i != j) {
				Cromosoma hijo = cruzarCromosomas(padres[i], padres[j]);
				if (valido(hijo)) {
					poblacion.push_back(hijo);
				}
			}
		}
	}
}

void generarPadres(
	const std::vector<Cromosoma> &poblacion,
	std::vector<Cromosoma> &padres
) {
	std::vector<Cromosoma> ruleta = poblacion;
	double fitnessTotal = 0.0;
	for (Cromosoma cromosoma : poblacion)
		fitnessTotal += fitness(cromosoma);

	// Giramos la ruleta
	for (int i = 0; i < round(poblacion.size() * T_CASAMIENTO); i++) {
		// elegimos un double aleatorio de 0 a fitnessTotal
		std::uniform_real_distribution<double> distribution(0, fitnessTotal);
		double random = distribution(gen);
		double fitnessActual = 0.0;
		for (int j = 0; j < ruleta.size(); j++) {
			// Voy sumando fitness por cada elemento de la ruleta
			fitnessActual += fitness(ruleta[j]);
			// Si mi aleatorio es menor o igual que el fitness actual entonces
			// toma el elemento actual, lo guarda en los padres y lo quita de la
			// ruleta. El fitness total habŕa cambiado también.
			if (random <= fitnessActual) {
				padres.push_back(ruleta[j]);
				fitnessTotal -= fitness(ruleta[j]);
				ruleta.erase(ruleta.begin() + j);
				break;
			}
		}
	}
}

bool cmp(const Cromosoma &a, const Cromosoma &b) {
	return fitness(a) > fitness(b);
}

void eliminarDuplicados(std::vector<std::vector<int>> &poblacion) {
	// Para igualar vectores es necesario ordernarlos primero, pero como
	// no queremos alterar la poblacion, los copiamos
	for (int i = 0; i < poblacion.size(); i++) {
		Cromosoma ivec = poblacion[i];
		std::sort(ivec.begin(), ivec.end());
		for (int j = i + 1; j < poblacion.size(); j++) {
			Cromosoma jvec = poblacion[j];
			std::sort(jvec.begin(), jvec.end());
			// Si el cromosoma en i es igual que el j, elimina el j
			if (ivec == jvec) {
				poblacion.erase(poblacion.begin() + j);
				j--;
			}
		}
	}
}

void reducirPoblacion(std::vector<Cromosoma> &poblacion) {
	std::sort(poblacion.begin(), poblacion.end(), cmp);
	if(poblacion.size() > TAM_POBLACION) {
		poblacion.erase(poblacion.begin() + TAM_POBLACION, poblacion.end());	
	}
}

Cromosoma algoritmoGenetico() {
	std::vector<Cromosoma> poblacion, padres;
	poblacionInicial(poblacion);

	double lastfitness = 0, firstfitness = 0.0, f = 0.0;
	for (int _ = 0; _ < NUM_GENERACIONES; _++) {
		std::vector<Cromosoma> padres;
		generarPadres(poblacion, padres);
		generarHijos(poblacion, padres);
		mutarPadres(poblacion, padres);
		eliminarDuplicados(poblacion);
		reducirPoblacion(poblacion);
		f = fitness(poblacion[0]);
		if(f != lastfitness) {
			std::cout << 1.0 - (firstfitness / f) << std::endl;
			if(_ == 0) firstfitness = f;
			lastfitness = f;
		}
	}

	return poblacion[0];
}

void imprimirEquipo(const Cromosoma &cromosoma) {
	int precioTotal = 0.0;
	for(int i = 0; i < cromosoma.size(); i++) {
		std::cout
			<< jugadores[cromosoma[i]].posicion << " - : "
			<< jugadores[cromosoma[i]].nombre << " (Overall: "
			<< jugadores[cromosoma[i]].overall;
		if(sinergia(cromosoma, i) >= 0) {
			std::cout << "+" << sinergia(cromosoma, i);
		} else {
			std::cout << sinergia(cromosoma, i);
		};
		std::cout
			<< ") (Nacionalidad: " << jugadores[cromosoma[i]].nacionalidad
			<< ") (Club: " << jugadores[cromosoma[i]].club
			<< ") (Precio: " << jugadores[cromosoma[i]].precio
			<< ")" << std::endl;
		precioTotal += jugadores[cromosoma[i]].precio;
	}
	std::cout << "Precio total: " << precioTotal << std::endl;
	std::cout << "Fitness del equipo: " << fitness(cromosoma) << std::endl;
}

void leerJugadores() {
	std::string nombreArchivo = "data.csv";
	std::ifstream archivo(nombreArchivo);
	if (!archivo.is_open()) {
		std::cerr << "Error abriendo el archivo " << nombreArchivo << std::endl;
		exit(1);
	}

	std::string linea, palabra, tmp;
	getline(archivo, linea); // primera linea de labels
	while (getline(archivo, linea)) {
		std::stringstream ss(linea);

		// leer valores entre comas y limpieza
		std::vector<std::string> row;
		for (int i = 0; i < 6; i++) {
			if (i == 5)
				getline(ss, palabra, ENDL);
			else
				getline(ss, palabra, ',');
			row.push_back(palabra);
		}

		Futbolista futbolista;
		futbolista.nombre = row[0];
		futbolista.nacionalidad = row[1];
		futbolista.overall = stoi(row[2]);
		futbolista.club = row[3];
		// conversion del valor
		if(row[4].back() == 'K') {
			row[4] = row[4].substr(3, row[4].size() - 4);
			futbolista.precio = stod(row[4]) * 100000;
		} else if(row[4].back() == 'M') {
			row[4] = row[4].substr(3, row[4].size() - 4);
			futbolista.precio = stod(row[4]) * 1000000;
		} else {
			futbolista.precio = 0;
		}
		futbolista.posicion = row[5];

		// añadir id del jugador a su posicion y el jugador al arreglo
		posicionJugadores.emplace(futbolista.posicion, std::set<int>());
		posicionJugadores[futbolista.posicion].insert(jugadores.size());
		jugadores.push_back(futbolista);
	}
	archivo.close();
}

void leerFormacionYPrecio() {
	std::string nombreArchivo = "FormacionYPrecio.txt";
	std::ifstream archivo(nombreArchivo);
	if (!archivo.is_open()) {
		std::cerr << "Error abriendo el archivo " << nombreArchivo << std::endl;
		exit(1);
	}
	// Lee linea
	std::string linea, palabra;
	getline(archivo, linea);
	std::stringstream ss(linea);

	// Lee precio
	getline(ss, palabra, ',');
	maxPrecio = stoi(palabra);

	// Lee formacion
	getline(ss, palabra, ',');
	int n = stoi(palabra);
	for (int i = 0; i < n; i++) {
		if (i == n - 1)
			getline(ss, palabra, ENDL);
		else
			getline(ss, palabra, ',');

		if (formacion.find(palabra) != formacion.end()) {
			formacion[palabra] = formacion[palabra] + 1;
		}
		else {
			formacion.emplace(palabra, 1);
		}
	}
}

int main() {
	leerJugadores();
	leerFormacionYPrecio();

	// Inicializa varias semillas y evalúa cuál es la mejor
	Cromosoma mejorCromosoma;
	for (int i = 0; i < NUM_SEMILLAS; i++) {
		Cromosoma cromosoma = algoritmoGenetico();
		if (mejorCromosoma.size() == 0 ||
		    fitness(cromosoma) > fitness(mejorCromosoma))
			mejorCromosoma = cromosoma;
	}

	std::cout << "Mejor equipo:" << std::endl;
	imprimirEquipo(mejorCromosoma);

	return 0;
}
