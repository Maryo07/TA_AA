#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
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

const double T_MUTACION = 0.2;
const double T_CASAMIENTO = 0.5;
const int TAM_POBLACION = 30;
const int NUM_GENERACIONES = 50000;
const int NUM_SEMILLAS = 1;

std::random_device rd;   // Semilla para el generador
std::mt19937 gen(rd());  // Generador Mersenne Twister

/******************  Restricciones  ************************/
// Por cada posición, cuántos futbolistas son necesarios.
std::map<std::string, int> formacion;

/***********************  Datos ****************************/
// Precio máximo del equipo
int maxPrecio;

// Futbolistas por índice
std::vector<Futbolista> jugadores;

// Índice de los futbolistas por posición y ordenados por precio ascendentemente
std::map<std::string, std::vector<int>> posicionJugadores;

typedef std::vector<int> Cromosoma;

bool Valido(const Cromosoma &cromosoma) {
	int precioTotal = 0;
	// Verifica que el precio del cromosoma sea menor al precio máximo
	for (int i = 0; i < cromosoma.size(); i++)
		precioTotal += jugadores[cromosoma[i]].precio;

	if (precioTotal >= maxPrecio)
		return false;

	// Si existen repetidos devuelve falso
	Cromosoma c = cromosoma;
	sort(c.begin(), c.end());
	if (adjacent_find(c.begin(), c.end()) != c.end())
		return false;
	else
		return true;
}

bool CompararJugadores(const int idA, const int idB) {
	return jugadores[idA].precio < jugadores[idB].precio;
}

bool CompararJugadorPresupuesto(const int presupuesto, const int id) {
	return presupuesto < jugadores[id].precio;
}

int FutbolistaAleatorio(const std::string &posicion, const int presupuesto) {
	auto end =
		std::upper_bound(posicionJugadores[posicion].begin(), posicionJugadores[posicion].end(),
	                     presupuesto, CompararJugadorPresupuesto);
	int n = (end - posicionJugadores[posicion].begin());
	if (n == 0)
		return -1;

	std::uniform_int_distribution<int> distribution(0, n);
	int random = distribution(gen);
	return posicionJugadores[posicion][random];
}

void PoblacionInicial(std::vector<Cromosoma> &poblacion) {
	while (poblacion.size() < TAM_POBLACION) {
		int presupuesto = maxPrecio;
		Cromosoma cromosoma;
		bool valido = true;
		// Itera entre todas las posiciones elegidas de la formación
		for (auto &par : formacion) {
			// Repite por la cantidad de futbolistas necesarios en la
			// posición elegida
			for (int i = 0; i < par.second; i++) {
				// Elige aleatoriamente de los futbolistas con la posición
				// elegida y solamente los jugadores para los cuales existe
				// presupuesto (si no existen para el presupuesto devuelve -1)
				int id = FutbolistaAleatorio(par.first, presupuesto);
				if (id == -1) {
					valido = false;
					break;
				}
				cromosoma.push_back(id);
				presupuesto -= jugadores[id].precio;
			}
			if (!valido)
				break;
		}
		if (valido and Valido(cromosoma))
			poblacion.push_back(cromosoma);
	}
}

double BonusSinergia(const Cromosoma &cromosoma, const int i) {
	double s = 0;

	// Para cada futbolista, añade 1 si encuentra a otro que coincida con su
	// país y equipo. Y -1 si no coincide con ninguno.
	for (int j = 0; j < cromosoma.size(); j++) {
		if (i == j)
			continue;

		// Existen futbolistas sin equipo, y no significa que comparten equipo
		// con otros futbolistas sin equipo.
		if (!jugadores[cromosoma[i]].club.empty() &&
		    jugadores[cromosoma[i]].club == jugadores[cromosoma[j]].club &&
		    jugadores[cromosoma[i]].nacionalidad == jugadores[cromosoma[j]].nacionalidad) {
			s += 1;
		}

		if ((jugadores[cromosoma[i]].club.empty() ||
		     jugadores[cromosoma[i]].club != jugadores[cromosoma[j]].club) &&
		    jugadores[cromosoma[i]].nacionalidad != jugadores[cromosoma[j]].nacionalidad) {
			s -= 1;
		}
	}
	// s dividido entre el máximo multiplicado por 2 (bonus)
	return (s * 2.0) / (cromosoma.size() - 1);
}

double Fitness(const Cromosoma &cromosoma) {
	double f = 0;
	for (int i = 0; i < cromosoma.size(); i++)
		f += BonusSinergia(cromosoma, i) + jugadores[cromosoma[i]].overall;

	return f;
}

