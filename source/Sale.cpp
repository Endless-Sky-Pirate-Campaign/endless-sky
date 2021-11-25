/* Sale.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/


#include "Sale.h"

const std::map<Sold::ShowSold, const std::string> Sold::show{{Sold::ShowSold::DEFAULT, ""},
															 {Sold::ShowSold::IMPORT, "import"},
															 {Sold::ShowSold::HIDDEN, "hidden"},
															 {Sold::ShowSold::NONE, ""}};



const double Sold::GetCost() const 
{
	return cost;
}



const Sold::ShowSold Sold::GetShown() const
{
	return shown;
}



const std::string &Sold::GetShow() const
{
	/*
	auto see = show.find(shown);
	return (see != show.end()) ? see->second : show.find(ShowSold::NONE)->second;
	*/
	return show.find(shown)->second;
}



void Sold::SetBase(double cost, const std::string shown) 
{
	this->cost = cost;
	if(shown == "")
		this->shown = ShowSold::DEFAULT;
	else if(shown == "import")
		this->shown = ShowSold::IMPORT;
	else if(shown == "hidden")
		this->shown = ShowSold::HIDDEN;
}
