/* CustomSale.cpp
Copyright (c) 2021 by Hurleveur

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "CustomSale.h"

#include "ConditionsStore.h"
#include "DataNode.h"
#include "GameData.h"
#include "Planet.h"
#include "Set.h"

#include <string>

using namespace std;

namespace
{
	const auto show = map<CustomSale::SellType, const string> {
		{CustomSale::SellType::DEFAULT, ""},
		{CustomSale::SellType::IMPORT, "import"}
	};
	// Default value, that should not conflict with actual values.
	const double DEFAULT = numeric_limits<double>::infinity();
}



void CustomSale::Load(const DataNode &node, const Set<Sale<Outfit>> &items, const Set<Outfit> &outfits)
{
	bool isAdd = false;
	const Outfit *outfit = nullptr;
	// Outfitters or outfits mode.
	const string mode = node.Token(1);
	auto parseValueOrOffset = [&isAdd, &outfit, &mode](double &amount, const DataNode &line) {
		int size = line.Size();
		// Default is 1, because we can just have an outfit defined here just to have a custom sellType.
		if(isAdd)
			amount += size > 2 ? line.Value(2) : 1.;
		else
			amount = size > 1 ? line.Value(1) : 1.;
		// % means it already is a relative price.
		// Otherwise it is a normal price that we must divide since they are stored as relative.
		// NOTE: this means that the offset is handled as relative to the existing modified price,
		// and not the default price (which is intended)!
		// Outfitter changes always are to be defined as relative in the data.
		if((mode != "outfitters" && (size == (2 + isAdd) ||
				(size > 2 && line.Token(2 + isAdd) != "%"))))
			amount /= outfit->Cost();
	};

	for(const DataNode &child : node)
	{
		const string &token = child.Token(0);

		bool isValue = token == "value";
		bool isOffset = token == "offset";

		bool remove = token == "remove";
		bool add = token == "add";

		int keyIndex = (add || remove);
		bool hasKey = child.Size() > 1;

		const string &key = child.Token(keyIndex);
		if(remove && !hasKey)
			Clear();
		else if(remove)
			if(key == "outfit")
			{
				// If an outfit is specified remove only that one. Otherwise clear all of them.
				if(child.Size() >= 3)
				{
					const Outfit *outfit = outfits.Get(child.Token(2));
					relativeOutfitPrices.erase(outfit);
					relativeOutfitOffsets.erase(outfit);
				}
				else
				{
					relativeOutfitOffsets.clear();
					relativeOutfitPrices.clear();
				}
			}
			else if(key == "outfitter")
			{
				// If an outfitter is specified remove only that one. Otherwise clear all of them.
				if(child.Size() >= 3)
				{
					const Sale<Outfit> *outfitter = items.Get(child.Token(2));
					relativePrices.erase(outfitter);
					relativeOffsets.erase(outfitter);
				}
				else
				{
					relativeOffsets.clear();
					relativePrices.clear();
				}
			}
			else if(key == "location")
				locationFilter = LocationFilter{};
			else if(key == "conditions")
				conditions = ConditionSet{};
			else
				child.PrintTrace("Skipping unrecognized clearing/deleting:");
		else if(add)
		{
			if(key == "location" && child.Size() == 1)
				locationFilter.Load(child);
			else if(key == "conditions")
				conditions.Load(child);
			else
				child.PrintTrace("Skipping unrecognized add:");
		}
		else if(key == "default")
			sellType = SellType::DEFAULT;
		else if(key == "import")
			sellType = SellType::IMPORT;
		else if(key == "location")
		{
			// Either just a planet or a whole filter.
			if(child.Size() == 1)
			{
				location = nullptr;
				locationFilter = LocationFilter{};
				locationFilter.Load(child);
			}
			else if(child.Size() == 2)
			{
				location = GameData::Planets().Get(child.Token(1));
				locationFilter = LocationFilter{};
				if(child.HasChildren())
					child.PrintTrace("Warning: location filter ignored due to use of explicit planet:");
			}
			else
				child.PrintTrace("Warning: use a location filter to choose from multiple planets:");
		}
		else if(key == "conditions")
		{
			// Override existing conditions and replace them.
			conditions = ConditionSet{};
			conditions.Load(child);
		}
		// CustomSales are separated between outfits and outfitters in the data files.
		// mode could apply to other things like shipyards and ships, later on.
		else if(mode == "outfits")
		{
			if(isValue || isOffset)
				for(const DataNode &grandChild : child)
				{
					isAdd = (grandChild.Token(0) == "add");
					outfit = outfits.Get(grandChild.Token(isAdd));

					if(isValue)
						parseValueOrOffset(relativeOutfitPrices[outfit], grandChild);
					else if(isOffset)
						parseValueOrOffset(relativeOutfitOffsets[outfit], grandChild);
				}
			// Default behavior assumes value.
			else if(child.Size() >= 1)
			{
				isAdd = (child.Token(0) == "add");
				outfit = outfits.Get(child.Token(isAdd));
				parseValueOrOffset(relativeOutfitPrices[outfit], child);
			}
			else
				child.PrintTrace("Skipping unrecognized (outfit assumed) attribute:");
		}
		else if(mode == "outfitters")
		{
			if(isValue || isOffset)
				for(const DataNode &grandChild : child)
				{
					isAdd = (grandChild.Token(0) == "add");
					const Sale<Outfit> *outfitter = items.Get(grandChild.Token(isAdd));

					if(isValue)
						parseValueOrOffset(relativePrices[outfitter], grandChild);
					else if(isOffset)
						parseValueOrOffset(relativeOffsets[outfitter], grandChild);
				}
			// Default behavior assumes value.
			else if(child.Size() >= 2)
			{
				const Sale<Outfit> *outfitter = items.Get(child.Token(0));
				relativePrices[outfitter] = child.Value(1);
			}
			else
				child.PrintTrace("Skipping unrecognized (outfitter assumed) attribute:");
		}
		else
			child.PrintTrace("Skipping unrecognized attribute:");
	}
}



bool CustomSale::Add(const CustomSale &other, const Planet &planet, const ConditionsStore &store)
{
	cacheValid = false;
	if(!Matches(planet, store))
		Clear();
	if(!other.Matches(planet, store))
		return false;

	// Selltypes are ordered by priority, a higher priority overrides lower ones.
	if(sellType < other.sellType)
	{
		*this = other;
		return true;
	}

	// The same logic applies for the relative prices or offsets, be they for  whole outfitters or outfits.

	// For prices, take the highest one.
	for(const auto &it : other.relativePrices)
	{
		auto ours = relativePrices.find(it.first);
		if(ours == relativePrices.end())
			relativePrices.emplace(it.first, it.second);
		else if(ours->second < it.second)
			ours->second = it.second;
	}
	// For offsets, add them to each others.
	for(const auto &it : other.relativeOffsets)
	{
		auto ours = relativeOffsets.find(it.first);
		if(ours == relativeOffsets.end())
			relativeOffsets.emplace(it.first, it.second);
		else
			ours->second += it.second;
	}
	// Same thing for the outfits.
	for(const auto &it : other.relativeOutfitPrices)
	{
		auto ours = relativeOutfitPrices.find(it.first);
		if(ours == relativeOutfitPrices.end())
			relativeOutfitPrices.emplace(it.first, it.second);
		else if(ours->second < it.second)
			ours->second = it.second;
	}
	for(const auto &it : other.relativeOutfitOffsets)
	{
		auto ours = relativeOutfitOffsets.find(it.first);
		if(ours == relativeOutfitOffsets.end())
			relativeOutfitOffsets.emplace(it.first, it.second);
		else
			ours->second += it.second;
	}
	return true;
}



double CustomSale::GetRelativeCost(const Outfit &item) const
{
	// Outfit prices have priority over outfitter prices, so consider them first,
	// and only consider the outfitter prices if the outfits have no set price.
	auto baseRelative = relativeOutfitPrices.find(&item);
	double baseRelativePrice = (baseRelative != relativeOutfitPrices.cend() ? baseRelative->second : DEFAULT);
	if(baseRelativePrice == DEFAULT)
		for(const auto &it : relativePrices)
			if(it.first->Has(&item))
			{
				baseRelativePrice = it.second;
				break;
			}
	auto baseOffset = relativeOutfitOffsets.find(&item);
	double baseOffsetPrice = (baseOffset != relativeOutfitOffsets.cend() ? baseOffset->second : DEFAULT);
	for(const auto &it : relativeOffsets)
		if(it.first->Has(&item))
		{
			if(baseOffsetPrice == DEFAULT)
				baseOffsetPrice = 0.;
			baseOffsetPrice += it.second;
		}
	// Apply the relative offset on top of each others, the result being applied on top of the relative price.
	// This means that an outfit can be affected by an outfitter offset, a custom outfit price, and outfit prices.
	if(baseRelativePrice != DEFAULT)
		return baseRelativePrice + (baseOffsetPrice != DEFAULT ? baseRelativePrice * baseOffsetPrice : 0.);
	else if(baseOffsetPrice != DEFAULT)
		return 1. + baseOffsetPrice;
	else
		return 1.;
}



CustomSale::SellType CustomSale::GetSellType() const
{
	return sellType;
}



const string &CustomSale::GetShown(CustomSale::SellType sellType)
{
	return show.find(sellType)->second;
}



const Sale<Outfit> &CustomSale::GetOutfits()
{
	if(!seen.empty() && cacheValid)
		return seen;
	else
		seen.clear();
	for(auto it : relativeOutfitPrices)
		seen.insert(it.first);
	for(auto it : relativeOutfitOffsets)
		seen.insert(it.first);
	for(auto &&sale : relativePrices)
		seen.Add(*sale.first);
	for(auto &&sale : relativeOffsets)
		seen.Add(*sale.first);
	cacheValid = true;
	return seen;
}



bool CustomSale::Has(const Outfit &item) const
{
	if(relativeOutfitPrices.find(&item) != relativeOutfitPrices.end())
		return true;
	if(relativeOutfitOffsets.find(&item) != relativeOutfitOffsets.end())
		return true;
	for(auto &&sale : relativePrices)
		if(sale.first->Has(&item))
			return true;
	for(auto &&sale : relativeOffsets)
		if(sale.first->Has(&item))
			return true;
	return false;
}



bool CustomSale::Matches(const Planet &planet, const ConditionsStore &playerConditions) const
{
	return (location ? location == &planet : locationFilter.Matches(&planet)) &&
		(conditions.IsEmpty() || conditions.Test(playerConditions));
}



bool CustomSale::IsEmpty()
{
	return GetOutfits().empty();
}



void CustomSale::Clear()
{
	*this = CustomSale{};
}
