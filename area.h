#ifndef AREA_H_
#define AREA_H_

/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: Oliver Morris - 979663

  This file contains the Area class declaration. Area objects contain all the
  Measure objects for a given local area, along with names for that area and a
  unique authority code.
 */

#include <string>
#include <map>
#include <unordered_set>

#include "lib_json.hpp"
#include "measure.h"

/*
  An Area object consists of a unique authority code, a container for names
  for the area in any number of different languages, and a container for the
  Measures objects.
*/
class Area {
 private:
	std::string area_code;
	std::map<std::string, std::string> names;
	std::map<std::string, Measure> measures;

 public:
	explicit Area(std::string &local_authority_code) noexcept;
	~Area();
	Area& operator=(const Area &other);
	std::string getLocalAuthorityCode() const noexcept;
	std::string getName(const std::string &lang) const;
	void setName(std::string lang, const std::string &name);
	Measure& getMeasure(const std::string &key) const;
	void setMeasure(const std::string &key, const Measure &measure);
	int size() const noexcept;
	friend std::ostream& operator<<(std::ostream &os, const Area &obj);
	bool operator==(const Area &rhs) const;
	friend void to_json(nlohmann::json& j, const Area& a);
	friend bool checkIfAreaMatchesFilter(const Area &area, const std::unordered_set<std::string> *filter);
};

#endif // AREA_H_