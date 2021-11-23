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



const double Sold::GetCost() const 
{
	return cost;
}



const std::string &Sold::GetShown() const
{
	return shown;
}



void Sold::SetBase(double cost, const std::string shown) 
{
	this->cost = cost;
	this->shown = shown;
}
