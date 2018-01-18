// Agent.cc

#include <iostream>
#include <list>
#include <vector>
#include "Agent.h"


using namespace std;

Agent::Agent ()
{
	currentState.worldSize = 2; // at least 2x2
	currentState.wumpusLocation = Location(0,0); // unknown
	currentState.goldLocation = Location(0,0); // unknown
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	currentState.agentLocation = Location(1,1);
	currentState.agentOrientation = RIGHT;
	currentState.agentHasArrow = true;
	currentState.agentHasGold = false;
	currentState.wumpusAlive = true;
	actionList.clear();
	searchEngine.AddSafeLocation(1,1); // (1,1) always safe
	// At start of game, wumpus is alive
	searchEngine.RemoveSafeLocation(currentState.wumpusLocation.X, currentState.wumpusLocation.Y);
	lastAction = CLIMB; // dummy action
	numActions = 0;

	frontier.clear();
	frontier.push_back(Location(1, 2));
	frontier.push_back(Location(2, 1));
}

Action Agent::Process (Percept& percept)
{
	Action action;
	list<Action> actionList2;
	bool foundPlan = false;

	lastPercept = percept;
	UpdateState(percept);

	if (actionList.empty()) {
		foundPlan = false;
		if ((! foundPlan) && percept.Glitter) {
			actionList.push_back(GRAB);
			foundPlan = true;
		}
		if ((! foundPlan) && currentState.agentHasGold && (currentState.agentLocation == Location(1,1))) {
			actionList.push_back(CLIMB);
			foundPlan = true;
		}
		if ((! foundPlan) && (! (currentState.goldLocation == Location(0,0))) && (! currentState.agentHasGold)) {
			// If know gold location, but don't have it, then find path to it
			actionList2 = searchEngine.FindPath(currentState.agentLocation, currentState.agentOrientation, currentState.goldLocation, UP);
			if (actionList2.size() > 0) {
				actionList.splice(actionList.end(), actionList2);
				foundPlan = true;
			}
		}
		if ((! foundPlan) && currentState.agentHasGold) {
			// If have gold, then find path to (1,1)
			actionList2 = searchEngine.FindPath(currentState.agentLocation, currentState.agentOrientation, Location(1,1), DOWN);
			if (actionList2.size() > 0) {
				actionList.splice(actionList.end(), actionList2);
				foundPlan = true;
			}
		}
		if ((! foundPlan) && percept.Stench && currentState.agentHasArrow) {
			actionList.push_back(SHOOT);
			foundPlan = true;
		}
		if ((! foundPlan) && percept.Bump) {
			actionList.push_back(TURNLEFT);
			actionList.push_back(GOFORWARD);
			foundPlan = true;
		}
		if (! foundPlan) {
			// Random move
			action = (Action) (rand() % 3);
			actionList.push_back(action);
			foundPlan = true;
		}
	}
	action = actionList.front();
	actionList.pop_front();
	// One final check that we aren't moving to our death
    if ((action == GOFORWARD) && FacingDeath()) {
    	action = TURNLEFT;
    }
	lastAction = action;
	numActions++;
	return action;
}

