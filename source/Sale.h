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



class Sold {
public:
	const double GetCost() const;
	const std::string &GetShown() const;
	void SetBase(double cost = 0., std::string shown = "");

	double cost = 0.;
	std::string shown = "";
};



template <class Item>
class OutfitSale;



// Class representing a set of items that are for sale on a given planet.
// Multiple sale sets can be merged together into a single one.
template <class Item>
class Sale : public std::set<const Item *> {
public:
	void Load(const DataNode &node, const Set<Item> &items);
	
	void Add(const Sale<Item> &other);

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



template <class Item>
class OutfitSale : public std::map<const Item*, Sold> {
public:
	void Load(const DataNode &node, const Set<Item> &items);
	
	void Add(const OutfitSale<Item> &other);
	
	Sold* GetSold(const Item* item) const;
	
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
			(*this)[items.Get(child.Token(1))].SetBase(child.Size() > 2 ? child.Value(2) : 0., child.Size() > 3 ? child.Token(3) : "");
		else
			(*this)[items.Get(child.Token(0))].SetBase(child.Size() > 1 ? child.Value(1) : 0., child.Size() > 2 ? child.Token(2) : "");
	}
}



template <class Item>
void OutfitSale<Item>::Add(const OutfitSale<Item> &other)
{
	this->insert(other.begin(), other.end());
}



template <class Item>
Sold* OutfitSale<Item>::GetSold(const Item* item) const
{
	return &const_cast<OutfitSale<Item> *>(this)->operator[](item);
}



template <class Item>
bool OutfitSale<Item>::Has(const Item *item) const
{
	return this->count(item);
}

#endif
