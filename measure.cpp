/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: Oliver Morris - 979663

  This file contains the implementation of the Measure class. Measure is a
  very simple class that needs to contain a few member variables for its name,
  codename, and a Standard Library container for data. The data you need to 
  store is values, organised by year. I'd recommend storing the values as 
  doubles.
*/

#include <stdexcept>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "measure.h"

using json = nlohmann::json;

/**
  Construct a single Measure, that has values across many years.

  All StatsWales JSON files have a codename for measures. You should convert 
  all codenames to lowercase.

  @param codename
    The codename for the measure

  @param label
    Human-readable (i.e. nice/explanatory) label for the measure

  @example
    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);
*/
Measure::Measure(const std::string &_codename, const std::string &_label) noexcept : label(_label)  {

	std::string tmp = _codename;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
	code = tmp;
	values.clear();
}

/**
 * Destructor for the Measure object.
 */
Measure::~Measure() {
	code = "";
	values.clear();
}

/**
 * Updates the content of the lhs Measure with the content of the rhs Measure.
 * Any values with the same key will be overwritten by the values in the rhs object.
 *
 * @param other Measure we are updating from.
 * @return Updated measure object.
 */
Measure& Measure::operator=(const Measure &other) {
	this->code = other.getCodename();
	this->label = other.getLabel();

	//place all values from the rhs into the value map. conflicting items will be overwritten.
	for(auto &it : other.values) {
		this->setValue(it.first, it.second);
	}

	return *this;
}

/**
  Retrieve the code for the Measure. This function should be callable from a
  constant context and must promise to not modify the state of the instance or 
  throw an exception.

  @return
    The codename for the Measure

  @example
    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    measure.setValue(1999, 12345678.9);
    ...
    auto codename2 = measure.getCodename();
*/
std::string Measure::getCodename() const noexcept{
	return code;
}

/**
  Retrieve the human-friendly label for the Measure. This function should be
  callable from a constant context and must promise to not modify the state of 
  the instance and to not throw an exception.

  @return
    The human-friendly label for the Measure

  @example
    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    measure.setValue(1999, 12345678.9);
    ...
    auto label = measure.getLabel();
*/
std::string Measure::getLabel() const noexcept{
	return label;
}

/**
  Change the label for the Measure.

  @param label
    The new label for the Measure

  @example
    Measure measure("pop", "Population");
    measure.setValue(1999, 12345678.9);
    ...
    measure.setLabel("New Population");
*/
void Measure::setLabel(const std::string &_label) {
	label = _label;
}

/**
  Retrieve a Measure's value for a given year.

  @param key
    The year to find the value for

  @return
    The value stored for the given year

  @throws
    std::out_of_range if year does not exist in Measure with the message
    No value found for year <year>

  @return
    The value

  @example
    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    measure.setValue(1999, 12345678.9);
    ...
    auto value = measure.getValue(1999); // returns 12345678.9
*/
double Measure::getValue(const unsigned int &key) const {
	try {
		return values.at(key);
	} catch (std::out_of_range &e) {
		throw std::out_of_range("No value found for year " + std::to_string(key));
	}
}

/**
  Add a particular year's value to the Measure object. If a value already
  exists for the year, replace it.

  @param key
    The year to insert a value at

  @param value
    The value for the given year

  @return
    void

  @example
    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    measure.setValue(1999, 12345678.9);
*/
void Measure::setValue(const unsigned int &key, const double &value) {

	auto did_insert = values.insert(std::pair<unsigned int, double>(key, value));
	//check to see if we need to overwrite existing data.
	if(!did_insert.second) {
		this->values[key] = value;
	}
}

/**
  Retrieve the number of years data we have for this measure. This function
  should be callable from a constant context and must promise to not change
  the state of the instance or throw an exception.

  @return
    The size of the measure

  @example
    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    measure.setValue(1999, 12345678.9);
    auto size = measure.size(); // returns 1
*/
int Measure::size() const noexcept{
	return values.size();
}

/**
  Calculate the difference between the first and last year imported. This
  function should be callable from a constant context and must promise to not
  change the state of the instance or throw an exception.

  @return
    The difference/change in value from the first to the last year, or 0 if it
    cannot be calculated

  @example
    Measure measure("pop", "Population");
    measure.setValue(1999, 12345678.9);
    measure.setValue(2010, 12345679.9);
    auto diff = measure.getDifference(); // returns 1.0
*/
double Measure::getDifference() const noexcept{
	if(this->size() > 1) {
		double first = values.begin()->second;
		double last = (--values.end())->second;

		return last - first;
	} else {
		return 0.0;
	}
}

