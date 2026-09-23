#pragma once

enum class LocationTag
{
	PlayerHouse,
	NeighborHouse1,
	NeighborHouse2,
	NeighborHouse3,
	PoliceStation,
	Park,
	ConstructionSite,
	TrashDump,

	Max
};

inline const char* LocationTagToString(LocationTag tag)
{
	static const char* names[] =
	{
		"PlayerHouse",
		"NeighborHouse1",
		"NeighborHouse2",
		"NeighborHouse3",
		"PoliceStation",
		"Park",
		"ConstructionSite",
		"TrashDump",
	};

	int index = static_cast<int>(tag);

	if (index < 0 || index >= static_cast<int>(LocationTag::Max))
	{
		return "Unknown";
	}

	return names[index];

}

inline bool IsUniqueLocationTag(LocationTag tag)
{
	switch (tag)
	{
	case LocationTag::PlayerHouse:
	case LocationTag::NeighborHouse1:
	case LocationTag::NeighborHouse2:
	case LocationTag::NeighborHouse3:
	case LocationTag::PoliceStation:
		return true;

	default:
		return false;
	}
}