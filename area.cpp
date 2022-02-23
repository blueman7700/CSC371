/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: Oliver Morris - 979663

  This file contains the implementation for the Area class. Area is a relatively
  simple class that contains a local authority code, a container of names in
  different languages (perhaps stored in an associative container?) and a series
  of Measure objects (also in some form of container).
*/

#include <stdexcept>
#include <regex>

#include "area.h"

#define REGEX_ISO_639_3 "^[a-z]{3}$"

/*
  An alias for the imported JSON parsing library.
*/
using json = nlohmann::json;

/**
  Construct an Area with a given local authority code.

  @param local_authority_code
    The local authority code of the Area

  @example
    Area("W06000023");
*/
Area::Area(std::string &local_authority_code) noexcept : area_code(local_authority_code) {
	names.clear();
	measures.clear();
}

/**
 * Destructor for the Area object.
 */
Area::~Area() {
	this->area_code = "";
	names.clear();
	measures.clear();
}

/**
 * Updates the content of the lhs Area with the content of the rhs Area.
 * Any values with the same key will be overwritten by the values in the rhs object.
 *
 * @param other Area we are updating from.
 * @return Updated Area object.
 */
Area& Area::operator=(const Area &other) {

	std::string tmp;
	this->area_code = other.getLocalAuthorityCode();

	//update all name values from the rhs object. Conflicting items will be overwritten.
	for(const auto &it : other.names) {
		tmp = it.first;
		this->setName(tmp, it.second);
	}

	//update all measures from the rhs object. Conflicting items will be overwritten.
	for(const auto &it : other.measures) {
		this->setMeasure(it.first, it.second);
	}

	return *this;
}

/**
  Retrieve the local authority code for this Area. This function should be
  callable from a constant context and not modify the state of the instance.
  
  @return
    The Area's local authority code

  @example
    Area area("W06000023");
    ...
    auto authCode = area.getLocalAuthorityCode();
*/
std::string Area::getLocalAuthorityCode() const noexcept{
	return this->area_code;
}

/**
  Get a name for the Area in a specific language.  This function should be
  callable from a constant context and not modify the state of the instance.

  @param lang
    A three-letter language code in ISO 639-3 format, e.g. cym or eng

  @return
    The name for the area in the given language

  @throws
    std::out_of_range if lang does not correspond to a language of a name stored
    inside the Area instance

  @example
    Area area("W06000023");
    std::string langCode  = "eng";
    std::string langValue = "Powys";
    area.setName(langCode, langValue);
    ...
    auto name = area.getName(langCode);
*/
std::string Area::getName(const std::string &lang) const {
	try {
		return names.at(lang);
	} catch (std::out_of_range &e) {
		throw std::out_of_range("No Name found for key " + lang);
	}
}

/**
  Set a name for the Area in a specific language.

  @param lang
    A three-letter (alphabetical) language code in ISO 639-3 format,
    e.g. cym or eng, which should be converted to lowercase

  @param name
    The name of the Area in `lang`

  @throws
    std::invalid_argument if lang is not a three letter alphabetic code

  @example
    Area area("W06000023");
    std::string langCodeEnglish  = "eng";
    std::string langValueEnglish = "Powys";
    area.setName(langCodeEnglish, langValueEnglish);

    std::string langCodeWelsh  = "cym";
    std::string langValueWelsh = "Powys";
    area.setName(langCodeWelsh, langValueWelsh);
*/
void Area::setName(std::string lang, const std::string &name) {

	std::regex iso_639_3 (REGEX_ISO_639_3);
	std::transform(lang.begin(), lang.end(), lang.begin(), [](unsigned char c){return std::tolower(c);});

	//see if the language code matches the specified ISO-639-3 format.
	if(!std::regex_match(lang, iso_639_3)) {
		throw std::invalid_argument("Area::setName: Language code must be three alphabetical letters only");
	}

	auto did_insert = names.insert(std::pair<std::string, std::string>(lang, name));
	//check to see if there is an existing value that needs to be overwritten.
	if(!did_insert.second) {
		this->names[lang] = name;
	}
}