/**
  Calculate the difference between the first and last year imported as a
  percentage. This function should be callable from a constant context and
  must promise to not change the state of the instance or throw an exception.

  @return
    The difference/change in value from the first to the last year as a decimal
    value, or 0 if it cannot be calculated

  @example
    Measure measure("pop", "Population");
    measure.setValue(1990, 12345678.9);
    measure.setValue(2010, 12345679.9);
    auto diff = measure.getDifferenceAsPercentage();
*/
double Measure::getDifferenceAsPercentage() const noexcept{
	if(this->size() > 1) {
		double first = values.begin()->second;
		double last = (--values.end())->second;

		// % diff is calculated with:
		// ((last - first) / |first|) * 100
		return ((last - first) / std::abs(first)) * 100.0;
	} else {
		return 0.0;
	}
}

/**
  Calculate the average/mean value for all the values. This function should be
  callable from a constant context and must promise to not change the state of 
  the instance or throw an exception.

  @return
    The average value for all the years, or 0 if it cannot be calculated

  @example
    Measure measure("pop", "Population");
    measure.setValue(1999, 12345678.9);
    measure.setValue(1999, 12345679.9);
    auto diff = measure.getDifference(); // returns 1
*/
double Measure::getAverage() const noexcept{
	double rolling_average = 0;

	for(auto it : values) {
		rolling_average += it.second;
	}

	return rolling_average / values.size();
}

/**
  Overload the << operator to print all of the Measure's imported data.

  We align the year and value outputs by padding the outputs with spaces,
  i.e. the year and values should be right-aligned to each other so they
  can be read as a table of numerical values.

  Years should be printed in chronological order. Three additional columns
  should be included at the end of the output, corresponding to the average
  value across the years, the difference between the first and last year,
  and the percentage difference between the first and last year.

  If there is no data in this measure, print the name and code, and 
  on the next line print: <no data>

  See the coursework specification for more information.

  @param os
    The output stream to write to

  @param measure
    The Measure to write to the output stream

  @return
    Reference to the output stream

  @example
    std::string codename = "Pop";
    std::string label = "Population";
    Measure measure(codename, label);

    measure.setValue(1999, 12345678.9);
    std::cout << measure << std::end;
*/
std::ostream& operator<<(std::ostream &os, const Measure &obj) {

	os << obj.getLabel() << " (" << obj.getCodename() << ")" << std::endl;

	std::stringstream title_stream;
	std::stringstream value_stream;

	value_stream << std::fixed << std::setprecision(6);
	unsigned int space;

	for(auto it : obj.values) {
		//leading space is the number of digits after decimal place, plus 6 decimal spaces.
		//also must include space for decimal point.
		space = std::to_string((int) it.second).length() + 7;

		title_stream << std::setw(space) << it.first << " ";
		value_stream << it.second << " ";
	}

	//format and add the Average title to the title stream
	space = std::to_string((int) obj.getAverage()).length() + 7;
	title_stream << std::setw(space) << "Average" << " ";

	//format and add the difference title to the title stream.
	space = std::to_string((int) obj.getDifference()).length() + 7;
	title_stream << std::setw(space) << "Diff." << " ";

	//format and add the percentage difference title to the title stream.
	space = std::to_string((int) obj.getDifferenceAsPercentage()).length() + 7;
	title_stream << std::setw(space) << "% Diff.";

	//add all extra values to the value stream.
	value_stream << obj.getAverage() << " ";
	value_stream << obj.getDifference() << " ";
	value_stream << obj.getDifferenceAsPercentage();

	//convert our string streams into strings and push to ostream.
	os << title_stream.str() << std::endl;
	os << value_stream.str() << std::endl;

	return os;
}

/**
  Overload the == operator for two Measure objects. Two Measure objects
  are only equal when their codename, label and data are all equal.

  @param lhs
    A Measure object

  @param lhs
    A second Measure object

  @return
    true if both Measure objects have the same codename, label and data; false
    otherwise
*/
bool Measure::operator==(const Measure &rhs) const {

	bool match_code;
	bool match_label;
	bool match_data = true;

	//check to see if the code and labels match.
	match_code = (this->code == rhs.getCodename());
	match_label = (this->label == rhs.getLabel());

	//check to see if the data held in the value map is equal;
	try {
		//if the size is not the same then they cannot have the same values.
		if(this->size() == rhs.size()) {
			for(auto it : this->values) {
				if(it.second != rhs.getValue(it.first)) {
					match_data = false;
				}
			}
		} else {
			match_data = false;
		}
	//if a std::out_of_range exception in thrown then one value does not exist in the other map.
	} catch (std::out_of_range &e) {
		match_data = false;
	}

	return match_code && match_label && match_data;
}

/**
 * Convert the measure object into a JSON object.
 * @return JSON representation of the Measure.
 */
json Measure::getValuesAsJSON() const {
	json json_values;

	std::string y;
	for(auto &it : values) {
		y = std::to_string(it.first);
		json_values.emplace(y, it.second);
	}

	return json_values;
}
