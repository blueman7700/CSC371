/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: Oliver Morris - 979663

  The file contains the Areas class implementation. Areas are the top
  level of the data structure in Beth Yw? for now.

  Areas is also responsible for importing data from a stream (using the
  various populate() functions) and creating the Area and Measure objects.
*/

#include <stdexcept>
#include <iostream>
#include <string>
#include <tuple>
#include <regex>

#include "lib_json.hpp"
#include "datasets.h"
#include "areas.h"

/*
  An alias for the imported JSON parsing library.
*/
using json = nlohmann::json;

/**
  Constructor for an Areas object.

  @example
    Areas data = Areas();
*/
Areas::Areas() {
	areas_container.clear();
}

/**
  Destructor for the Areas object.
 */
Areas::~Areas() {
	areas_container.clear();
}

/**
  Add a particular Area to the Areas object.

  If an Area already exists with the same local authority code, overwrite all
  data contained within the existing Area with those in the new
  Area (i.e. they should be combined, but the new Area's data should take
  precedence, e.g. replace a name with the same language identifier).

  @param key
    The local authority code of the Area

  @param value
    The Area object that will contain the Measure objects

  @return
    void

  @example
    Areas data = Areas();
    std::string localAuthorityCode = "W06000023";
    Area area(localAuthorityCode);
    data.setArea(localAuthorityCode, area);
*/
void Areas::setArea(const std::string &auth_code, const Area &area) {

	try {
		//try to get any area that already exists at the key location.
		Area &old = (this->getArea(auth_code));

		//if we got this far then we need to update the existing area with data from the new area.
		old = area;

	} catch (std::out_of_range &e) {
		areas_container.insert(std::pair<std::string, Area>(auth_code, area));
	}
}

/**
   Retrieve an Area instance with a given local authority code.

  @param key
    The local authority code to find the Area instance of

  @return
    An Area object

  @throws
    std::out_of_range if an Area with the set local authority code does not
    exist in this Areas instance

  @example
    Areas data = Areas();
    std::string localAuthorityCode = "W06000023";
    Area area(localAuthorityCode);
    data.setArea(localAuthorityCode, area);
    ...
    Area area2 = areas.getArea("W06000023");
*/
Area &Areas::getArea(const std::string &auth_code) const {
	try {
		return (Area &) (areas_container.at(auth_code));
	} catch (std::out_of_range &e) {
		throw std::out_of_range("No area found matching " + auth_code);
	}
}

/**
  Retrieve the number of Areas within the container. This function should be
  callable from a constant context, not modify the state of the instance, and
  must promise not throw an exception.

  @return
    The number of Area instances

  @example
    Areas data = Areas();
    std::string localAuthorityCode = "W06000023";
    Area area(localAuthorityCode);
    data.setArea(localAuthorityCode, area);
    
    auto size = areas.size(); // returns 1
*/
int Areas::size() const {
	return areas_container.size();
}

