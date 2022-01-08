/* Planet.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef PLANET_H_
#define PLANET_H_

#include "ConditionSet.h"
#include "CustomSale.h"
#include "Sale.h"

#include <list>
#include <memory>
#include <set>
#include <string>
#include <vector>

class DataNode;
class Fleet;
class Government;
class Outfit;
class PlayerInfo;
class Ship;
class Sprite;
class System;



// Class representing a stellar object you can land on. (This includes planets,
// moons, and space stations.) Each planet has a certain set of services that
// are available, as well as attributes that determine what sort of missions
// might choose it as a source or destination.
class Planet {
public:
	class Port
	{
	public:
		// The different properties that can be recharged by a port.
		enum RechargeType
		{
			None = 0x0,
			Shields = 0x1,
			Hull = 0x2,
			Energy = 0x4,
			Fuel = 0x8,
			All = Shields | Hull | Energy | Fuel,
		};
	public:
		// The name of this port.
		std::string name;

		// The description of this port. Shown when clicking on the
		// port button on the planet panel.
		std::string description;

		// What is recharged when landing on this port.
		RechargeType recharge = None;

		// Whether this port has news.
		bool hasNews = false;
	};


public:
	// Load a planet's description from a file.
	void Load(const DataNode &node);
	// Check if both this planet and its containing system(s) have been defined.
	bool IsValid() const;

	// Get the name of the planet (all wormholes use the same name).
	// When saving missions or writing the player's save, the reference name
	// associated with this planet is used even if the planet was not fully
	// defined (i.e. it belongs to an inactive plugin).
	const std::string &Name() const;
	void SetName(const std::string &name);
	// Get the name used for this planet in the data files.
	const std::string &TrueName() const;
	// Get the planet's descriptive text.
	const std::string &Description() const;
	// Get the landscape sprite.
	const Sprite *Landscape() const;
	// Get the name of the ambient audio to play on this planet.
	const std::string &MusicName() const;

	// Get the list of "attributes" of the planet.
	const std::set<std::string> &Attributes() const;

	// Get planet's noun descriptor from attributes
	const std::string &Noun() const;

	// Check whether there is a spaceport (which implies there is also trading,
	// jobs, banking, and hiring).
	bool HasSpaceport() const;
	// Check whether there is a port (which may even be a full spaceport).
	bool HasPort() const;
	// Get this planet's port. Might be empty if there is no port.
	const Port &GetPort() const;

	// Check if this planet is inhabited (i.e. it has a spaceport, and does not
	// have the "uninhabited" attribute).
	bool IsInhabited() const;

	// Check if the security of this planet has been changed from the default so
	// that we can check if an uninhabited world should fine the player.
	bool HasCustomSecurity() const;

	// Check if this planet has a shipyard.
	bool HasShipyard() const;
	// Get the list of ships in the shipyard.
	const Sale<Ship> &Shipyard() const;
	// Check if this planet has an outfitter.
	bool HasOutfitter() const;
	// Get the list of outfits available from the outfitter with their custom elements.
	const Sale<Outfit> &Outfitter() const;
	// Get the local price of this outfit.
	double GetLocalRelativePrice(const Outfit *outfit, const ConditionSet::Conditions conditions) const;
	// Get the availability of this outfit.
	CustomSale::SellType GetAvailability(const Outfit *outfit, const ConditionSet::Conditions conditions) const;
	
	// Get this planet's government. If not set, returns the system's government.
	const Government *GetGovernment() const;
	// You need this good a reputation with this system's government to land here.
	double RequiredReputation() const;
	// This is what fraction of your fleet's value you must pay as a bribe in
	// order to land on this planet. (If zero, you cannot bribe it.)
	double GetBribeFraction() const;
	// This is how likely the planet's authorities are to notice if you are
	// doing something illegal.
	double Security() const;

	// Set or get what system this planet is in. This is so that missions, for
	// example, can just hold a planet pointer instead of a system as well.
	const System *GetSystem() const;
	// Check if this planet is in the given system. Note that wormholes may be
	// in more than one system.
	bool IsInSystem(const System *system) const;
	void SetSystem(const System *system);
	// Remove the given system from the list of systems this planet is in. This
	// must be done when game events rearrange the planets in a system.
	void RemoveSystem(const System *system);

	// Check if this is a wormhole (that is, it appears in multiple systems).
	bool IsWormhole() const;
	const System *WormholeSource(const System *to) const;
	const System *WormholeDestination(const System *from) const;
	const std::vector<const System *> &WormholeSystems() const;

	// Check if the given ship has all the attributes necessary to allow it to
	// land on this planet.
	bool IsAccessible(const Ship *ship) const;
	// Check if this planet has any required attributes that restrict landability.
	bool IsUnrestricted() const;

	// Below are convenience functions which access the game state in Politics,
	// but do so with a less convoluted syntax:
	bool HasFuelFor(const Ship &ship) const;
	bool CanLand(const Ship &ship) const;
	bool CanLand() const;
	bool CanUseServices() const;
	void Bribe(bool fullAccess = true) const;

	// Demand tribute, and get the planet's response.
	std::string DemandTribute(PlayerInfo &player) const;
	void DeployDefense(std::list<std::shared_ptr<Ship>> &ships) const;
	void ResetDefense() const;


private:
	bool isDefined = false;
	std::string name;
	std::string description;
	Port port;
	const Sprite *landscape = nullptr;
	std::string music;

	std::set<std::string> attributes;

	std::set<const Sale<Ship> *> shipSales;
	std::set<const Sale<Outfit> *> outfitSales;
	// The lists above will be converted into actual ship lists when they are
	// first asked for:
	mutable CustomSale customSale;
	mutable Sale<Ship> shipyard;
	mutable Sale<Outfit> outfitter;

	const Government *government = nullptr;
	double requiredReputation = 0.;
	double bribe = 0.01;
	double security = .25;
	bool inhabited = false;
	bool customSecurity = false;
	// Any required attributes needed to land on this planet.
	std::set<std::string> requiredAttributes;

	// The salary to be paid if this planet is dominated.
	int tribute = 0;
	// The minimum combat rating needed to dominate this planet.
	int defenseThreshold = 4000;
	mutable bool isDefending = false;
	// The defense fleets that should be spawned (in order of specification).
	std::vector<const Fleet *> defenseFleets;
	// How many fleets have been spawned, and the index of the next to be spawned.
	mutable size_t defenseDeployed = 0;
	// Ships that have been created by instantiating its defense fleets.
	mutable std::list<std::shared_ptr<Ship>> defenders;

	std::vector<const System *> systems;
};



#endif
