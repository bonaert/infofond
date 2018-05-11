#include "graph.hpp"
#include "../solver/Solver.hpp"


#define TIMEWAIT 3			// le temps d'attente minimum dans une gare desservie
#define SLOW 1				// nombre de train lent
#define FAST 0				// nombre de train rapide
#define TRAIN (SLOW+FAST)	// nombre de train
#define TIMESLOT 30			// nombre de minutes dans la plage horaire
#define STATION 4			// nombre de gare (voir la map)
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