/**
  Retrieve a Measure object, given its codename. This function should be case
  insensitive when searching for a measure.

  @param key
    The codename for the measure you want to retrieve

  @return
    A Measure object

  @throws
    std::out_of_range if there is no measure with the given code, throwing
    the message:
    No measure found matching <codename>

  @example
    Area area("W06000023");
    Measure measure("Pop", "Population");
    area.setMeasure("Pop", measure);
    ...
    auto measure2 = area.getMeasure("pop");
*/
Measure& Area::getMeasure(const std::string &key) const {

	std::string lower_key = key;
	std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);

	try {
		return (Measure &) (measures.at(lower_key));
	} catch (std::out_of_range &e){
		throw std::out_of_range("No measure found matching " + lower_key);
	}
}

/**
  Add a particular Measure to this Area object. Note that the Measure's
  codename should be converted to lowercase.

  If a Measure already exists with the same codename in this Area, overwrite any
  values contained within the existing Measure with those in the new Measure
  passed into this function. The resulting Measure stored inside the Area
  instance should be a combination of the two Measures instances.

  @param key
    The codename for the Measure

  @param measure
    The Measure object

  @return
    void

  @example
    Area area("W06000023");

    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    double value = 12345678.9;
    measure.setValue(1999, value);

    area.setMeasure(code, measure);
*/
void Area::setMeasure(const std::string &key, const Measure &measure) {

	std::string tmp = key;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

	try {
		//try to get any measure that already exists at the key location.
		Measure &old = (this->getMeasure(tmp));

		//if we got this far then we need to update the existing measure with data from the new measure.
		old = measure;

	} catch (std::out_of_range &e) {
		measures.insert(std::pair<std::string, Measure>(tmp, measure));
	}
}

/**
  Retrieve the number of Measures we have for this Area. This function should be 
  callable from a constant context, not modify the state of the instance, and
  must promise not throw an exception.

  @return
    The size of the Area (i.e., the number of Measures)

  @example
    Area area("W06000023");
    std::string langCode  = "eng";
    std::string langValue = "Powys";
    area.setName(langCode, langValue);

    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    area.setMeasure(code, measure);
    auto size = area.size();
*/
int Area::size() const noexcept{
	return measures.size();
}

/**
  Overload the stream output operator as a free/global function.

  Output the name of the Area in English and Welsh, followed by the local
  authority code. Then output all the measures for the area (see the coursework
  worksheet for specific formatting).

  If the Area only has only one name, output this. If the area has no names,
  output the name "Unnamed".

  Measures should be ordered by their Measure codename. If there are no measures
  output the line "<no measures>" after you have output the area names.

  See the coursework specification for more examples.

  @param os
    The output stream to write to

  @param area
    Area to write to the output stream

  @return
    Reference to the output stream

  @example
    Area area("W06000023");
    area.setName("eng", "Powys");
    std::cout << area << std::endl;
*/
std::ostream& operator<<(std::ostream &os, const Area &obj) {

	std::string eng_name;
	bool has_eng = true;
	std::string cym_name;
	bool has_welsh = true;

	//get the name of the area in english.
	try {
		eng_name = obj.names.at("eng");
	} catch (std::out_of_range &e) {
		eng_name = "";
		has_eng = false;
	}

	//get the name of the area in welsh.
	try {
		cym_name = obj.names.at("cym");
	} catch (std::out_of_range &e) {
		cym_name = "";
		has_welsh = false;
	}

	//check what names the area has and print to screen.
	if(has_eng) {
		os << eng_name;
		if(has_welsh) {
			os << " / " << cym_name;
		}
	} else if (has_welsh){
		os << cym_name;
	} else {
		os << "Unnamed";
	}

	os << " (" << obj.getLocalAuthorityCode() << ")" << std::endl;

	//output all measures to the stream. If there are no measures then we output <no measures>.
	if (obj.size() == 0) {
		os << "<no measures>" << std::endl;
	} else {
		for(auto &it : obj.measures) {
			os << it.second << std::endl;
		}
	}

	return os;
}