/**
  This function specifically parses the compiled areas.csv file of local 
  authority codes, and their names in English and Welsh.

  This is a simple dataset that is a comma-separated values file (CSV), where
  the first row gives the name of the columns, and then each row is a set of
  data.

  For this coursework, you can assume that areas.csv will always have the same
  three columns in the same order.

  Once the data is parsed, you need to create the appropriate Area objects and
  insert them in to a Standard Library container within Areas.

  @param is
    The input stream from InputSource

  @param cols
    A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
    that give the column header in the CSV file

  @param areasFilter
    An umodifiable pointer to set of umodifiable strings for areas to import,
    or an empty set if all areas should be imported

  @return
    void

  @see
    See datasets.h for details of how the variable cols is organised

  @see
    See bethyw.cpp for details of how the variable areasFilter is created

  @example
    InputFile input("data/areas.csv");
    auto is = input.open();

    auto cols = InputFiles::AREAS.COLS;

    auto areasFilter = BethYw::parseAreasArg();

    Areas data = Areas();
    areas.populateFromAuthorityCodeCSV(is, cols, &areasFilter);

  @throws 
    std::runtime_error if a parsing error occurs (e.g. due to a malformed file)
    std::out_of_range if there are not enough columns in cols
*/
void Areas::populateFromAuthorityCodeCSV(
	std::istream &is,
	const BethYw::SourceColumnMapping &cols,
	const StringFilterSet *const areasFilter) {

	char delimiter = ',';
	std::string line_buff;
	std::string value;
	std::string tmp;
	bool error = false;

	//get the first line of the file and place into a string stream.
	std::getline(is, line_buff);
	std::stringstream line_stream(line_buff);

	try {
		//check if the local authority column is present in the file.
		tmp = cols.at(BethYw::SourceColumn::AUTH_CODE);
		std::getline(line_stream, value, delimiter);
		//std::cout << value << std::endl;
		if (value != tmp) {
			error = true;
		}

		//check if the english area name is present in the file.
		tmp = cols.at(BethYw::SourceColumn::AUTH_NAME_ENG);
		std::getline(line_stream, value, delimiter);
		//std::cout << value << std::endl;
		if (value != tmp) {
			error = true;
		}

		//check if the welsh area name is present in the file.
		tmp = cols.at(BethYw::SourceColumn::AUTH_NAME_CYM);
		std::getline(line_stream, value, delimiter);
		//std::cout << value << std::endl;
		if (value != tmp) {
			error = true;
		}
	} catch (std::out_of_range &e) {
		throw std::out_of_range("Not enough entries in cols. Expected 3 entries but only found :"
									+ std::to_string(cols.size()));
	}

	if (error) {
		//if the column headers dont match then we have an issue
		throw std::runtime_error("Malformed File!");
	}

	//if the filter is null or empty then we load everything.
	bool load_all = (areasFilter == nullptr || areasFilter->empty());

	while (std::getline(is, line_buff)) {

		//clear the stream after every iteration.
		line_stream.str("");
		line_stream.clear();

		//push current line into string stream.
		line_stream << line_buff;

		//get local authority code from file
		std::getline(line_stream, value, delimiter);
		Area a = Area(value);

		//get english name from file.
		std::getline(line_stream, value, delimiter);
		tmp = "eng";
		a.setName(tmp, value);

		//get welsh name from file.
		std::getline(line_stream, value, delimiter);
		tmp = "cym";
		a.setName(tmp, value);

		//check to see if the current area needs to be inserted into the map.
		if (!load_all) {
			if (checkIfAreaMatchesFilter(a, areasFilter)) {
				this->setArea(a.getLocalAuthorityCode(), a);
			}
		} else {
			this->setArea(a.getLocalAuthorityCode(), a);
		}
	}

	//clean up variables.
	line_buff = "";
	value = "";
	line_stream.str("");
	line_stream.clear();
}

