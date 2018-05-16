#include "graph.hpp"
#include "../solver/Solver.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

// Slow Fast Map

#define TIMEWAIT 3			// le temps d'attente minimum dans une gare desservie
#define SLOW 2			// nombre de train lent
#define FAST 1				// nombre de train rapide
#define TRAIN (SLOW+FAST)	// nombre de train
#define TIMESLOT 26			// nombre de minutes dans la plage horaire
#define STATION 3			// nombre de gare (voir la map)
#define TRAVELDURATION 3	// duree maximal des voyages direct
#define TIMEWINDOW 10		// frequence des trains direct
#define MAPFILE "maps/slow-fast.txt"


// Cycle Map
/*
#define TIMEWAIT 3
#define SLOW 1
#define FAST 0
#define TRAIN (SLOW+FAST)
#define TIMESLOT 26
#define STATION 4
#define TRAVELDURATION 12
#define TIMEWINDOW 14
#define MAPFILE "maps/cycle.txt"
*/


int example_of_array[1];
Solver solver;
vec<Lit> literals;

int dans_gare[TRAIN][TIMESLOT][STATION];
int sur_voie[TRAIN][TIMESLOT][STATION][STATION];
int fait_trajet[TRAIN][TIMESLOT][STATION][TIMESLOT][STATION];

void initVariables () {
	for (int t = 0; t < TRAIN; ++t)
	{
		for (int i1 = 0; i1 < TIMESLOT; ++i1)
		{
			for (int g1 = 0; g1 < STATION; ++g1)
			{
				for (int g2 = 0; g2 < STATION; ++g2)
				{
					sur_voie[t][i1][g1][g2] = solver.newVar();
					for (int i2 = 0; i2 < TIMESLOT; ++i2)
					{
						fait_trajet[t][i1][g1][i2][g2] = solver.newVar();
					}
				}
				dans_gare[t][i1][g1] = solver.newVar();
			}
		}
	}
}



// Helpers
bool voie_exists(Graph* map, int i, int k) {
	return map->get_duration(i, k) > 0;
}

bool isFast(int t){
	return SLOW <= t && t < TRAIN;
}

bool isSmall(Graph* map, int gare) {
	return map->get_type(gare) == Small;
}


// contrainte1


/**
 * Pour toute paire de gare (g, g'), pour chaque fenêtre horaire de durée TimeWindow,
 * il existe un train qui dessert g dans cette fenêtre, puis, plus tard, ce même train
 * desservira g 0 , après une durée de trajet d’au plus TravelDuration.
 */
vec<Lit> contraintes1;
void setupContrainte1(Graph* map) {
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (g1 == g2) continue;
			
			// Version non-glissante
			for (int twStart = 0; twStart < TIMESLOT - TIMEWINDOW; twStart = twStart + TIMEWINDOW)
			// Amélioration 1: utiliser des time windows glissantes
			// for (int twStart = 0; twStart < TIMESLOT - TIMEWINDOW - TRAVELDURATION; twStart++)
			{
				contraintes1.clear();

				for (int i1 = twStart; i1 < twStart + TIMEWINDOW && i1 < TIMESLOT; ++i1) {
					for (int i2 = i1 + 1; i2 <= i1 + TRAVELDURATION && i2 < TIMESLOT; ++i2)
					{
						for (int t = 0; t < TRAIN; ++t)
						{
							contraintes1.push(Lit(fait_trajet[t][i1][g1][i2][g2]));
						}
					}
				}

				solver.addClause(contraintes1);
			}	
		}
	}
}

// end contrainte 1

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
	// Les trains de type slow sont des omnibus : ils sont tenus de s’arrêter dans toutes les
    // gares. Les trains de type fast ne sont tenus de s’arrêter que dans les gares de type big.
    for (int t = 0; t < TRAIN; ++t)
    {
    	for (int i = 0; i < TIMESLOT - 1; ++i)
    	{
    		for (int g1 = 0; g1 < TRAIN; ++g1)
    		{
    			for (int g2 = 0; g2 < TRAIN; ++g2)
    			{
    				if (!voie_exists(map, g1, g2)) {
    					continue;
    				}

    				for (int g3 = 0; g3 < TRAIN; ++g3)
    				{
    					if (!voie_exists(map, g2, g3)) {
							continue;
						}

						if (!(isFast(t) && isSmall(map, g2))) {
							solver.addBinary(~Lit(sur_voie[t][i][g1][g2]), ~Lit(sur_voie[t][i + 1][g2][g3]));
						}
    				}
    			}
    		}
    	}
    }

}