Cromosoma MutarCromosoma(Cromosoma cromosoma) {
	int n = round(cromosoma.size() * T_MUTACION);
	int presupuesto = maxPrecio;
	for (int i = 0; i < cromosoma.size(); i++)
		presupuesto -= jugadores[cromosoma[i]].precio;

	for (int i = 0; i < n; i++) {
		std::uniform_int_distribution<int> distribution(0, cromosoma.size() - 1);
		int j = distribution(gen);
		std::string posicion = jugadores[cromosoma[j]].posicion;
		presupuesto += jugadores[cromosoma[j]].precio;

		// Nunca devuelve -1 pues puede volver a elegir el futbolista que se va
		// a reemplazar
		int id = FutbolistaAleatorio(posicion, presupuesto);
		cromosoma[j] = id;
		presupuesto -= jugadores[id].precio;
	}

	return cromosoma;
}

Cromosoma CruzarCromosomas(const Cromosoma &padre1, const Cromosoma &padre2) {
	Cromosoma hijo = padre1;
	std::uniform_int_distribution<int> distribution(0, hijo.size() - 1);
	int puntoCorte = distribution(gen);

	for (int i = puntoCorte; i < hijo.size(); i++)
		hijo[i] = padre2[i];

	return hijo;
}

void MutarPadres(std::vector<Cromosoma> &poblacion, const std::vector<Cromosoma> &padres) {
	for (int i = 0; i < padres.size(); i++) {
		Cromosoma mutado = MutarCromosoma(padres[i]);
		if (Valido(mutado))
			poblacion.push_back(mutado);
	}
}

void GenerarHijos(std::vector<Cromosoma> &poblacion, const std::vector<Cromosoma> &padres) {
	for (int i = 0; i < padres.size(); i++) {
		for (int j = 0; j < padres.size(); j++) {
			if (i != j) {
				Cromosoma hijo = CruzarCromosomas(padres[i], padres[j]);
				if (Valido(hijo))
					poblacion.push_back(hijo);
			}
		}
	}
}

void GenerarPadres(const std::vector<Cromosoma> &poblacion, std::vector<Cromosoma> &padres) {
	std::list<Cromosoma> ruleta(poblacion.begin(), poblacion.end());
	double fitnessTotal = 0.0;
	for (Cromosoma cromosoma : poblacion)
		fitnessTotal += Fitness(cromosoma);

	// Giramos la ruleta
	for (int i = 0; i < round(poblacion.size() * T_CASAMIENTO); i++) {
		std::uniform_real_distribution<double> distribution(0, fitnessTotal);
		double random = distribution(gen);
		double fitnessActual = 0.0;
		for (auto it = ruleta.begin(); it != ruleta.end(); it++) {
			// Va sumando fitness por cada elemento de la ruleta
			fitnessActual += Fitness(*it);
			// Si random es menor o igual que el fitness actual entonces toma
			// el elemento actual, lo guarda en los padres y lo quita de la
			// ruleta. El fitnessTotal habŕa cambiado también.
			if (random <= fitnessActual) {
				padres.push_back(*it);
				fitnessTotal -= Fitness(*it);
				ruleta.erase(it);
				break;
			}
		}
	}
}