/**
  Data from StatsWales is in the JSON format, and contains three
  top-level keys: odata.metadata, value, odata.nextLink. value contains the
  data we need. Rather than been hierarchical, it contains data as a
  continuous list (e.g. as you would find in a table). For each row in value,
  there is a mapping of various column headings and their respective values.

  Therefore, you need to go through the items in value (in a loop)
  using a JSON library. To help you, I've selected the nlohmann::json
  library that you must use for your coursework. Read up on how to use it here:
  https://github.com/nlohmann/json

  Example of using this library:
    - Reading/parsing in from a stream is very simply using the >> operator:
        json j;
        stream >> j;

    - Looping through parsed JSON is done with a simple for each loop. Inside
      the loop, you can access each using the array syntax, with the key/
      column name, e.g. data["Localauthority_ItemName_ENG"] gives you the
      local authority name:
        for (auto& el : j["value"].items()) {
           auto &data = el.value();
           std::string localAuthorityCode = data["Localauthority_ItemName_ENG"];
           // do stuff here...
        }

  In this function, you will have to parse the JSON datasets, extracting
  the local authority code, English name (the files only contain the English
  names), and each measure by year.

  If you encounter an Area that does not exist in the Areas container, you
  should create the Area object

  If areasFilter is a non-empty set only include areas matching the filter. If
  measuresFilter is a non-empty set only include measures matching the filter.
  If yearsFilter is not equal to <0,0>, only import years within the range
  specified by the tuple (inclusive).

  I've provided the column names for each JSON file that you need to parse
  as std::strings in datasets.h. This mapping should be passed through to the
  cols parameter of this function.

  Note that in the JSON format, years are stored as strings, but we need
  them as ints. When retrieving values from the JSON library, you will
  have to cast them to the right type.

  @param is
    The input stream from InputSource

  @param cols
    A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
    that give the column header in the CSV file

  @param areasFilter
    An umodifiable pointer to set of umodifiable strings of areas to import,
    or an empty set if all areas should be imported

  @param measuresFilter
    An umodifiable pointer to set of umodifiable strings of measures to import,
    or an empty set if all measures should be imported

  @param yearsFilter
    An umodifiable pointer to an umodifiable tuple of two unsigned integers,
    where if both values are 0, then all years should be imported, otherwise
    they should be treated as the range of years to be imported (inclusively)

  @return
    void

  @throws 
    std::runtime_error if a parsing error occurs (e.g. due to a malformed file)
    std::out_of_range if there are not enough columns in cols

  @see
    See datasets.h for details of how the variable cols is organised

  @see
    See bethyw.cpp for details of how the variable areasFilter is created

  @example
    InputFile input("data/popu1009.json");
    auto is = input.open();

    auto cols = InputFiles::DATASETS["popden"].COLS;

    auto areasFilter = BethYw::parseAreasArg();
    auto measuresFilter = BethYw::parseMeasuresArg();
    auto yearsFilter = BethYw::parseMeasuresArg();

    Areas data = Areas();
    areas.populateFromWelshStatsJSON(
      is,
      cols,
      &areasFilter,
      &measuresFilter,
      &yearsFilter);
*/
void Areas::populateFromWelshStatsJSON(
	std::istream &is,
	const BethYw::SourceColumnMapping &cols,
	const StringFilterSet *const areasFilter,
	const StringFilterSet *const measuresFilter,
	const YearFilterTuple *const yearsFilter)