/**
  Overload the == operator for two Area objects as a global/free function. Two
  Area objects are only equal when their local authority code, all names, and
  all data are equal.

  @param rhs
    A second Area object

  @return
    true if both Area instances have the same local authority code, names
    and data; false otherwise.

  @example
    Area area1("MYCODE1");
    Area area2("MYCODE1");

    bool eq = area1 == area2;
*/
bool Area::operator==(const Area &rhs) const{

	//check if the codes match.
	bool match_codes = this->getLocalAuthorityCode() == rhs.getLocalAuthorityCode();

	bool match_names = false;
	unsigned int num_matches = 0;

	//if the sizes do not match then they cannot have the same names.
	if(this->names.size() == rhs.names.size()) {
		try {
			//try and match all values from one object, if a key does not exist in the other object
			//then we need to catch the exception.
			for(const auto &it : this->names) {
				if(it.second == rhs.getName(it.first)) {
					num_matches++;
				}
			}
		} catch (std::out_of_range &e) {
			//if a key does not exist then the names do not match. The size will never be <0
			//so it will end up being false.
			num_matches = -1;
		}

		//check if all of the values matched.
		if(num_matches == this->names.size()) {
			match_names = true;
		}
	}

	num_matches = 0;
	bool match_data = false;

	//if the sizes do not match then they cannot have the same data.
	if(this->size() == rhs.size()) {
		try {
			//try and match all values from one object, if a key does not exist in the other object
			//then we need to catch the exception.
			for(const auto &it : this->measures) {
				if(rhs.getMeasure(it.first) == it.second) {
					num_matches++;
				}
			}
		} catch (std::out_of_range &e) {
			num_matches = -1;
		}

		if(num_matches == this->measures.size()) {
			match_data = true;
		}
	}

	return (match_codes && match_names && match_data);
}

/**
 * Converts the object into a JSON string and adds it to a JSON object.
 * This method is called automatically when trying to emplace this object
 * into a nlohmann::json object.
 *
 * @param j JSON object that the data will be attached to.
 * @param a Area object to be converted.
 */
void to_json(json& j, const Area& a) {
	json area_as_json;

	json names;
	json measures;

	//dump the names map into the json object. the json lib handles the conversion for us.
	for(auto &it : a.names) {
		names.emplace(it.first, it.second);
	}
	area_as_json.emplace("names", names);

	//dump the measures into the json object.
	//NOTE: couldn't write the to_json method the same way for the measure
	//		as i was getting an issue where another empty object was being
	//		created to encapsulate the measures.
	if(!a.measures.empty()) {
		for(auto &it : a.measures) {
			measures.emplace(it.first, it.second.getValuesAsJSON());
		}
		area_as_json.emplace("measures", measures);
	}

	//add the JSON representation of the area into the parent JSON object.
	j.emplace(a.getLocalAuthorityCode(), area_as_json);
}

/**
 * Checks to see if a given area exists within an area filter.
 *
 * @param area_filter Pointer to an unordered set of strings that contains the strings to match to the area.
 * @param area Area that will be checked.
 * @return True if the area exists in the filter, otherwise false.
 */
bool checkIfAreaMatchesFilter(const Area &area, const std::unordered_set<std::string> *filter) {

	bool match = false;
	std::string area_regex_pattern;
	std::regex regex;
	std::string area_auth_code;
	std::string curr_name;

	//if the filter is a null pointer or empty then match is set to true.
	if(filter == nullptr || filter->empty()) {
		match = true;
	} else {
		for(const auto &code : *filter) {

			//case insensitive regex pattern
			area_regex_pattern = "^.*" + code +".*$";
			regex.assign(area_regex_pattern, std::regex_constants::icase);

			//check if the area code matches the regex pattern.
			if(std::regex_match(area.area_code, regex)) {
				match = true;
			} else {
				//check if any of the names match the regex pattern.
				for(const auto &name : area.names) {
					curr_name = name.second;
					if(std::regex_match(curr_name, regex)) {
						match = true;
					}
				}
			}
		}
	}

	return match;
}