/* Sale.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef SALE_H_
#define SALE_H_

#include "DataNode.h"
#include "Set.h"

#include <set>
#include <string>
#include <algorithm>


template <class Item>
class OutfitSale;



// Container used for the price and selling type of different items (outfits in this case).
class Sold {
public:
	enum ShowSold {
		NONE = 0,
		DEFAULT = (1 << 0),
		IMPORT = (1 << 1),
		HIDDEN = (1 << 2)
	};
	
	
public:
	const double GetCost() const;
	
	inline void SetCost(double newCost) { cost = newCost; }
	
	const ShowSold GetShown() const;
	
	const std::string &GetShow() const;
	
	void SetBase(double cost = 0., std::string shown = "");
	
	
public:
	// Contains the Show types and their corresponding strings to show with the item.
	static const std::map<ShowSold, const std::string> show;
	
	
private:
	double cost = 0.;
	ShowSold shown = ShowSold::DEFAULT;
};



// Class representing a set of items that are for sale on a given planet.
// Multiple sale sets can be merged together into a single one.
template <class Item>
class Sale : public std::set<const Item *> {
public:
	void Load(const DataNode &node, const Set<Item> &items);
	
	void Add(const Sale<Item> &other);
	
	// OutfitSale can be added into Sale by ignoring Sold and adding the outfit pointers.
	void Add(const OutfitSale<Item> &other);
	
	bool Has(const Item *item) const;
	
	Sale<Item>& operator= (const OutfitSale<Item>& outfitSale);
};



template <class Item>
void Sale<Item>::Load(const DataNode &node, const Set<Item> &items)
{
	for(const DataNode &child : node)
	{
		const std::string &token = child.Token(0);
		bool remove = (token == "clear" || token == "remove");
		if(remove && child.Size() == 1)
			this->clear();
		else if(remove && child.Size() >= 2)
			this->erase(items.Get(child.Token(1)));
		else if(token == "add" && child.Size() >= 2)
			this->insert(items.Get(child.Token(1)));
		else
			this->insert(items.Get(token));
	}
}



template <class Item>
void Sale<Item>::Add(const Sale<Item> &other)
{
	this->insert(other.begin(), other.end());
}



template <class Item>
void Sale<Item>::Add(const OutfitSale<Item> &outfitSale)
{
	for(auto& item : outfitSale)
		this->insert(item.first);
}



template <class Item>
bool Sale<Item>::Has(const Item *item) const
{
	return this->count(item);
}



template <class Item>
Sale<Item>& Sale<Item>::operator= (const OutfitSale<Item>& outfitSale)
{
	std::transform(outfitSale.cbegin(), outfitSale.cend(),
		std::inserter(*this, this->begin()),
		[](const std::pair<const Item*, Sold>& key_value)
		{ return key_value.first; });
	return *this;
}



// Class similar to Sale but used when items have local changes, it has
// their corresponding custom prices or showing/sellable types in the form of Sold.
template <class Item>
class OutfitSale : public std::map<const Item*, Sold> {
public:
	void Load(const DataNode &node, const Set<Item> &items);
	
	void Add(const OutfitSale<Item> &other);
	
	const Sold* GetSold(const Item* item) const;
	
	const double GetCost(const Item* item) const;
	
	const Sold::ShowSold GetShown(const Item* item) const;
	
	bool Has(const Item *item) const;
};



template <class Item>
void OutfitSale<Item>::Load(const DataNode &node, const Set<Item> &items)
{
	for(const DataNode &child : node)
	{
		const std::string &token = child.Token(0);
		bool remove = (token == "clear" || token == "remove");
		if(remove && child.Size() == 1)
			this->clear();
		else if(remove && child.Size() >= 2)
			this->erase(items.Get(child.Token(1)));
		else if(token == "add" && child.Size() >= 2)
			(*this)[items.Get(child.Token(1))].SetBase(child.Size() > 2 ? 
				child.Value(2) : 0., child.Size() > 3 ? child.Token(3) : "");
		else if(token == "hidden" || token == "import")
			for(const DataNode &subChild : child)
				(*this)[items.Get(subChild.Token(0))].SetBase(subChild.Size() > 1 ? 
					subChild.Value(1) : 0., token);
		else
			(*this)[items.Get(child.Token(0))].SetBase(child.Size() > 1 ? 
				child.Value(1) : 0., child.Size() > 2 ? child.Token(2) : "");
	}
}



// operator[] is used to override existing data instead, priorities are
// hidden > import > highest price
template <class Item>
void OutfitSale<Item>::Add(const OutfitSale<Item> &other)
{
	for(auto& it : other)
	{
		this->insert(it);
		const Sold* sold = GetSold(it.first);
		if(!sold)
			// This will not override existing items.
			this->insert(it);

		if(sold->GetShown() == it.second.GetShown())
			(*this)[it.first].SetCost(std::max(sold->GetCost(), it.second.GetCost()));
		else if(sold->GetShown() < it.second.GetShown())
			(*this)[it.first].SetBase(it.second.GetCost(), it.second.GetShow());
	}
}



template <class Item>
const Sold* OutfitSale<Item>::GetSold(const Item* item) const
{
	auto sold = this->find(item);
	return (sold != this->end()) ? &sold->second : nullptr;
}



template <class Item>
const double OutfitSale<Item>::GetCost(const Item* item) const
{
	const Sold* sold = GetSold(item);
	return sold ? sold->GetCost() : 0.;
}



template <class Item>
const Sold::ShowSold OutfitSale<Item>::GetShown(const Item* item) const
{
	const Sold* sold = GetSold(item);
	return sold ? sold->GetShown() : Sold::ShowSold::NONE;
}



template <class Item>
bool OutfitSale<Item>::Has(const Item *item) const
{
	return this->count(item);
}

#endif