noexcept(false) {

	//convert the stream into a JSON object.
	json j;
	is >> j;

	unsigned int year_range_start,
		year_range_end,
		current_year;

	std::string current_local_auth_code,
		current_area_name_eng,
		current_measure_code,
		current_measure_label,
		measure_code_tag,
		measure_label_tag,
		tmp;

	double current_value;

	//if the filters are null or empty then we load everything.
	bool load_all_areas = (areasFilter == nullptr || areasFilter->empty());
	bool load_all_measures = (measuresFilter == nullptr || measuresFilter->empty());

	bool load_all_years = false;
	bool should_load;
	bool match;

	//check to see if all years need to be loaded.
	if (yearsFilter != nullptr) {
		year_range_start = std::get<0>(*yearsFilter);
		year_range_end = std::get<1>(*yearsFilter);
		if (year_range_end == 0) {
			load_all_years = true;
		}
	} else {
		load_all_years = true;
	}

	//iterate through every element in the JSON object.
	for (auto &el : j["value"].items()) {
		auto &data = el.value();

		should_load = true;
		try {
			current_local_auth_code = data[cols.at(BethYw::SourceColumn::AUTH_CODE)];
			current_area_name_eng = data[cols.at(BethYw::SourceColumn::AUTH_NAME_ENG)];

			//year value is stored as a string so we need to convert to a u_int.
			tmp = data[cols.at(BethYw::SourceColumn::YEAR)];
			current_year = std::stoi(tmp);

			//check if the value is stored as a string or a double in the file.
			//for some reason aqi values are stored as strings and not doubles???
			if (data[cols.at(BethYw::SourceColumn::VALUE)].is_string()) {
				tmp = data[cols.at(BethYw::SourceColumn::VALUE)];
				current_value = std::stod(tmp);
			} else {
				current_value = data[cols.at(BethYw::SourceColumn::VALUE)];
			}

			//measure code could be held in either MEASURE_CODE or SINGLE_MEASURE_CODE, so we try both.
			try {
				measure_code_tag = cols.at(BethYw::SourceColumn::MEASURE_CODE);
				measure_label_tag = cols.at(BethYw::SourceColumn::MEASURE_NAME);

				current_measure_code = data[measure_code_tag];
				current_measure_label = data[measure_label_tag];
			} catch (std::out_of_range &e1) {
				current_measure_code = cols.at(BethYw::SourceColumn::SINGLE_MEASURE_CODE);
				current_measure_label = cols.at(BethYw::SourceColumn::SINGLE_MEASURE_NAME);
			}

			//set the measure code to lowercase for ease of use.
			std::transform(current_measure_code.begin(),
						   current_measure_code.end(),
						   current_measure_code.begin(),
						   ::tolower);

		} catch (std::out_of_range &e2) {
			throw std::out_of_range("Not enough columns in cols!");
		}

		//check to see if we should load the data from this entry.
		if (!(load_all_years || ((year_range_start <= current_year) && (current_year <= year_range_end)))) {
			should_load = false;
		}

		//check if the current value exists in the measure filter
		if (!load_all_measures) {
			match = false;
			for (auto &it : *measuresFilter) {

				//convert the key to lowercase to make matching easier.
				tmp = it;
				std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
				if (tmp == current_measure_code) {
					match = true;
				}
			}
			should_load &= match;
		}

		if (should_load) {
			try {
				//try and get the area, if it does not exist then catch the error and make a new one.
				Area &a = this->getArea(current_local_auth_code);

				//check to see if the current area exists in the filter
				if (load_all_areas || checkIfAreaMatchesFilter(a, areasFilter)) {
					//check to see if the measure exists in the area. if not then we create one.
					try {
						a.getMeasure(current_measure_code).setValue(current_year, current_value);
					} catch (std::out_of_range &e) {
						Measure new_measure = Measure(current_measure_code, current_measure_label);
						new_measure.setValue(current_year, current_value);
						a.setMeasure(new_measure.getCodename(), new_measure);
					}
				}
			} catch (std::out_of_range &e) {
				//create a new area from the data parsed from the JSON file.
				Area new_area = Area(current_local_auth_code);
				tmp = "eng";
				new_area.setName(tmp, current_area_name_eng);

				//check to see if this new area exists in the area filter.
				if (load_all_areas || checkIfAreaMatchesFilter(new_area, areasFilter)) {
					//no need to check if the measure exists because the area has only just been created.
					Measure new_measure = Measure(current_measure_code, current_measure_label);
					new_measure.setValue(current_year, current_value);
					new_area.setMeasure(new_measure.getCodename(), new_measure);

					//add this new area to the areas map
					this->setArea(current_local_auth_code, new_area);
				}
			}
		}
	}

	//clean up variables. Not exactly necessary but better safe than sorry.
	current_local_auth_code = "";
	current_area_name_eng = "";
	current_measure_code = "";
	current_measure_label = "";
	measure_code_tag = "";
	measure_label_tag = "";
	tmp = "";

	year_range_start = 0;
	year_range_end = 0;
	current_year = 0;
	current_year = 0;
}

/**
  This function imports CSV files that contain a single measure. The 
  CSV file consists of columns containing the authority code and years.
  Each row contains an authority code and values for each year (or no value
  if the data doesn't exist).

  Note that these files do not include the names for areas, instead you 
  have to rely on the names already populated through 
  Areas::populateFromAuthorityCodeCSV();

  The datasets that will be parsed by this function are
   - complete-popu1009-area.csv
   - complete-popu1009-pop.csv
   - complete-popu1009-opden.csv

  @param is
    The input stream from InputSource

  @param cols
    A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
    that give the column header in the CSV file

  @param areasFilter
    An umodifiable pointer to set of umodifiable strings for areas to import,
    or an empty set if all areas should be imported

  @param measuresFilter
    An umodifiable pointer to set of strings for measures to import, or an empty 
    set if all measures should be imported

  @param yearsFilter
    An umodifiable pointer to an umodifiable tuple of two unsigned integers,
    where if both values are 0, then all years should be imported, otherwise
    they should be treated as a the range of years to be imported

  @return
    void

  @see
    See datasets.h for details of how the variable cols is organised

  @see
    See bethyw.cpp for details of how the variable areasFilter is created

  @example
    InputFile input("data/complete-popu1009-pop.csv");
    auto is = input.open();

    auto cols = InputFiles::DATASETS["complete-pop"].COLS;

    auto areasFilter = BethYw::parseAreasArg();
    auto yearsFilter = BethYw::parseYearsArg();

    Areas data = Areas();
    areas.populateFromAuthorityCodeCSV(is, cols, &areasFilter, &yearsFilter);

  @throws 
    std::runtime_error if a parsing error occurs (e.g. due to a malformed file)
    std::out_of_range if there are not enough columns in cols
*/
void Areas::populateFromAuthorityByYearCSV(
	std::istream &is,
	const BethYw::SourceColumnMapping &cols,
	const StringFilterSet *const areasFilter,
	const StringFilterSet *const measuresFilter,
	const YearFilterTuple *const yearsFilter)
