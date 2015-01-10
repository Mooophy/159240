// Name(s): 
//   ID(s): 


// 159.240 - Assignment 3 skeleton code.

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

// The Ship struct
struct Ship {
	float x,y; 		// Every ship has a position
	float vx,vy;	// and a velocity
	int team;
	
	
	// These three data members are for targeting.
	// Ignore them
	bool target_acquired;	
	bool destroyed;
	Ship* target;
};

// Parameters for each of the rules
const float	g_cohesion_desire 		= 0.0004f;	// how much a ship wants to stick to its neighbours
const float	g_cohesion_radius 		= 100.0f;	// in what radius are ships considered neighbours for this rule

const float g_separation_radius		= 5.0f;		// in what radius are ships too close to each other.
const float g_separation_affinity	= 0.1;		// how much a ship wants to stay separated.

const float	g_alignment_desire	 	= 0.002f;	// how much a ship wants to align to its neighbours
const float g_alignment_radius		= 100.0f;	// the radius in which ships are considered neighbours for this rule


// Some general parameters
const int g_windowWidth = 1280;		// how wide is the screen
const int g_windowHeight = 800;		// how high is the screen
const float	g_global_speed 			= 0.7f;		// bullets and ships are slowed by this coefficient
const float	g_max_velocity 			= 2.5f;		// no ships can go faster than this number
const float	g_velocity_degrade 		= 1.0f;		// for 1.0f, degrading the velocity (ie. friction) is disabled. Set to 0.8 to see 
const bool	g_friendly_fire 			= false;		// whether or not bullets can destroy any ship


const int 	g_initial_agent_count 	= 80;			// Number of ships to start with
const float g_initial_speed 			= 0.9f;		// The initial speed of the ships
const int 	g_team_count 				= 2;

const float	g_target_pursuit_coefficient = 0.05f;	// how much ships try to pursue their targets

const float g_bullet_speed				= 4.9f;		// how fast bullets travel in one timestep
const float g_bullet_close_distance = 4.0f;		// if a bullet is closer than this, then the ship explodes


// These are some global parameters that keep track of how many ships are created and destroyed
unsigned int g_ships_destroyed		= 0;
unsigned int g_ships_created			= g_initial_agent_count;


const float g_scale 						= 4.0;	// The scale size of the simulation



int RandInt(int x,int y) { 
	return rand()%(y-x+1)+x; 
}
double RandFloat() { 
	return (rand())/(RAND_MAX+1.0); 
}


// Some Win32 code is hidden in here. Please don't change support.h
#include "support.h"


// This function just makes sure that a ship can't go faster than g_max_velocity.
// Sometimes they just disappear if we don't do this.
void ClampVelocity(Ship *s) {
	if (s->vx > g_max_velocity) s->vx = g_max_velocity;
	if (s->vy > g_max_velocity) s->vy = g_max_velocity;
			
	if (s->vx < -g_max_velocity) s->vx = -g_max_velocity;
	if (s->vy < -g_max_velocity) s->vy = -g_max_velocity;
}

// This function applies the velocity of a ship to its position to move it along.
void MoveShip(Ship *s) {
	s->x += s->vx * g_global_speed;
	s->y += s->vy * g_global_speed;
}

// This function gets the distance as a float from ship a to ship b. 
float GetDistanceBetween(Ship* a, Ship* b) {
	float tx = b->x - a->x;
	float ty = b->y - a->y;
	
	return sqrtf(tx*tx + ty*ty);
}

//---------------- COHESION RULE ----------------
// This function adds the cohesion rule contribution to the 
// velocity of the Ship pointer passed to it.
void AddCohesionRule(Ship *a) {
	
}

//---------------- SEPARATION RULE ----------------
void AddSeparationRule(Ship *a) {
	
}

//---------------- ALIGNMENT RULE ----------------
void AddAlignmentRule(Ship *a) {
	
}


void AddAttackRule(Ship *a) {
	a->target_acquired = true;

	// First we have to find the closest ship to the Ship a.
	
	// To do this, we set a minimum distance to the distance between Ship a, and Ship 0.
	float min = GetDistanceBetween(g_agents[0], a);
	a->target = NULL;
	Ship *target = g_agents[0];
	//a->target = g_agents[0]; // Keep track of which ship is the closest.

	// We now loop through all the ships, and check to see if we can find a smaller distance
	for (unsigned int m=0; m < g_agents.size(); ++m) {
		
		float potnewmin = GetDistanceBetween(g_agents[m], a);
		
		// here we check if our potential new minimum distance is better, and reassign the target
		// if it is.
		if (potnewmin < min && g_agents[m]->team != a->team) {
			min = potnewmin;
			target = g_agents[m];
		}
	}

	if (target != NULL && target->team != a->team) {
		
		// If we get in here, it means that we have a target to attack.
		// What we need to do, is obtain a relative vector to the target,
		// and shoot in that direction.
		
		a->target = target;
		float vx = a->target->x - a->x;
		float vy = a->target->y - a->y;
		float mag = sqrtf(vx*vx + vy*vy);
		vx /= mag+0.0001f;
		vy /= mag+0.0001f;
		if (a->vx * vx + a->vy * vy > 0.8f) {
			if (RandFloat() < 0.01f) {
				// This function will actually shoot a bullet in the direction
				// we give it.
				FireBullet(a->x, a->y, vx*g_bullet_speed,vy*g_bullet_speed, a); 
			}
		}
		
		a->vx += vx * g_target_pursuit_coefficient;
		a->vy += vy * g_target_pursuit_coefficient;
	}
}

void DegradeVelocity(Ship* a) {
	a->vx = a->vx * g_velocity_degrade;
	a->vy = a->vy * g_velocity_degrade;
}




bool UpdateAgents() {
	for (unsigned int i=0; i < g_agents.size(); ++i) {
		
		AddCohesionRule(g_agents[i]);		// Cohesion rule applied to Ship i
		AddSeparationRule(g_agents[i]);	// Separation rule applied to Ship i
		AddAlignmentRule(g_agents[i]);	// Alignment rule applied to Ship i
		
		AddAttackRule(g_agents[i]);		// Special attack rule applied to Ship i
	
		
		DegradeVelocity(g_agents[i]);		// This function will add some friction to slow things down
				
		ClampVelocity(g_agents[i]);		// Check that velocities aren't too big.
		MoveShip(g_agents[i]);				// Finally, move the ship by its velocity.
		
	}
	
	return true; 	// return true, everything went ok.
}




