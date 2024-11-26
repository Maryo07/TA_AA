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
};

const double T_MUTACION = 0.5;
const double T_CASAMIENTO = 0.5;
const uint32_t TAM_POBLACION = 20;
const uint32_t NUM_GENERACIONES = 100;

std::vector<Futbolista> auxJugadores;

typedef std::vector<int> Cromosoma;

bool valido(Cromosoma &cromosoma) {
	Cromosoma c = cromosoma;
	sort(c.begin(), c.end());
	// Si existen repetidos devuelve falso
	if (adjacent_find(c.begin(), c.end()) != c.end())
		return false;
	else
		return true;
}

void poblacionInicial(std::vector<Cromosoma> &poblacion, std::map<std::string, int> &formacion,
                      std::map<std::string, std::set<int>> &posicionJugadores, std::mt19937 &gen) {
	while (poblacion.size() < TAM_POBLACION) {
		Cromosoma cromosoma;
		// Itera entre todas las posiciones de la formacion
		for (auto &par : formacion) {
			// Repite por la cantidad de jugadores necesarias en la
			// posicion elegida
			for (int i = 0; i < par.second; i++) {
				// Elige aleatoriamente de los jugadores con la posicion elegida
				std::uniform_int_distribution<int> distribution(
					0, posicionJugadores[par.first].size() - 1);
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

double fitness(const Cromosoma &cromosoma, const std::vector<Futbolista> &jugadores) {
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
			if (jugadores[cromosoma[i]].club == jugadores[cromosoma[j]].club)
				f += (0.5 * jugadores[cromosoma[i]].overall);

			if (jugadores[cromosoma[i]].nacionalidad == jugadores[cromosoma[j]].nacionalidad)
				f += (0.5 * jugadores[cromosoma[i]].overall);

			f += jugadores[cromosoma[i]].overall;
		}
	}
	// Como máximo cada jugador obtiene 1 de sinergia, por lo que se divide
	// entre el cuadrado de la cantidad de jugadores para obtener un número del
	// 0 al 1.
	// f /= pow(cromosoma.size(), 2.0);

	// Fitness como multiplicacion de sinergia y overall
	return (f);
}

Cromosoma mutarCromosoma(Cromosoma cromosoma, std::map<std::string, int> &formacion,
                         std::vector<Futbolista> &jugadores,
                         std::map<std::string, std::set<int>> &posicionJugadores,
                         std::mt19937 &gen) {
	int n = round(cromosoma.size() * T_MUTACION);

	for (int i = 0; i < n; i++) {
		// selecciona aleatoriamente un índice j del cromosoma para reemplazar
		std::uniform_int_distribution<int> distribution1(0, cromosoma.size() - 1);
		int j = distribution1(gen);
		std::string posicion = jugadores[cromosoma[j]].posicion;

		// Elige aleatoriamente de los jugadores con la posicion elegida
		std::uniform_int_distribution<int> distribution2(0, posicionJugadores[posicion].size() - 1);
		int r = distribution2(gen);
		auto it = posicionJugadores[posicion].begin();
		for (int i = 0; i < r; i++)
			it++;

		// Reemplazo en el cromosoma
		cromosoma[j] = *it;
	}

	return cromosoma;
}

Cromosoma cruzarCromosomas(const Cromosoma &padre1, const Cromosoma &padre2, std::mt19937 &gen) {
	Cromosoma hijo = padre1;
	std::uniform_int_distribution<int> distribution(0, hijo.size() - 1);
	int puntoCorte = distribution(gen);
	for (int i = puntoCorte; i < hijo.size(); i++) {
		hijo[i] = padre2[i];
	}
	return hijo;
}

void mutarPadres(std::vector<Cromosoma> &poblacion, std::vector<Cromosoma> &padres,
                 std::map<std::string, int> &formacion, std::vector<Futbolista> &jugadores,
                 std::map<std::string, std::set<int>> &posicionJugadores, std::mt19937 &gen) {
	for (int i = 0; i < padres.size(); i++) {
		Cromosoma mutado = mutarCromosoma(padres[i], formacion, jugadores, posicionJugadores, gen);
		if (valido(mutado))
			poblacion.push_back(mutado);
	}
}

void generarHijos(std::vector<Cromosoma> &poblacion, const std::vector<Cromosoma> &padres,
                  std::mt19937 &gen) {
	for (int i = 0; i < padres.size(); i++)
		for (int j = 0; j < padres.size(); j++)
			if (i != j) {
				Cromosoma hijo = cruzarCromosomas(padres[i], padres[j], gen);
				if (valido(hijo))
					poblacion.push_back(hijo);
			}
}

void generarPadres(const std::vector<Cromosoma> &poblacion, std::vector<Cromosoma> &padres,
                   std::vector<Futbolista> &jugadores, std::mt19937 &gen) {
	std::vector<Cromosoma> ruleta = poblacion;
	double fitnessTotal = 0.0;
	for (Cromosoma cromosoma : poblacion)
		fitnessTotal += fitness(cromosoma, jugadores);

	// Giramos la ruleta
	for (int i = 0; i < round(poblacion.size() * T_CASAMIENTO); i++) {
		// elegimos un double aleatorio de 0 a fitnessTotal
		std::uniform_real_distribution<double> distribution(0, fitnessTotal);
		double random = distribution(gen);
		double fitnessActual = 0.0;
		for (int j = 0; j < ruleta.size(); j++) {
			// Voy sumando fitness por cada elemento de la ruleta
			fitnessActual += fitness(ruleta[j], jugadores);
			// Si mi aleatorio es menor o igual que el fitness actual entonces
			// tomo el elemento actual, lo guardo en los padres y lo quito de la
			// ruleta. El fitness total habŕa cambiado también.
			if (random <= fitnessActual) {
				padres.push_back(ruleta[j]);
				fitnessTotal -= fitness(ruleta[j], jugadores);
				ruleta.erase(ruleta.begin() + j);
				break;
			}
		}
	}
}

bool cmp(const Cromosoma &a, const Cromosoma &b) {
	return fitness(a, auxJugadores) > fitness(b, auxJugadores);
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

void reducirPoblacion(std::vector<Cromosoma> &poblacion, std::vector<Futbolista> &jugadores) {
	auxJugadores = jugadores;
	std::sort(poblacion.begin(), poblacion.end(), cmp);

	if (poblacion.size() > TAM_POBLACION)
		poblacion.erase(poblacion.begin() + TAM_POBLACION, poblacion.end());
}

Cromosoma algoritmoGenetico(std::map<std::string, int> &formacion,
                            std::vector<Futbolista> &jugadores,
                            std::map<std::string, std::set<int>> &posicionJugadores,
                            std::mt19937 &gen) {
	std::vector<Cromosoma> poblacion, padres;
	poblacionInicial(poblacion, formacion, posicionJugadores, gen);

	for (int _ = 0; _ < NUM_GENERACIONES; _++) {
		std::vector<Cromosoma> padres;
		generarPadres(poblacion, padres, jugadores, gen);
		generarHijos(poblacion, padres, gen);
		mutarPadres(poblacion, padres, formacion, jugadores, posicionJugadores, gen);
		eliminarDuplicados(poblacion);
		reducirPoblacion(poblacion, jugadores);
	}

	return poblacion[0];
}

void imprimirEquipo(const Cromosoma &equipo, std::vector<Futbolista> &jugadores) {
	for (int id : equipo) {
		std::cout << jugadores[id].posicion << " - : " << jugadores[id].nombre
				  << " (Overall: " << jugadores[id].overall << ")"
				  << " (Nacionalidad: " << jugadores[id].nacionalidad << ")"
				  << " (Club: " << jugadores[id].club << ")" << std::endl;
	}
}

void leerJugadores(std::vector<Futbolista> &jugadores,
                   std::map<std::string, std::set<int>> &posicionJugadores) {
	std::string nombreArchivo = "data.csv";
	std::ifstream archivo(nombreArchivo);
	if (!archivo.is_open()) {
		std::cerr << "Error abriendo el archivo " << nombreArchivo << std::endl;
		exit(1);
	}

	std::string linea, palabra, tmp;
	while (getline(archivo, linea)) {
		std::stringstream ss(linea);
		bool mala_linea = false;

		// leer valores entre comas y limpieza
		std::vector<std::string> row;
		for (int i = 0; i < 5; i++) {
			if (i == 4)
				getline(ss, palabra, ENDL);
			else
				getline(ss, palabra, ',');

			if (palabra.empty()) {
				mala_linea = true;
				break;
			}
			row.push_back(palabra);
		}
		if (mala_linea) {
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
		posicionJugadores.emplace(futbolista.posicion, std::set<int>());
		posicionJugadores[futbolista.posicion].insert(jugadores.size());
		jugadores.push_back(futbolista);
	}
	archivo.close();
}

void leerFormacion(std::map<std::string, int> &formacion) {
	std::string nombreArchivo = "Formaciones.txt";
	std::ifstream archivo(nombreArchivo);
	if (!archivo.is_open()) {
		std::cerr << "Error abriendo el archivo " << nombreArchivo << std::endl;
		exit(1);
	}

	std::string linea, palabra;
	getline(archivo, linea);

	std::stringstream ss(linea);
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
	// Restriccion de la formacion. Por cada posicion cuántos jugadores necesito.
	std::map<std::string, int> formacion;
	// Los jugadores por índice.
	std::vector<Futbolista> jugadores;
	// Índice de los jugadores por posicion.
	std::map<std::string, std::set<int>> posicionJugadores;
	leerJugadores(jugadores, posicionJugadores);
	leerFormacion(formacion);

	std::random_device rd;   // Semilla para el generador
	std::mt19937 gen(rd());  // Generador Mersenne Twister

	Cromosoma mejorEquipo;
	for (int i = 0; i < 10; i++) {
		Cromosoma cromosoma = algoritmoGenetico(formacion, jugadores, posicionJugadores, gen);
		if (mejorEquipo.size() == 0 ||
		    fitness(cromosoma, jugadores) > fitness(mejorEquipo, jugadores))
			mejorEquipo = cromosoma;
	}

	// Imprimir el mejor equipo
	std::cout << "Mejor equipo:" << std::endl;
	imprimirEquipo(mejorEquipo, jugadores);
	std::cout << "Fitness del equipo: " << fitness(mejorEquipo, jugadores) << std::endl;

	return 0;
}