noexcept(false) {

	//check to see if this file contains data in the measures filter.
	bool should_load = false;
	std::string tmp;
	std::string tmp2;

	//if the filter is null or empty then we can load this file regardless.
	if (measuresFilter == nullptr || measuresFilter->empty()) {
		should_load = true;
	} else {
		try {
			for (const auto &it : *measuresFilter) {

				tmp = cols.at(BethYw::SourceColumn::SINGLE_MEASURE_CODE);
				tmp2 = it;

				//convert values to lowercase before testing for equality.
				std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
				std::transform(tmp2.begin(), tmp2.end(), tmp2.begin(), ::tolower);

				if (tmp == tmp2) {
					should_load = true;
				}
			}
		} catch (std::out_of_range &e) {
			throw std::out_of_range("Not enough columns in cols!");
		}
	}

	//load data from file into areas.
	if (should_load) {

		char delimiter = ',';
		std::stringstream line_stream;
		std::map<int, unsigned int> allowed_years;
		double current_measure_value;
		std::vector<double> values;

		std::string line_buffer,
			current_value,
			current_area_code,
			measure_code,
			measure_label;

		unsigned int current_year,
			year_range_start,
			year_range_end;

		bool load_all_years = false;
		//if the filter is null or empty then we load everything.
		bool load_all_areas = (areasFilter == nullptr || areasFilter->empty());

		measure_code = cols.at(BethYw::SourceColumn::SINGLE_MEASURE_CODE);
		measure_label = cols.at(BethYw::SourceColumn::SINGLE_MEASURE_NAME);

		//check to see if all years need to be loaded.
		if (yearsFilter != nullptr) {
			year_range_start = std::get<0>(*yearsFilter);
			year_range_end = std::get<1>(*yearsFilter);
			if (year_range_end == 0) {
				load_all_years = true;
			}
		} else {
			load_all_years = true;
		}

		//place column headers into the line stream for validation.
		std::getline(is, line_buffer);
		line_stream << line_buffer;

		line_buffer = "";
		//check to see if the file header matches the column header.
		std::getline(line_stream, line_buffer, delimiter);
		if (line_buffer != cols.at(BethYw::SourceColumn::AUTH_CODE)) {
			throw std::runtime_error("Malformed file!");
		}

		//read all year headers and add allowed years to the map.
		int i = 0;
		while (std::getline(line_stream, current_value, delimiter)) {
			current_year = std::stoi(current_value);

			//if the current year is in range then we store it and its index into the map for later use.
			if (load_all_years || ((current_year <= year_range_end) && (current_year >= year_range_start))) {
				allowed_years.insert(std::pair<int, unsigned int>(i, current_year));
			}
			i++;
		}

		while (std::getline(is, line_buffer)) {
			//clear values vector before re-use.
			values.clear();

			//clear and set the stream for the current line
			line_stream.str("");
			line_stream.clear();
			line_stream << line_buffer;

			std::getline(line_stream, current_area_code, delimiter);

			//load all yearly readings into the values array.
			while (std::getline(line_stream, current_value, delimiter)) {
				current_measure_value = std::stod(current_value);
				values.push_back(current_measure_value);
			}

			//the map of allowed years gives us the indices of the values,
			//so we loop through every year allowed to us and add the values to the measures.
			for (const auto &it : allowed_years) {
				try {
					Area &a = this->getArea(current_area_code);

					//check if the current area should be loaded
					if (load_all_areas || checkIfAreaMatchesFilter(a, areasFilter)) {
						try {
							Measure &m = a.getMeasure(measure_code);
							m.setValue(it.second, values[it.first]);
						} catch (std::out_of_range &e) {
							auto tmp_measure = Measure(measure_code, measure_label);
							tmp_measure.setValue(it.second, values[it.first]);
							a.setMeasure(tmp_measure.getCodename(), tmp_measure);
						}
					}
					//if there is no area then we must just skip the entry
				} catch (std::out_of_range &e) {}
			}
		}

		//clean up variables
		line_buffer = "";
		current_value = "";
		current_area_code = "";
		measure_code = "";
		measure_label = "";

		current_year = 0;
		year_range_start = 0;
		year_range_end = 0;
		current_measure_value = 0;

		line_stream.str("");
		line_stream.clear();

		allowed_years.clear();
		values.clear();
	}
}