void Agent::UpdateState (Percept& percept)
{
	// Check if wumpus killed
	if (percept.Scream)
	{
		currentState.wumpusAlive = false;
		// Since only kill wumpus point-blank, we know its location is in front of agent
		currentState.wumpusLocation = currentState.agentLocation;
		switch (currentState.agentOrientation)
		{
			case RIGHT: currentState.wumpusLocation.X++; break;
			case UP: currentState.wumpusLocation.Y++; break;
			case LEFT: currentState.wumpusLocation.X--; break;
			case DOWN: currentState.wumpusLocation.Y--; break;
		}
	}
	// Check if have gold
	if (lastAction == GRAB)
	{
		currentState.agentHasGold = true;
		currentState.goldLocation = currentState.agentLocation;
	}
	// Check if used arrow
	if (lastAction == SHOOT)
	{
		currentState.agentHasArrow = false;
	}
	// Update orientation
	if (lastAction == TURNLEFT)
	{
		currentState.agentOrientation = (Orientation) ((currentState.agentOrientation + 1) % 4);
	}
	if (lastAction == TURNRIGHT)
	{
		currentState.agentOrientation = (Orientation) ((currentState.agentOrientation + 3) % 4);
	}
	// Update location
	if ((lastAction == GOFORWARD) && (! percept.Bump))
	{
		switch (currentState.agentOrientation)
		{
			case RIGHT: currentState.agentLocation.X++; break;
			case UP: currentState.agentLocation.Y++; break;
			case LEFT: currentState.agentLocation.X--; break;
			case DOWN: currentState.agentLocation.Y--; break;
		}

		// Remove new location from frontier
		frontier.erase(std::remove(frontier.begin(), frontier.end(), currentState.agentLocation), frontier.end());

		// Update frontier	
		Location up = currentState.agentLocation;
		Location down = currentState.agentLocation;
		Location left = currentState.agentLocation;
		Location right = currentState.agentLocation;

		up.Y++;
		down.Y--;
		left.X--;
		right.X++;

		if(currentState.agentOrientation != DOWN && Contains(frontier, up.X, up.Y) < 0 && up.Y <= 4 )
			frontier.push_back(up);
		if(currentState.agentOrientation != UP && Contains(frontier, down.X, down.Y) < 0 && down.Y >= 1)
			frontier.push_back(down);
		if(currentState.agentOrientation != RIGHT && Contains(frontier, left.X, left.Y) < 0 && left.X >= 1)
			frontier.push_back(left);
		if(currentState.agentOrientation != LEFT && Contains(frontier, right.X, right.Y) < 0 && right.X <= 4)
			frontier.push_back(right);
	}

	// Update world size
	if (currentState.agentLocation.X > currentState.worldSize)
	{
		currentState.worldSize = currentState.agentLocation.X;
	}
	if (currentState.agentLocation.Y > currentState.worldSize)
	{
		currentState.worldSize = currentState.agentLocation.Y;
	}

	// Update safe locations in search engine
	int x = currentState.agentLocation.X;
	int y = currentState.agentLocation.Y;
	searchEngine.AddSafeLocation(x,y);
	if ((! percept.Breeze) && ((! percept.Stench) || (! currentState.wumpusAlive)))
	{
		if (x > 1) searchEngine.AddSafeLocation(x-1,y);
		if (y > 1) searchEngine.AddSafeLocation(x,y-1);
		if (x < currentState.worldSize) searchEngine.AddSafeLocation(x+1,y);
		if (y < currentState.worldSize) searchEngine.AddSafeLocation(x,y+1);
	}

	if (percept.Breeze) {
		if (!ContainsLocation(breezeLocations, currentState.agentLocation))
			breezeLocations.push_back(currentState.agentLocation);
	}
	if (percept.Stench) {
		if(!ContainsLocation(stenchLocations,currentState.agentLocation))
			stenchLocations.push_back(currentState.agentLocation);
		InferWumpus();
	}
}

bool Agent::FacingDeath()
{
	int i = 0;
	int x = currentState.agentLocation.X;
	int y = currentState.agentLocation.Y;
	Orientation orientation = currentState.agentOrientation;
	if (orientation == RIGHT) {
		x++;
	}
	if (orientation == UP) {
		y++;
	}
	if (orientation == LEFT) {
		x--;
	}
	if (orientation == DOWN) {
		y--;
	}
	vector<Location>::iterator itr;
	Location facingLoc = Location(x,y);
	for (itr = currentState.pitLocations.begin(); itr != currentState.pitLocations.end(); itr++) {
		if (*itr == facingLoc) {
			return true;
		}
	}
	if ((currentState.wumpusLocation == facingLoc) && currentState.wumpusAlive) {
		return true;
	}

	if(InferPit())
		return true;

	return false;
}

void Agent::GameOver (int score)
{
	if ((score < 0) && (numActions < 1000)) {
		// Agent died by GOFORWARD into location with wumpus or pit, so make that location unsafe
		int x = currentState.agentLocation.X;
		int y = currentState.agentLocation.Y;
	    Orientation orientation = currentState.agentOrientation;
	    if (orientation == RIGHT) {
	    	x++;
	    }
	    if (orientation == UP) {
	    	y++;
	    }
	    if (orientation == LEFT) {
	    	x--;
	    }
	    if (orientation == DOWN) {
	    	y--;
	    }
	    if (lastPercept.Breeze && (! lastPercept.Stench)) {
	    	currentState.pitLocations.push_back(Location(x,y));
	    }
	    if (lastPercept.Stench && (! lastPercept.Breeze)) {
	    	currentState.wumpusLocation = Location(x,y);
	    }
	}
}

bool Agent::InferPit()
{
	return Reason();
}