vec<Lit> contraints4;
void setupContrainte4(Graph *map){
	// On reste dans les gares TimeWait minutes
	for (int t = 0; t < TRAIN; ++t)
	{
		for (int g = 0; g < STATION; ++g)
		{
			for (int i = 0; i < TIMESLOT; ++i)
			{
				for (int iEnd = i + 1; iEnd < i + TIMEWAIT && iEnd < TIMESLOT; ++iEnd)
				{
					solver.addTernary(Lit(dans_gare[t][i - 1][g]), ~Lit(dans_gare[t][i][g]), Lit(dans_gare[t][iEnd][g]));
				}
			}
		}
	}
}

void setupContrainte5(Graph *map){
	int voieDuration;
	// On reste dans les gares voieDuration minutes
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (g1 == g2 || !voie_exists(map, g1, g2)) continue;

			voieDuration = map->get_duration(g1, g2);

			for (int t = 0; t < TRAIN; ++t)
			{
				for (int i = 0; i < TIMESLOT - voieDuration; ++i)
				{
					for (int i2 = i + 1; i2 < i + voieDuration; ++i2)
					{
						solver.addTernary(Lit(sur_voie[t][i - 1][g1][g2]), ~Lit(sur_voie[t][i][g1][g2]), Lit(sur_voie[t][i2][g1][g2]));
					}
				}
			}
		}
	}
}


// Contrainte 6

std::vector<int> combination;
std::vector<int> choices;
vec<Lit> ensembleConstraints;

void makeContraintForEnsemble(Graph *map, int gare) {
	ensembleConstraints.clear();
	std::cout << "Train Ensemble:";
	for (std::vector<int>::iterator train = combination.begin(); train != combination.end(); ++train)
	{
		std::cout << " " << *train;
		for (int i = 0; i < TIMESLOT; ++i)
		{
			ensembleConstraints.push(~Lit(dans_gare[*train][i][gare]));
		}
	}
	std::cout << std::endl;
	solver.addClause(ensembleConstraints);
}

void makeConstraintFor6(Graph *map, int gare, int offset, int k) {
  if (k == 0) {
    makeContraintForEnsemble(map, gare);
    return;
  }
  for (int i = offset; i <= (int) choices.size() - k; ++i) {
    combination.push_back(choices[i]);
    makeConstraintFor6(map, gare, i+1, k-1);
    combination.pop_back();
  }
}

void makeConstraintForGare(Graph *map, int gare) {
	int capacity = map->get_capacity(gare);
	int n = TRAIN, k = capacity + 1;

	if (n <= k) {
		return;
	}

	choices.clear();
    for (int i = 0; i < TRAIN; ++i) {
    	choices.push_back(i);
    }

	makeConstraintFor6(map, gare, 0, k);
}

void setupContrainte6(Graph* map){
	// On doit respecter la capacité des gares
	for (int g = 0; g < STATION; ++g)
	{
		makeConstraintForGare(map, g);
	}

}

// End contrainte6

vec<Lit> contraintesImplicites1;
void setupContrainteImplicite1(Graph *map){
	// Tout train doit être quelque part
	for (int t = 0; t < TRAIN; ++t)
	{
		for (int i = 0; i < TIMESLOT; ++i)
		{
			contraintesImplicites1.clear();

			for (int g1 = 0; g1 < STATION; ++g1)
			{
				for (int g2 = 0; g2 < STATION; ++g2)
				{
					if (g1 != g2 && voie_exists(map, g1, g2)) {
						contraintesImplicites1.push(Lit(sur_voie[t][i][g1][g2]));
					}
				}
			}

			for (int g1 = 0; g1 < STATION; ++g1)
			{
				contraintesImplicites1.push(Lit(dans_gare[t][i][g1]));
		    }
			
			solver.addClause(contraintesImplicites1);
		}
	}

}

