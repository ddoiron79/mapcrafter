/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "extended_ini.h"

#include <string>
#include <vector>
#include <set>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace config2 {

template <typename T>
class Field {
private:
	T value;
	bool loaded;
	bool valid;
public:
	Field(T value = T()) : value(value), loaded(false), valid(false) {}
	~Field() {}

	bool load(const ConfigSection& section, const std::string& key) {
		if (loaded)
			return true;
		if (section.has(key)) {
			value = section.get<T>(key);
			loaded = true;
			valid = true;
		} else
			loaded = false;
		return loaded;
	}

	bool load(const ConfigSection& section, const std::string& key, T default_value) {
		if (loaded)
			return true;
		value = section.get<T>(key, default_value);
		loaded = true;
		valid = true;
		return loaded;
	}

	bool require(ValidationList& validation, std::string message) {
		if (!loaded) {
			validation.push_back(ValidationMessage::error(message));
			return false;
		}
		return true;
	}

	bool validateOneOf(ValidationList& validation, std::string message, std::vector<T> values) {
		if (!loaded)
			return false;
		for (auto it = values.begin(); it != values.end(); ++it)
			if (value == *it)
				return true;
		return false;
	}

	T getValue() const { return value; }
	bool isLoaded() const { return loaded; }
	bool isValid() const { return valid; }
};

class WorldSection {
private:
	bool global;

	Field<fs::path> input_dir;
public:
	WorldSection(bool global = false) : global(global) {}
	~WorldSection() {}

	void setGlobal(bool global) { this->global = global; }
	bool parse(const ConfigSection& section, ValidationList& validation);

	fs::path getInputDir() const { return input_dir.getValue(); }
};

class MapSection {
private:
	bool global;

	std::string name_short, name_long;
	Field<std::string> world;

	Field<fs::path> texture_dir;
	Field<std::string> rotations;
	std::set<int> rotations_set;
	Field<std::string> rendermode;
	Field<int> texture_size;

	Field<bool> render_unknown_blocks, render_leaves_transparent, render_biomes;
public:
	MapSection(bool global = false)
		: global(global), texture_size(12),
		  render_unknown_blocks(false), render_leaves_transparent(false), render_biomes(false) {}
	~MapSection() {}

	void setGlobal(bool global) { this->global = global; }
	bool parse(const ConfigSection& section, ValidationList& validation);

	std::string getShortName() const { return name_short; }
	std::string getLongName() const { return name_long; }
	std::string getWorld() const { return world.getValue(); }

	fs::path getTextureDir() const { return texture_dir.getValue(); }
	std::set<int> getRotations() const { return rotations_set; }
	std::string getRendermode() const { return rendermode.getValue(); }
	int getTextureSize() const { return texture_size.getValue(); }

	bool renderUnknownBlocks() const { return render_unknown_blocks.getValue(); }
	bool renderLeavesTransparent() const { return render_leaves_transparent.getValue(); }
	bool renderBiomes() const { return render_biomes.getValue(); }
};

typedef std::vector<std::pair<std::string, ValidationList > > ValidationMap;

class ConfigParser {
private:
	WorldSection world_global;
	MapSection map_global;

	fs::path output_dir;

	std::map<std::string, WorldSection> worlds;
	std::vector<MapSection> maps;
public:
	ConfigParser() : world_global(true), map_global(true) {}
	~ConfigParser() {}

	bool parse(const std::string& filename, ValidationMap& validation);

	fs::path getOutputDir() const { return output_dir; }

	bool hasWorld(const std::string& world) const { return worlds.count(world); }
	const std::map<std::string, WorldSection>& getWorlds() const { return worlds; }
	const WorldSection& getWorld(const std::string& world) const { return worlds.at(world); }

	bool hasMap(const std::string& map) const;
	const std::vector<MapSection>& getMaps() const { return maps; }
	const MapSection& getMap(const std::string& map) const;
};

} /* namespace config */
} /* namespace mapcrafter */
#endif /* PARSER_H_ */