void Agent::InferWumpus()
{
	Location l = this->currentState.agentLocation;
	if (ContainsLocation(stenchLocations, Location(l.X + 1, l.Y + 1))==1 && ContainsLocation(stenchLocations, Location(l.X, l.Y + 1))==0) {
		currentState.wumpusLocation = Location(l.X + 1, l.Y);
	}
	else if (ContainsLocation(stenchLocations, Location(l.X + 1, l.Y + 1))==1 && ContainsLocation(stenchLocations,Location(l.X+1,l.Y))==0) {
		currentState.wumpusLocation = Location(l.X, l.Y + 1);
	}
	else if (ContainsLocation(stenchLocations,Location(l.X+1,l.Y-1))==1 && ContainsLocation(stenchLocations,Location(l.X,l.Y-1))==0) {
		currentState.wumpusLocation = Location(l.X + 1, l.Y);
	} 
	else if (ContainsLocation(stenchLocations,Location(l.X-1,l.Y+1))==1 && ContainsLocation(stenchLocations,Location(l.X,l.Y+1))==0) {
		currentState.wumpusLocation = Location(l.X - 1, l.Y);
	}
}


int Agent::ContainsLocation(vector<Location> &locations, Location location) {

	if (std::find(locations.begin(), locations.end(), location) != locations.end())
		return 1;

	if (std::find(searchEngine.safeLocations.begin(), searchEngine.safeLocations.end(), location) != searchEngine.safeLocations.end())
		return 0;

	return -1;
}

// determine if a location vector contains a location
int Agent::Contains(vector<Location> vec, int x, int y)
{
	int size = vec.size();

	int i = 0;
	for(i = 0; i < size; i++)
	{
		if(vec[i] == Location(x, y))
			return i;
	}

	return -1;
}

// check if the query has less than a 50% chance of having a pit
bool Agent::Reason()
{
	Location query;
	int q = 0;
	vector<bool> frontierValue;
	int i = -1;
	double trueSum = 0;
	double falseSum = 0;
	double alpha = 0;

	query = currentState.agentLocation;
	switch (currentState.agentOrientation)
	{
	case RIGHT: query.X++; break;
	case UP: query.Y++; break;
	case LEFT: query.X--; break;
	case DOWN: query.Y--; break;
	}

	// determine query index in frontier and initialize frontier values to true
	for(i = 0; i < frontier.size(); i++)
	{
		if(frontier[i] == query)
			q = i;

		frontierValue.push_back(true);
	}
	
	i = -1;

	// calculate probability through enumeration
	Enumerate(q, frontierValue, i, &trueSum, &falseSum);

	if(trueSum == 0)
		return false;

	// calculate alpha to normalize the true and false probabilities
	alpha = trueSum + falseSum;

	// normalize the true probability
	trueSum = trueSum / alpha;

	// if the probability is less than 50% return true otherwise false
	if(trueSum > 0.5)
		return true;
	else	
		return false;
}

// determine all combinations of frontier pit locations and their probability
void Agent::Enumerate(int query, vector<bool> frontierValue, int i, double *trueSum, double *falseSum)
{
	// 
	if(i == frontier.size() - 1)
	{
		// check if frontier pit locations do not contradict known breeze and safe locations
		if(!CheckLocations(frontierValue))
			return;
		
		// determine probability of pit location combination
		double probability = 1;
		for(bool value : frontierValue)
		{
			if(value)
				probability *= 0.2;
			else
				probability *= 0.8;
		}

		// if the query location has a pit then add then probability
		// to the probability of the query location having a pit
		if(frontierValue[query])
			(*trueSum) += probability;
		// if the query location does not have a pit then add then probability
		// to the probability of the query location not having a pit
		else
			(*falseSum) += probability;

		return;
	}

	i++;

	Enumerate(query, frontierValue, i, trueSum, falseSum);

	frontierValue[i] = false;

	Enumerate(query, frontierValue, i, trueSum, falseSum);
}

// check frontier values coordinate with known breeze and safe locations
bool Agent::CheckLocations(vector<bool> frontierValue)
{
	int i = 0;

	// check each known breeze is adjacent to a frontier pit location
	for(Location breeze : breezeLocations)
	{
		if((i = Contains(frontier, breeze.X, breeze.Y + 1)) > 0 && frontierValue[i])
			continue;
		if((i = Contains(frontier, breeze.X, breeze.Y - 1)) > 0 && frontierValue[i])
			continue;
		if((i = Contains(frontier, breeze.X - 1, breeze.Y)) > 0 && frontierValue[i])
			continue;
		if((i = Contains(frontier, breeze.X + 1, breeze.Y)) > 0 && frontierValue[i])
			continue;
		
		return false;
	}

	// check each frontier pit location is not a known safe location
	for(i = 0; i < frontier.size(); i++)
	{
		if(frontierValue[i] && searchEngine.SafeLocation(frontier[i].X, frontier[i].Y))
			return false;
	}

	return true;
}