/**
  Parse data from an standard input stream `is`, that has data of a particular
  `type`, and with a given column mapping in `cols`.

  This function should look at the `type` and hand off to one of the three 
  functions populate………() functions.

  The function must check if the stream is in working order and has content.

  @param is
    The input stream from InputSource

  @param type
    A value from the BethYw::SourceDataType enum which states the underlying
    data file structure

  @param cols
    A map of the enum BethYw::SourceColumnMapping (see datasets.h) to strings
    that give the column header in the CSV file

  @return
    void

  @throws 
    std::runtime_error if a parsing error occurs (e.g. due to a malformed file),
    the stream is not open/valid/has any contents, or an unexpected type
    is passed in.
    std::out_of_range if there are not enough columns in cols

  @see
    See datasets.h for details of the values variable type can have

  @see
    See datasets.h for details of how the variable cols is organised

  @see
    See bethyw.cpp for details of how the variable areasFilter is created

  @example
    InputFile input("data/popu1009.json");
    auto is = input.open();

    auto cols = InputFiles::DATASETS["popden"].COLS;

    Areas data = Areas();
    areas.populate(
      is,
      DataType::WelshStatsJSON,
      cols);
*/
void Areas::populate(std::istream &is,
					 const BethYw::SourceDataType &type,
					 const BethYw::SourceColumnMapping &cols) {

	try {
		switch (type) {
			case BethYw::AuthorityCodeCSV: populateFromAuthorityCodeCSV(is, cols);
				break;
			case BethYw::AuthorityByYearCSV: populateFromAuthorityByYearCSV(is, cols);
				break;
			case BethYw::WelshStatsJSON: populateFromWelshStatsJSON(is, cols);
				break;
			default: throw std::runtime_error("Areas::populate: Unexpected data type");
		}
	} catch (std::runtime_error &e1) {
		throw e1;
	} catch (std::out_of_range &e2) {
		throw e2;
	}
}

