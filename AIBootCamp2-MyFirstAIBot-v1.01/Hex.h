#ifndef HEX_H
#define HEX_H

#include<vector>
#include<algorithm>
#include <exception>
#include "Globals.h"
#include <string>

class Hex
{

public:

	const static std::vector<Hex> hexDirection;

	int x, y, z;

	Hex() = default;
	Hex(int q, int r);
	Hex(int x, int y, int z);

	void GetAxial(int& q, int& r) const;
	int DistanceTo(const Hex hex) const;
	int NbBorder(int maxQ, int maxR) const;
	Hex GetNeighbour(EHexCellDirection direction) const;
	std::vector<Hex> GetNeighbours() const;

	EHexCellDirection ToDirection() const;

	Hex operator+ (const Hex& hex) const;
	Hex operator- (const Hex& hex) const;
	bool operator== (const Hex& hex) const;
	bool operator!= (const Hex& hex) const;
	bool operator< (const Hex& hex) const;

	std::string toString() const;

};

namespace std
{
	template<> struct hash<Hex>
	{
		typedef Hex argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& hex) const noexcept
		{
			result_type const h1(std::hash<int>{}(hex.x));
			result_type const h2(std::hash<int>{}(hex.y));
			result_type const h3(std::hash<int>{}(hex.z));
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};
}



#endif