void setupContrainteImplicite2(Graph *map){
	// Un train ne peut pas être dans 2 gares différentes au même moment
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (g1 == g2) continue;

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
	// Un train ne peut pas être sur 2 voies différents au même moment
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
	for (int g2 = 0; g2 < STATION; ++g2)
	{
		for (int g3 = 0; g3 < STATION; ++g3)
		{
			if (g2 == g3 || !voie_exists(map, g2, g3)) {
				// Voie n'existe pas
				continue;
			}

			for (int g1 = 0; g1 < STATION; ++g1)
			{
				for (int i = 0; i < TIMESLOT; ++i)
				{
					for (int t = 0; t < TRAIN; ++t)
					{
						solver.addBinary(~Lit(dans_gare[t][i][g1]), ~Lit(sur_voie[t][i][g2][g3]));
					}
				}
			}
		}
	}
}

vec<Lit> contraintesImplicites5;
void setupContrainteImplicite5(Graph *map)
{
// 	Lorsqu’on sort d’un segment A-B, soit on arrive à gare B soit on est sur
// un segment B-C

		for (int g1 = 0; g1 < STATION; ++g1)
		{
			for (int g2 = 0; g2 < STATION; ++g2)
			{
				if (g1 == g2 || !voie_exists(map, g1, g2)) continue;
				for (int t = 0; t < TRAIN; ++t)
				{
					for (int i = 1; i < TIMESLOT; ++i)
					{
						contraintesImplicites5.clear();
						contraintesImplicites5.push(~Lit(sur_voie[t][i-1][g1][g2]));
						contraintesImplicites5.push(Lit(sur_voie[t][i][g1][g2]));
						contraintesImplicites5.push(Lit(dans_gare[t][i][g2]));

						for (int g3 = 0; g3 < STATION; ++g3)
						{
							if (g2 == g3 || !voie_exists(map, g2, g3)) continue;
							contraintesImplicites5.push(Lit(sur_voie[t][i][g2][g3]));
						}
						solver.addClause(contraintesImplicites5);
					}
				}
			}
		}
}

vec<Lit> contraintesImplicites6;
void setupContrainteImplicite6(Graph *map){
	//Lorsqu’un train sort d’une gare A, il doit être sur un segment qui part de A


	for (int g1 = 0; g1 < STATION; ++g1)
	{
    	for (int t = 0; t < TRAIN; ++t)
		{
		    for (int i = 1; i < TIMESLOT; ++i)
			{
				contraintesImplicites6.clear();
				contraintesImplicites6.push(~Lit(dans_gare[t][i-1][g1]));
				contraintesImplicites6.push(Lit(dans_gare[t][i][g1]));

				for (int g2 = 0; g2 < STATION; ++g2) // ??? si y a pas de gare liée
				{
						if (g1 == g2 || !voie_exists(map, g1, g2)) continue;
						
						contraintesImplicites6.push(Lit(sur_voie[t][i][g1][g2]));
				}
				solver.addClause(contraintesImplicites6);
			}
		}
	}
}


void printTrainMovements(Graph* map) {
	for (int t = 0; t < TRAIN; ++t)
	{
		std::cout << "Train number " << t;
		if (isFast(t)) {
			std::cout << " (fast)";
		} else {
			std::cout << " (slow)";
		}
		std::cout << std::endl; 
		for (int g = 0; g < STATION; ++g)
		{
			std::cout << map->get_name(g);
			std::cout << " |";
			for (int i = 0; i < TIMESLOT; ++i)
			{
				if (solver.model[dans_gare[t][i][g]] == l_True) {
                    std::cout << "X";
                } else {
                	std::cout << "-";
                }
			}
			std::cout << '|' << std::endl;
		}
		std::cout << std::endl;
	}
}

void printGeneralInfo() {
	std::cout << "TIMEWAIT " << TIMEWAIT << std::endl;
	std::cout << "SLOW " << SLOW << std::endl;
	std::cout << "FAST " << FAST << std::endl;
	std::cout << "TRAIN " << TRAIN << std::endl;
	std::cout << "TIMESLOT " << TIMESLOT << std::endl;
	std::cout << "STATION " << STATION << std::endl;
	std::cout << "TRAVELDURATION " << TRAVELDURATION << std::endl;
	std::cout << "TIMEWINDOW " << TIMEWINDOW << std::endl << std::endl << std::endl;
}