/**
  Parse data from an standard input stream, that is of a particular type,
  and with a given column mapping, filtering for specific areas, measures,
  and years, and fill the container.

  This function should look at the `type` and hand off to one of the three 
  functions you've implemented above.

  The function must check if the stream is in working order and has content.

  This overloaded function includes pointers to the three filters for areas,
  measures, and years.

  @param is
    The input stream from InputSource

  @param type
    A value from the BethYw::SourceDataType enum which states the underlying
    data file structure

  @param cols
    A map of the enum BethyYw::SourceColumnMapping (see datasets.h) to strings
    that give the column header in the CSV file

  @param areasFilter
    An umodifiable pointer to set of umodifiable strings for areas to import,
    or an empty set if all areas should be imported

  @param measuresFilter
    An umodifiable pointer to set of umodifiable strings for measures to import,
    or an empty set if all measures should be imported

  @param yearsFilter
    An umodifiable pointer to an umodifiable tuple of two unsigned integers,
    where if both values are 0, then all years should be imported, otherwise
    they should be treated as a the range of years to be imported

  @return
    void

  @throws 
    std::runtime_error if a parsing error occurs (e.g. due to a malformed file),
    the stream is not open/valid/has any contents, or an unexpected type
    is passed in.
    std::out_of_range if there are not enough columns in cols

  @see
    See datasets.h for details of the values variable type can have

  @see
    See datasets.h for details of how the variable cols is organised

  @see
    See bethyw.cpp for details of how the variables areasFilter, measuresFilter,
    and yearsFilter are created

  @example
    InputFile input("data/popu1009.json");
    auto is = input.open();

    auto cols = InputFiles::DATASETS["popden"].COLS;

    auto areasFilter = BethYw::parseAreasArg();
    auto measuresFilter = BethYw::parseMeasuresArg();
    auto yearsFilter = BethYw::parseMeasuresArg();

    Areas data = Areas();
    areas.populate(
      is,
      DataType::WelshStatsJSON,
      cols,
      &areasFilter,
      &measuresFilter,
      &yearsFilter);
*/
void Areas::populate(
	std::istream &is,
	const BethYw::SourceDataType &type,
	const BethYw::SourceColumnMapping &cols,
	const StringFilterSet *const areasFilter,
	const StringFilterSet *const measuresFilter,
	const YearFilterTuple *const yearsFilter) {

	try {
		switch (type) {
			case BethYw::AuthorityCodeCSV: populateFromAuthorityCodeCSV(is, cols, areasFilter);
				break;
			case BethYw::AuthorityByYearCSV:
				populateFromAuthorityByYearCSV(is, cols, areasFilter, measuresFilter, yearsFilter);
				break;
			case BethYw::WelshStatsJSON: populateFromWelshStatsJSON(is, cols, areasFilter, measuresFilter, yearsFilter);
				break;
			default: throw std::runtime_error("Areas::populate: Unexpected data type");
		}
	} catch (std::runtime_error &e1) {
		throw e1;
	} catch (std::out_of_range &e2) {
		throw e2;
	}
}

/**
  Convert this Areas object, and all its containing Area instances, and
  the Measure instances within those, to values.

  Use the sample JSON library as above to create this. Construct a blank
  JSON object:
    json j;

  Convert this json object to a string:
    j.dump();

  You then need to loop through your areas, measures, and years/values
  adding this data to the JSON object.

  Read the documentation for how to convert your outcome code to JSON:
    https://github.com/nlohmann/json#arbitrary-types-conversions
  
  The JSON should be formatted as such:
    {
    "<localAuthorityCode1>" : {
                              "names": { "<languageCode1>": "<languageName1>",
                                         "<languageCode2>": "<languageName2>" 
                                         …
                                         "<languageCodeN>": "<languageNameN>" }, 
                              "measures" : { "<year1>": <value1>,
                                             "<year2>": <value2>,
                                             …
                                            "<yearN>": <valueN> }
                               },
    "<localAuthorityCode2>" : {
                              "names": { "<languageCode1>": "<languageName1>",
                                         "<languageCode2>": "<languageName2>" 
                                         …
                                         "<languageCodeN>": "<languageNameN>" }, 
                              "measures" : { "<year1>": <value1>,
                                             "<year2>": <value2>,
                                             …
                                            "<yearN>": <valueN> }
                               }
    ...
    "<localAuthorityCodeN>" : {
                              "names": { "<languageCode1>": "<languageName1>",
                                         "<languageCode2>": "<languageName2>" 
                                         …
                                         "<languageCodeN>": "<languageNameN>" }, 
                              "measures" : { "<year1>": <value1>,
                                             "<year2>": <value2>,
                                             …
                                            "<yearN>": <valueN> }
                               }
    }

  An empty JSON is "{}" (without the quotes), which you must return if your
  Areas object is empty.
  
  @return
    std::string of JSON

  @example
    InputFile input("data/popu1009.json");
    auto is = input.open();

    auto cols = InputFiles::DATASETS["popden"].COLS;

    auto areasFilter = BethYw::parseAreasArg();
    auto measuresFilter = BethYw::parseMeasuresArg();
    auto yearsFilter = BethYw::parseMeasuresArg();

    Areas data = Areas();
    std::cout << data.toJSON();
*/
std::string Areas::toJSON() const {
	json j = {};

	//convert each area into a json object and add to the parent json object.
	for (const auto &it : areas_container) {
		to_json(j, it.second);
	}

	//check if the json is empty.
	if (j.dump() == "null") {
		return "{}";
	}

	return j.dump();
}

