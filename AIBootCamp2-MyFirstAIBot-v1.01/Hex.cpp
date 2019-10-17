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

int Hex::NbBorder(int maxQ, int maxR) const
{
	if (x == 0) {
		if (y == 0)
			return 4;
		if (y == (maxR - 1))
			return (y % 2) ? 3 : 4;
		return 2;
	}
	if (x == (maxQ - 1)) {
		if(x/2+y == (maxR-1))
			return (x % 2) ? 4 : 3;
		if(x/2+y == 0)
			return (x % 2) ? 3 : 4;
		return 2;
	}
	if (x/2+y == (maxQ -1))
		return (x % 2) ? 3 : 1;
	if (x / 2 + y == (maxQ - 1))
		return (x % 2) ? 1 : 3;
	return 0;
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
