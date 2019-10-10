#include "Hex.h"
#include <cmath>

const std::vector<Hex> Hex::hexDirection = { Hex{0,-1,1}, Hex{-1,0,1}, Hex{-1, 1, 0}, Hex{0, 1, -1}, Hex{1, 0, -1}, Hex{1, -1, 0}, Hex{0, 0, 0} };

Hex::Hex(int q, int r) : x{q}, y{r}, z{-x-y}
{

}

Hex::Hex(int x, int y, int z) : x{x}, y{y}, z{z}
{
}

void Hex::GetAxial(int& q, int& r) const
{
	q = x;
	r = y;
}

int Hex::DistanceTo(const Hex hex) const
{
	return (std::abs(x - hex.x) + std::abs(y - hex.y) + std::abs(z - hex.z))/2;
}



Hex Hex::GetNeighbour(EHexCellDirection direction) const
{
	return *this + hexDirection[direction];
}

std::vector<Hex> Hex::GetNeighbours() const
{
	std::vector<Hex> neighbours;
	for (int i = 0; i < EHexCellDirection::CENTER ; ++i)
		neighbours.push_back(GetNeighbour(EHexCellDirection(i)));
	return neighbours;
}

EHexCellDirection Hex::ToDirection() const
{
	auto it = std::find(begin(hexDirection), end(hexDirection), *this);
	if (it != end(hexDirection))
		return EHexCellDirection(it - begin(hexDirection));

	throw new std::exception("Ce n'est pas une direction!");
}

Hex Hex::operator+(const Hex& hex) const
{
	return Hex{x + hex.x, y + hex.y, z + hex.z};
}

Hex Hex::operator-(const Hex& hex) const
{
	return Hex{x - hex.x, y - hex.y, z - hex.z};
}

bool Hex::operator==(const Hex& hex) const
{
	return x == hex.x && y == hex.y && z == hex.z;
}

bool Hex::operator!=(const Hex& hex) const
{
	return ! (*this == hex);
}

bool Hex::operator<(const Hex& hex) const
{
	if (x != hex.x)
		return x < hex.x;
	if (y != hex.y)
		return y < hex.y;
	return z < hex.z;
}

std::string Hex::toString() const
{
	return "x : " + std::to_string(x) + " , y : " + std::to_string(y) + " , z : " + std::to_string(z);
}