/**
  Overload the << operator to print all of the imported data.

  Output should be formatted like the following to pass the tests. Areas should
  be printed, ordered alphabetically by their local authority code. Measures 
  within each Area should be ordered alphabetically by their codename.

  See the coursework specification for more information, although for reference
  here is a quick example of how output should be formatted:

    <English name of area 1> / <Welsh name of area 1> (<authority code 1>)
    <Measure 1 name> (<Measure 1 code>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean 1> <diff 1> <diffp 1>

    <Measure 2 name> (<Measure 2 code>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean 2> <diff 2> <diffp 2>

    <Measure 3 name> (<Measure 3 code>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean 3> <diff 3> <diffp 3>

    ...

    <Measure x name> (<Measure x code>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean x> <diff x> <diffp x>


    <English name of area 2> / <Welsh name of area 2> (<authority code 2>)
    <Measure 1 name> (<Measure 1 code>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean 1> <diff 1> <diffp 1>

    <Measure 2 name> (<Measure 2 code>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean 2> <diff 2> <diffp 2>

    <Measure 3 name> (<Measure 3 code>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean 3> <diff 3> <diffp 3>

    ...

    <Measure x name> (<Measure x code>)
     <year 1>  <year 2> <year 3> ...  <year n>
    <value 1>  <year 2> <year 3> ... <value n> <mean x> <diff x> <diffp x>

    ...

    <English name of area y> / <Welsh name of area y> (<authority code y>)
    <Measure 1 name> (<Measure 1 codename>)
     <year 1>  <year 2> <year 3> ...  <year n>
    <value 1>  <year 2> <year 3> ... <value n> <mean 1> <diff 1> <diffp 1>

    <Measure 2 name> (<Measure 2 codename>)
     <year 1>  <year 2> <year 3> ...  <year n>
    <value 1>  <year 2> <year 3> ... <value n> <mean 2> <diff 2> <diffp 2>

    <Measure 3 name> (<Measure 3 codename>)
     <year 1>  <year 2> <year 3> ...  <year n>
    <value 1>  <year 2> <year 3> ... <value n> <mean 3> <diff 3> <diffp 3>

    ...

    <Measure x name> (<Measure x codename>)
     <year 1>  <year 2> <year 3> ...  <year n>  Average    Diff.   % Diff.
    <value 1>  <year 2> <year 3> ... <value n> <mean x> <diff x> <diffp x>

  With real data, your output should start like this for the command
  bethyw --dir <dir> -p popden -y 1991-1993 (truncated for readability):

    Isle of Anglesey / Ynys Môn (W06000001)
    Land area (area) 
          1991       1992       1993    Average    Diff.  % Diff. 
    711.680100 711.680100 711.680100 711.680100 0.000000 0.000000 

    Population density (dens) 
         1991      1992      1993   Average    Diff.  % Diff. 
    97.126504 97.486216 98.038430 97.550383 0.911926 0.938905 

    Population (pop) 
            1991         1992         1993      Average      Diff.  % Diff. 
    69123.000000 69379.000000 69772.000000 69424.666667 649.000000 0.938906 


    Gwynedd / Gwynedd (W06000002)
    Land area (Area)
    ...

  @param os
    The output stream to write to

  @param areas
    The Areas object to write to the output stream

  @return
    Reference to the output stream

  @example
    Areas areas();
    std::cout << areas << std::end;
*/
std::ostream &operator<<(std::ostream &os, const Areas &obj) {
	for (const auto &it : obj.areas_container) {
		os << it.second << std::endl;
	}

	return os;
}
