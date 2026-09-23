#pragma once

class LocationPointManager;

class LocationPointSerializer
{
public:
	static bool Save(
		const std::string& path,
		const LocationPointManager& manager);

	static bool Load(
		const std::string& path,
		LocationPointManager& manager);
};