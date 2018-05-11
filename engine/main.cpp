#include "graph.hpp"
#include "../solver/Solver.hpp"
#include <iostream>


#define TIMEWAIT 3			// le temps d'attente minimum dans une gare desservie
#define SLOW 1				// nombre de train lent
#define FAST 0				// nombre de train rapide
#define TRAIN (SLOW+FAST)	// nombre de train
#define TIMESLOT 30			// nombre de minutes dans la plage horaire
#define STATION 3			// nombre de gare (voir la map)
#define TIMEDURATION 5		// duree maximal des voyages direct
#define TIMEWINDOW 5		// frequence des trains direct


int example_of_array[1];
Solver solver;
vec<Lit> literals;

int dans_gare[TRAIN][TIMESLOT][STATION];
int sur_voie[TRAIN][TIMESLOT][STATION][STATION];

void initVariables () {
	for (int i = 0; i < TRAIN; ++i)
	{
		for (int j = 0; j < TIMESLOT; ++j)
		{
			for (int k = 0; k < STATION; ++k)
			{
				for (int l = 0; l < STATION; ++l)
				{
					sur_voie[i][j][k][l] = solver.newVar();
				}
				dans_gare[i][j][k] = solver.newVar();
			}
		}
	}
}



// Helpers
bool voie_exists(Graph* map, int i, int k) {
	return map->get_duration(i, k) > 0;
}


// useless
void constraint_AB (Graph *map) {
	literals.clear();
	literals.push(Lit(example_of_array[0]));
	solver.addClause(literals);
}


void setupContrainte1(Graph *map){

}

vec<Lit> literals2;
void setupContrainte2(Graph *map){
	// Pas 2 trains sur la même voie
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (g1 == g2) {
				continue;
			}

			for (int i = 0; i < TIMESLOT; ++i)
			{
				for (int t1 = 0; t1 < TRAIN; ++t1)
				{
					for (int t2 = t1 + 1; t2 < TRAIN; ++t2)
					{
						solver.addBinary(~Lit(sur_voie[t1][i][g1][g2]), ~Lit(sur_voie[t2][i][g1][g2]));
					}
				}
			}
		}
	}
}

void setupContrainte3(Graph *map){

}

void setupContrainte4(Graph *map){

}

void setupContrainte5(Graph *map){

}

void setupContrainte6(Graph *map){

}

vec<Lit> contraintesImplicites2;
void setupContrainteImplicite1(Graph *map){
	// Tout train doit être quelque part
	for (int t = 0; t < TRAIN; ++t)
	{
		for (int i = 0; i < TIMESLOT; ++i)
		{
			contraintesImplicites2.clear();
			
			for (int g1 = 0; g1 < STATION; ++g1)
			{
				for (int g2 = 0; g2 < STATION; ++g2)
				{
					if (g1 != g2 && voie_exists(map, g1, g2)) {
						contraintesImplicites2.push(Lit(sur_voie[t][i][g1][g2]));	
					}
				}
				contraintesImplicites2.push(Lit(dans_gare[t][i][g1]));
			}
			solver.addClause(contraintesImplicites2);
		}
	}

}

void setupContrainteImplicite2(Graph *map){
	// Un train ne peut pas être dans 2 gares différentes au même moment
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = g1 + 1; g2 < STATION; ++g2)
		{
			for (int i = 0; i < TIMESLOT; ++i)
			{
				for (int t = 0; t < TRAIN; ++t)
				{
					solver.addBinary(~Lit(dans_gare[t][i][g1]), ~Lit(dans_gare[t][i][g2]));
				}
			}
		}
	}
}

void setupContrainteImplicite3(Graph *map){
	// Un train ne peut pas être sur 2 segment différents au même moment
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (g1 == g2 || !voie_exists(map, g1, g2)) {
				// Voie n'existe pas
				continue;
			}

			for (int g3 = 0; g3 < STATION; ++g3)
			{
				for (int g4 = 0; g4 < STATION; ++g4)
				{
					if (g3 == g4 || !voie_exists(map, g3, g4)) {
						// Voie n'existe pas
						continue;
					}

					if (g1 == g3 && g2 == g4) {
						// Meme voie
						continue;
					}


					for (int i = 0; i < TIMESLOT; ++i)
					{
						for (int t = 0; t < TRAIN; ++t)
						{
							solver.addBinary(~Lit(sur_voie[t][i][g1][g2]), ~Lit(sur_voie[t][i][g3][g4]));
						}
					}
				}
			}
		}
	}
}

void setupContrainteImplicite4(Graph *map){
	// Un train ne peut pas être sur un segment et une gare au même moment
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (g1 == g2 || !voie_exists(map, g1, g2)) {
				// Voie n'existe pas
				continue;
			}

			for (int g3 = 0; g3 < STATION; ++g3)
			{
				for (int g4 = 0; g4 < STATION; ++g4)
				{
					if (g3 == g4 || !voie_exists(map, g3, g4)) {
						// Voie n'existe pas
						continue;
					}

					if (g1 == g3 && g2 == g4) {
						// Meme voie
						continue;
					}


					for (int i = 0; i < TIMESLOT; ++i)
					{
						for (int t = 0; t < TRAIN; ++t)
						{
							solver.addBinary(~Lit(sur_voie[t][i][g1][g2]), ~Lit(sur_voie[t][i][g3][g4]));
						}
					}
				}
			}
		}
	}
}

void setupContrainteImplicite5(Graph *map){

}

void setupContrainteImplicite6(Graph *map){
	
}




int main() {
	// ---------- Map ---------- //

	Graph* map;
	map = Graph::parse((char *)"maps/slow-fast.txt"); // you can change the map file here !!
	map->print();
	assert(STATION == map->get_size());

	// ---------- Outputs ---------- //

	std::cout << "The number of slow trains is " << SLOW << std::endl;
	std::cout << "The number of fast trains is " << FAST << std::endl;

	for (int i = 0; i < STATION; ++i)
	{
		std::cout << "Station " << map->get_name(i) << " " << i << " with capacity " << map->get_capacity(i) << std::endl;
		for (int j = 0; j < STATION; ++j)
		{
			if (map->get_duration(i, j) > 0) {
				std::cout << "    Distance from " << i << " to " << j << " is " << map->get_duration(i, j) << std::endl;
			}
		}
	}

	// ---------- Variables ---------- //

	
	initVariables();



	// ---------- Constraints ---------- //

	setupContrainte1(map);
	setupContrainte2(map);
	setupContrainte3(map);
	setupContrainte4(map);
	setupContrainte5(map);
	setupContrainte6(map);

	std::cout << "Adding implicit clauses"  << std::endl;

	setupContrainteImplicite1(map);
	setupContrainteImplicite2(map);
	setupContrainteImplicite3(map);
	setupContrainteImplicite4(map);
	setupContrainteImplicite5(map);
	setupContrainteImplicite6(map);





	// ---------- Solver ---------- //

	printf("\n\n");
	solver.solve();
	printf("\n");



	// ---------- Printer ---------- //

	if (!solver.okay()) {
		printf("\nNO\n");
	}
	else {
		printf("\nYES\n");
	}



	// ---------- Delete ---------- //

	delete map;
	return EXIT_SUCCESS;
}