void printTravels(Graph* map) {
	std::cout << "Print train movements (to easily examine whether timeWindow rules are respected";
	std::cout << std::endl << std::endl;

	for (int t = 0; t < TRAIN; ++t)
	{
		for (int i1 = 0; i1 < TIMESLOT; ++i1)
		{
			for (int i2 = i1 + 1; i2 < TIMESLOT; ++i2)
			{
				for (int g1 = 0; g1 < STATION; ++g1)
				{
					for (int g2 = 0; g2 < STATION; ++g2)
					{
						if (solver.model[fait_trajet[t][i1][g1][i2][g2]] == l_True) {
                            std::cout << map->get_name(g1) << ":" << i1 << " -- t" 
                                      << t << " -- " << i2 << ":" << g2 << std::endl;
                        }
					}
				}
			}
		}
	}
	std::cout << std::endl << std::endl;
}

void printRes(Graph* map) {
	printGeneralInfo();
	printTravels(map);
	printTrainMovements(map);
	
}

/**
 * vérifier que  un déplacement d'une gare 1 à 2 en un temps TD correpond bien a un voyage
 */
void setupContrainte1PourUtiliserDerniereVariable() {
	for (int t = 0; t < TRAIN; ++t)
	{
		for (int g1 = 0; g1 < STATION; ++g1)
		{
			for (int g2 = 0; g2 < STATION; ++g2)
			{
				if (g1 == g2) continue;

				for (int i1 = 0; i1 < TIMESLOT; ++i1)
				{
					for (int i2 = i1 + 1; i2 < TIMESLOT; ++i2)
					{
                        solver.addBinary(~Lit(fait_trajet[t][i1][g1][i2][g2]), Lit(dans_gare[t][i1][g1]));
                        solver.addBinary(~Lit(fait_trajet[t][i1][g1][i2][g2]), Lit(dans_gare[t][i2][g2]));
					}
				}
			}
		}
	}
}




/**
 * vérifier que un voyage correpond bien a un déplacement d'une gare 1 à 2 en un temps TD
 */
vec<Lit> contraintesNewVar;
void setupContrainte2PourUtiliserDerniereVariable() {
	for (int t = 0; t < TRAIN; ++t)
	{
		for (int g1 = 0; g1 < STATION; ++g1)
		{
			for (int g2 = 0; g2 < STATION; ++g2)
			{
				if (g1 == g2) continue;

				for (int i1 = 0; i1 < TIMESLOT; ++i1)
				{
					for (int i2 = i1 + 1; i2 < TIMESLOT; ++i2)
					{
						contraintesNewVar.clear();
						contraintesNewVar.push(~Lit(dans_gare[t][i1][g1]));
						contraintesNewVar.push(~Lit(dans_gare[t][i2][g2]));
                        contraintesNewVar.push(Lit(fait_trajet[t][i1][g1][i2][g2]));
                        solver.addClause(contraintesNewVar);
                        
					}
				}
			}
		}
	}
}

void setupContraintesPourUtiliserDerniereVariable(){
	setupContrainte1PourUtiliserDerniereVariable();
	setupContrainte2PourUtiliserDerniereVariable();
}


void setupContrainteJamaisRetard(Graph* map){
	int voieDuration;
	// Après voieDuration minutes, on n'est plus sur la voie
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (g1 == g2 || !voie_exists(map, g1, g2)) continue;

			voieDuration = map->get_duration(g1, g2);

			for (int t = 0; t < TRAIN; ++t)
			{
				for (int i = 0; i < TIMESLOT - voieDuration - 1; ++i)
				{
					solver.addTernary(Lit(sur_voie[t][i - 1][g1][g2]), ~Lit(sur_voie[t][i][g1][g2]), ~Lit(sur_voie[t][i + voieDuration][g1][g2]));
				}
			}
		}
	}
}

int main() {
	// ---------- Map ---------- //

	Graph* map;
	map = Graph::parse((char *) MAPFILE); // you can change the map file here !!
	map->print();
	assert(STATION == map->get_size());

	// ---------- Outputs ---------- //

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

	setupContraintesPourUtiliserDerniereVariable();

	// Amelioration 2: si on enleve ca, on autorise du retard, pour mieux simuler la SNCB
	// setupContrainteJamaisRetard(map);






	// ---------- Solver ---------- //

	printf("\n\n");
	solver.solve();
	printf("\n");



	// ---------- Printer ---------- //

	if (!solver.okay()) {
		printf("\nNO\n");
	}
	else {
		printf("\nFound a solution\n\n\n");
		printRes(map);
	}




	// ---------- Delete ---------- //

	delete map;
	return EXIT_SUCCESS;
}
