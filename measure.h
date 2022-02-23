#ifndef MEASURE_H_
#define MEASURE_H_

/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: Oliver Morris - 979663

  This file contains the decalaration of the Measure class.
 */

#include <string>
#include <map>
#include <memory>

#include "lib_json.hpp"

/*
  The Measure class contains a measure code, label, and a container for readings
  from across a number of years.
*/
class Measure {
 private:
	std::string code;
	std::string label;
	std::map<unsigned int, double> values;

 public:
  Measure(const std::string &code, const std::string &label) noexcept;
  ~Measure();
  Measure& operator=(const Measure &other);
  std::string getCodename() const noexcept;
  std::string getLabel() const noexcept;
  void setLabel(const std::string &_label);
  double getValue(const unsigned int &key) const;
  void setValue(const unsigned int &key, const double &value);
  int size() const noexcept;
  double getDifference() const noexcept;
  double getDifferenceAsPercentage() const noexcept;
  double getAverage() const noexcept;
  friend std::ostream& operator<<(std::ostream &os, const Measure &obj);
  bool operator==(const Measure &rhs) const;
  nlohmann::json getValuesAsJSON() const;

};

#endif // MEASURE_H_