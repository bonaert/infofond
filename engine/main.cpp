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



// one variable
void init_varA () {
	example_of_array[0] = solver.newVar();
}


// no more variables
void init_varB () {

}


// useless
void constraint_AB (Graph *map) {
	literals.clear();
	literals.push(Lit(example_of_array[0]));
	solver.addClause(literals);
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

	

	init_varA();
	init_varB();



	// ---------- Constraints ---------- //

	constraint_AB(map);



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