void EliminarDuplicados(std::vector<std::vector<int>> &poblacion) {
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

bool CompararCromosomas(const Cromosoma &a, const Cromosoma &b) {
	return Fitness(a) > Fitness(b);
}

void ReducirPoblacion(std::vector<Cromosoma> &poblacion) {
	std::sort(poblacion.begin(), poblacion.end(), CompararCromosomas);

	if (poblacion.size() > TAM_POBLACION)
		poblacion.erase(poblacion.begin() + TAM_POBLACION, poblacion.end());
}

Cromosoma AlgoritmoGenetico() {
	std::vector<Cromosoma> poblacion, padres;
	PoblacionInicial(poblacion);

	double fitnessAnterior = 0, primerFitness = 0.0, f = 0.0;
	int cuentaNoMejora = 0;
	for (int i = 0; i < NUM_GENERACIONES; i++) {
		std::vector<Cromosoma> padres;
		GenerarPadres(poblacion, padres);
		GenerarHijos(poblacion, padres);
		MutarPadres(poblacion, padres);
		EliminarDuplicados(poblacion);
		ReducirPoblacion(poblacion);
		f = Fitness(poblacion[0]);
		// Imprime cuando existe una mejora
		if (f != fitnessAnterior) {
			std::cout << 1.0 - (primerFitness / f) << std::endl;
			if (i == 0)
				primerFitness = f;

			fitnessAnterior = f;
			cuentaNoMejora = 0;
		}
		else {
			cuentaNoMejora++;
			// Encontró óptimo local
			if (cuentaNoMejora >= 1000)
				break;
		}
	}

	return poblacion[0];
}

void imprimirPrecio(int precio) {
	if (precio <= 0)
		return;

	imprimirPrecio(precio / 1000);
	if (precio % 1000 == 0)
		std::cout << " 000";
	else
		std::cout << " " << precio % 1000;
}

void ImprimirEquipo(const Cromosoma &cromosoma) {
	int precioTotal = 0.0;

	std::cout << std::setprecision(2) << std::fixed;

	std::cout << std::setfill('=') << std::setw(110) << "" << std::setfill(' ') << std::endl
			  << std::setw(60) << "MEJOR EQUIPO " << std::endl
			  << std::setfill('=') << std::setw(110) << "" << std::setfill(' ') << std::endl;

	std::cout << "POSICIÓN" << std::setw(13) << "JUGADOR" << std::setw(15) << "OVERALL"
			  << std::setw(17) << "NACIONALIDAD" << std::setw(16) << "CLUB" << std::setw(27)
			  << "PRECIO" << std::endl;
	std::cout << std::setfill('-') << std::setw(110) << "" << std::setfill(' ') << std::endl;

	for (int i = 0; i < cromosoma.size(); i++) {
		std::cout << std::setw(3) << "" << jugadores[cromosoma[i]].posicion << std::setw(6) << ""
				  << std::left << std::setw(18) << jugadores[cromosoma[i]].nombre
				  << jugadores[cromosoma[i]].overall;

		if (BonusSinergia(cromosoma, i) >= 0)
			std::cout << "+" << BonusSinergia(cromosoma, i);

		else
			std::cout << BonusSinergia(cromosoma, i);

		std::cout << std::setw(8) << "" << std::setw(13) << jugadores[cromosoma[i]].nacionalidad
				  << std::setw(28) << jugadores[cromosoma[i]].club;
		imprimirPrecio(jugadores[cromosoma[i]].precio);
		if (jugadores[cromosoma[i]].precio == 0)
			std::cout << std::setw(8) << "" << "0";
		std::cout << " €" << std::endl;
		precioTotal += jugadores[cromosoma[i]].precio;
	}
	std::cout << "Precio total:";
	imprimirPrecio(precioTotal);
	std::cout << " €" << std::endl;
	std::cout << "Fitness del equipo: " << Fitness(cromosoma) << std::endl;
}

void LeerFutbolistas() {
	std::string nombreArchivo = "data.csv";
	std::ifstream archivo(nombreArchivo);
	if (!archivo.is_open()) {
		std::cerr << "Error abriendo el archivo " << nombreArchivo << std::endl;
		exit(1);
	}

	std::string linea, palabra, tmp;
	getline(archivo, linea);  // Primera linea de labels
	while (getline(archivo, linea)) {
		std::stringstream ss(linea);

		// Leer valores entre comas y limpieza
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
		// Conversión del valor
		if (row[4].back() == 'K') {
			row[4] = row[4].substr(3, row[4].size() - 4);
			futbolista.precio = stod(row[4]) * 100000;
		}
		else if (row[4].back() == 'M') {
			row[4] = row[4].substr(3, row[4].size() - 4);
			futbolista.precio = stod(row[4]) * 1000000;
		}
		else {
			futbolista.precio = 0;
		}
		futbolista.posicion = row[5];

		// Añadir id del futbolista a su posición y el futbolista al arreglo
		posicionJugadores.emplace(futbolista.posicion, std::vector<int>());
		posicionJugadores[futbolista.posicion].push_back(jugadores.size());
		jugadores.push_back(futbolista);
	}

	// Ordena cada lista de futbolistas por su precio de forma ascendente
	for (auto par : posicionJugadores) {
		sort(posicionJugadores[par.first].begin(), posicionJugadores[par.first].end(),
		     CompararJugadores);
	}
}

void LeerFormacionYPrecio() {
	std::string nombreArchivo = "FormacionYPrecio.txt";
	std::ifstream archivo(nombreArchivo);
	if (!archivo.is_open()) {
		std::cerr << "Error abriendo el archivo " << nombreArchivo << std::endl;
		exit(1);
	}

	// Lee línea
	std::string linea, palabra;
	getline(archivo, linea);
	std::stringstream ss(linea);

	// Lee precio
	getline(ss, palabra, ',');
	maxPrecio = stoi(palabra);

	// Lee formación
	getline(ss, palabra, ',');
	int n = stoi(palabra);
	for (int i = 0; i < n; i++) {
		if (i == n - 1)
			getline(ss, palabra, ENDL);
		else
			getline(ss, palabra, ',');

		if (formacion.find(palabra) != formacion.end())
			formacion[palabra] = formacion[palabra] + 1;
		else
			formacion.emplace(palabra, 1);
	}
}

int main() {
	LeerFutbolistas();
	LeerFormacionYPrecio();

	// Inicializa varias semillas y evalúa cuál es la mejor
	Cromosoma mejorCromosoma;
	for (int i = 0; i < NUM_SEMILLAS; i++) {
		Cromosoma cromosoma = AlgoritmoGenetico();
		if (mejorCromosoma.size() == 0 || Fitness(cromosoma) > Fitness(mejorCromosoma))
			mejorCromosoma = cromosoma;
	}

	ImprimirEquipo(mejorCromosoma);

	return 0;
}
