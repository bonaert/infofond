#include "graph.hpp"
#include "../solver/Solver.hpp"
#include <iostream>
#include <vector>

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


bool isFast(int t){
	return SLOW <= t && t < TRAIN;
}

bool isSmall(Graph* map, int gare) {
	return map->get_type(gare) == Small;
}


// contrainte1

vec<Lit> contraintes1Trajet;
void setupContrainte1FaireTrajet(Graph *map){
	int i2;
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			for (int iStart = 0; iStart <= TIMESLOT - TIMEWINDOW; iStart = iStart + TIMEWINDOW)
			{
				contraintes1Trajet.clear();

				for (int i1 = iStart; i1 < iStart + TIMEWINDOW && iStart < TIMESLOT; ++i1)
				{
					i2 = i1 + map->get_duration(g1, g2);

					if (i2 >= TIMESLOT) {
						break;
					}


					for (int t = 0; t < TRAIN; ++t)
					{
						contraintes1Trajet.push(~Lit(sur_voie[t][i1][g1][g2]));
						contraintes1Trajet.push(~Lit(sur_voie[t][i2][g1][g2]));
					}
				}

				solver.addClause(contraintes1Trajet);
			}
		}
	}
}

vec<Lit> contraintes1Gare;
void setupContrainte1AllerGare(Graph *map){
	for (int g = 0; g < STATION; ++g)
	{
		for (int iStart = 0; iStart + TIMEWINDOW <= TIMESLOT; iStart = iStart + TIMEWINDOW)
		{
			contraintes1Gare.clear();

			for (int i = iStart; i < iStart + TIMEWINDOW && i < TIMESLOT; ++i)
			{
				for (int t = 0; t < TRAIN; ++t)
				{
					std::cout << "(t, i, g) = " << t << " " << i << " " << g << std::endl;
					contraintes1Gare.push(Lit(dans_gare[t][i][g]));
				}
			}

			solver.addClause(contraintes1Gare);
		}
	}
}

void setupContrainte1(Graph *map){
	// Trajet entre toute paire de gares dans chaque timewindow
	setupContrainte1FaireTrajet(map);
	// setupContrainte1AllerGare(map);
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
	int travelDuration;
	for (int g1 = 0; g1 < STATION; ++g1)
	{
		for (int g2 = 0; g2 < STATION; ++g2)
		{
			if (!voie_exists(map, g1, g2)) {
				continue;
			}

			travelDuration = map->get_duration(g1, g2);

			for (int t = 0; t < TRAIN; ++t)
			{
				for (int i = 0; i < TIMESLOT - travelDuration; ++i)
				{
					for (int i2 = 0; i2 < i + travelDuration; ++i2)
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
				for (int i = 0; i < TIMESLOT; ++i)
				{
					for (int t = 0; t < TRAIN; ++t)
					{
						solver.addBinary(~Lit(dans_gare[t][i][g3]), ~Lit(sur_voie[t][i][g1][g2]));
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
					  for (int i = 1; i < TIMESLOT; ++i) // TODO indices des i ?
						{
								contraintesImplicites6.clear();
								contraintesImplicites6.push(~Lit(dans_gare[t][i-1][g1]));
								contraintesImplicites6.push(Lit(dans_gare[t][i][g1]));

								for (int g2 = 0; g2 < STATION; ++g2) // ??? si y a pas de gare liée
								{
										if (g1 == g2 || !voie_exists(map, g1, g2))
										{
											contraintesImplicites6.push(Lit(sur_voie[t][i][g1][g2]));
										}
								}
								solver.addClause(contraintesImplicites6);
						}
				}


	  }
}


void printResults(){

}

void printRes() {
	for (int i = 0; i < TIMESLOT; ++i) {
        std::cout << "Time = " << i << std::endl;
        std::cout << "-------" << std::endl;
        for (int t = 0; t < TRAIN; ++t) {
        	for (int g1 = 0; g1 < STATION; ++g1) {
        		std::cout << "i: " << i << " t: " << t << " g1: " << g1 << std::endl;
                if (solver.model[dans_gare[t][i][g1]] == l_True) {
                    std::cout << "Le train " << t << " est dans la gare " << g1 << std::endl;

                }

                for (int g2 = 0; g2 < STATION; ++g2)  {
                    if (solver.model[sur_voie[t][i][g1][g2]] == l_True) {
                        std::cout << "Le train " << t << " est sur le segment entre la gare "
                                  << g1 << " et " << g2 << std::endl;

                    }
                }
            }

        }
        std::cout << std::endl;
    }
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
		printRes();
	}




	// ---------- Delete ---------- //

	delete map;
	return EXIT_SUCCESS;
}
