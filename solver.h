#ifndef SOLVER_H
#define SOLVER_H

#include <string>

class C_Solver
{
public:
	C_Solver();
	bool possible;
	std::string solString;
	int length;
	int numBoxPushes;
	int numBoxLines;
	void solve();
	void reset() { possible = false; length = 0; numBoxPushes = 0; numBoxLines = 0; solString = ""; }
};

#endif