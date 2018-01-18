// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"

#include "Location.h"
#include "Orientation.h"
#include "Search.h"
#include <list>
#include <algorithm>

#include "WorldState.h"

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void GameOver (int score);

	void UpdateState (Percept& percept);
	bool FacingDeath();
	WorldState currentState;
	Action lastAction;
	Percept lastPercept;
	list<Action> actionList;
	int numActions;
	SearchEngine searchEngine;
	vector<Location> breezeLocations;
	vector<Location> stenchLocations;
	vector<Location> frontier;


	bool InferPit();
	void InferWumpus();
	int ContainsLocation(vector<Location> &locations, Location location);
	int Contains(vector<Location> vec, int x, int y);
	bool Reason();
	void Enumerate(int query, vector<bool> frontierValue, int i, double *trueSum, double *falseSum);
	bool CheckLocations(vector<bool> frontierValue);
};

#endif // AGENT_H
