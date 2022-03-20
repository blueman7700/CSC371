/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: Oliver Morris - xxxxxx

  This file contains all the helper functions for initialising and running
  Beth Yw? In languages such as Java, this would be a class, but we really
  don't need a class here. Classes are for modelling data, and so forth, but
  here the code is pretty much a sequential block of code (BethYw::run())
  calling a series of helper functions.
*/

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>
#include <regex>

#include "lib_cxxopts.hpp"
#include "areas.h"
#include "bethyw.h"

#define REGEX_SINGLE_YEAR "^([0-9]{4})$"
#define REGEX_YEAR_RANGE "^([0-9]{4})-([0-9]{4})$"

/**
  Run Beth Yw?, parsing the command line arguments, importing the data,
  and outputting the requested data to the standard output/error.

  Hint: cxxopts.parse() throws exceptions you'll need to catch. Read the cxxopts
  documentation for more information.

  @param argc
    Number of program arguments

  @param argv
    Program arguments

  @return
    Exit code
*/
int BethYw::run(int argc, char *argv[]) {

	//catch any parsing errors thrown by cxxopts.parse()
	try {
		auto cxxopts = BethYw::cxxoptsSetup();
		auto args = cxxopts.parse(argc, argv);

		// Print the help usage if requested
		if (args.count("help")) {
			std::cerr << cxxopts.help() << std::endl;
			return 0;
		}

		// Parse data directory argument
		std::string dir = args["dir"].as<std::string>() + DIR_SEP;

		// Parse other arguments and import data
		try {
			auto datasetsToImport = BethYw::parseDatasetsArg(args);
			auto areasFilter      = BethYw::parseAreasArg(args);
			auto measuresFilter   = BethYw::parseMeasuresArg(args);
			auto yearsFilter      = BethYw::parseYearsArg(args);
			datasetsToImport.size();
			Areas data = Areas();

			//attempt to load area.csv and datasets
			try {
				BethYw::loadAreas(data, dir, areasFilter);

				BethYw::loadDatasets(data,
							dir,
							datasetsToImport,
							areasFilter,
							measuresFilter,
							yearsFilter);
			} catch (std::out_of_range &e1) {
				std::cerr << "Error importing dataset:" << std::endl << e1.what() << std::endl;
			} catch (std::runtime_error &e2) {
				std::cerr << "Error importing dataset:" << std::endl << e2.what() << std::endl;
			}

			if (args.count("json")) {
				// The output as JSON
				std::cout << data.toJSON();
			} else {
				// The output as tables
				std::cout << data;
			}

		} catch (std::invalid_argument &e1) {
			std::cerr << e1.what() << std::endl;
		} catch (std::runtime_error &e2) {
			std::cerr << e2.what() << std::endl;
		}
	} catch (cxxopts::OptionParseException &e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

/**
  This function sets up and returns a valid cxxopts object. You do not need to
  modify this function.

  @return
     A constructed cxxopts object

  @example
    auto cxxopts = BethYw::cxxoptsSetup();
    auto args = cxxopts.parse(argc, argv);
*/
cxxopts::Options BethYw::cxxoptsSetup() {
	cxxopts::Options cxxopts(
		"bethyw",
		"Student ID: " + STUDENT_NUMBER + "\n\n"
										  "This program is designed to parse official Welsh Government"
										  " statistics data files.\n");

	cxxopts.add_options()
		("dir",
			"Directory for input data passed in as files",
			cxxopts::value<std::string>()->default_value("datasets"))

		("d,datasets",
			"The dataset(s) to import and analyse as a comma-separated list of codes "
			"(omit or set to 'all' to import and analyse all datasets)",
			cxxopts::value<std::vector<std::string>>())

		("a,areas",
			"The areas(s) to import and analyse as a comma-separated list of "
			"authority codes (omit or set to 'all' to import and analyse all areas)",
			cxxopts::value<std::vector<std::string>>())

		("m,measures",
			"Select a subset of measures from the dataset(s) "
			"(omit or set to 'all' to import and analyse all measures)",
			cxxopts::value<std::vector<std::string>>())

		("y,years",
			"Focus on a particular year (YYYY) or "
			"inclusive range of years (YYYY-ZZZZ)",
			cxxopts::value<std::string>()->default_value("0"))

		("j,json",
			"Print the output as JSON instead of tables.")

		("h,help",
		"Print usage.");

	return cxxopts;
}

/**
  Parse the datasets argument passed into the command line. 

  The datasets argument is optional, and if it is not included, all datasets 
  should be imported. If it is included, it should be a comma-separated list of 
  datasets to import. If the argument contains the value "all"
  (case-insensitive), all datasets should be imported.

  This function validates the passed in dataset names against the codes in
  DATASETS array in the InputFiles namespace in datasets.h. If an invalid code
  is entered, throw a std::invalid_argument with the message:
  No dataset matches key: <input code>
  where <input name> is the name supplied by the user through the argument.

  @param args
    Parsed program arguments

  @return
    A std::vector of BethYw::InputFileSource instances to import

  @throws
    std::invalid_argument if the argument contains an invalid dataset with
    message: No dataset matches key <input code>

  @example
    auto cxxopts = BethYw::cxxoptsSetup();
    auto args = cxxopts.parse(argc, argv);

    auto datasetsToImport = BethYw::parseDatasetsArg(args);
 */
std::vector<BethYw::InputFileSource> BethYw::parseDatasetsArg(cxxopts::ParseResult &args) {

	// Retrieve all valid datasets, see datasets.h
	size_t numDatasets = InputFiles::NUM_DATASETS;
	auto &allDatasets = InputFiles::DATASETS;

	// Create the container for the return type
	std::vector<InputFileSource> datasetsToImport;

	//if an OptionParseException is thrown then no dataset has been specified,
	//therefore we just load every dataset
	try {
		if(args.count("datasets")) {
			// You can get the std::vector of arguments from cxxopts like this.
			// Note that this function will throw an exception if datasets is not set as
			// an argument. Check the documentation! Read it and understand it.
			auto inputDatasets = args["datasets"].as<std::vector<std::string>>();

			//first check if the all flag is present in the list of arguments.
			bool allFlag = false;
			for(auto &it : inputDatasets) {
				if(it == "all") {
					allFlag = true;
				}
			}

			//iterate through every dataset passed in as an argument and compares them to the
			//list of all datasets in InputFiles::DATASETS. If the allFlag is set to true, we
			//can skip this and load all datasets.
			if(!allFlag) {
				unsigned int i;
				bool isValid;
				for(auto &it : inputDatasets) {
					isValid = false;
					//check if the current param matches any known dataset.
					for(i = 0; i < numDatasets; i++) {
						if(it == (allDatasets[i].CODE)) {
							isValid = true;
							datasetsToImport.push_back(allDatasets[i]);
						}
					}

					//if no match was made then the argument is invalid.
					if(!isValid) {
						throw std::invalid_argument("No dataset matches key: " + it + "");
					}
				}
			} else {
				for (unsigned int i = 0; i < numDatasets; i++)
					datasetsToImport.push_back(allDatasets[i]);
			}
		} else {
			throw cxxopts::OptionParseException("No Argument(s) for 'datasets'");
		}

	}catch (const cxxopts::OptionException& e) {
		//if there are no arguments then this exception is caught and all datasets are loaded.
		for (unsigned int i = 0; i < numDatasets; i++)
			datasetsToImport.push_back(allDatasets[i]);
	}

	return datasetsToImport;
}

/**
  Parses the areas command line argument, which is optional. If it doesn't 
  exist or exists and contains "all" as value (any case), all areas should be
  imported, i.e., the filter should be an empty set.

  Unlike datasets we can't check the validity of the values as it depends
  on each individual file imported (which hasn't happened until runtime).
  Therefore, we simply fetch the list of areas and later pass it to the
  Areas::populate() function.

  The filtering of inputs should be case insensitive.

  @param args
    Parsed program arguments

  @return 
    An std::unordered_set of std::strings corresponding to specific areas
    to import, or an empty set if all areas should be imported.

  @throws
    std::invalid_argument if the argument contains an invalid areas value with
    message: Invalid input for area argument
*/
std::unordered_set<std::string> BethYw::parseAreasArg(cxxopts::ParseResult &args) {

	std::unordered_set<std::string> areas;
	try {
		if(args.count("areas")) {
			// Retrieve the areas argument like so:
			auto temp = args["areas"].as<std::vector<std::string>>();

			//check to see if "all" is a parameter.
			bool allFlag = false;
			for(auto &it : temp) {
				if(it == "all") {
					allFlag = true;
				}
			}

			//if "all" is not present, then add all arguments to the set.
			if(!allFlag) {
				for(auto &it : temp) {
					areas.insert(it);
				}
			}
		} else {
			throw cxxopts::OptionParseException("No Argument(s) for 'areas'");
		}
	} catch (cxxopts::OptionParseException &e) {}

	return areas;
}

/**
  Parse the measures command line argument, which is optional. If it doesn't
  exist or exists and contains "all" as value (any case), all measures should
  be imported.

  Unlike datasets we can't check the validity of the values as it depends
  on each individual file imported (which hasn't happened until runtime).
  Therefore, we simply fetch the list of areas and later pass it to the
  Areas::populate() function.

  The filtering of inputs should be case insensitive.

  @param args
    Parsed program arguments

  @return 
    An std::unordered_set of std::strings corresponding to specific measures
    to import, or an empty set if all measures should be imported.

  @throws
    std::invalid_argument if the argument contains an invalid measures value
    with the message: Invalid input for measures argument
*/
std::unordered_set<std::string> BethYw::parseMeasuresArg(cxxopts::ParseResult &args) {
	// The unordered set you will return
	std::unordered_set<std::string> measures;

	try {
		if(args.count("measures")) {
			// Retrieve the measures argument like so:
			auto temp = args["measures"].as<std::vector<std::string>>();

			//check to see if "all" is a parameter.
			bool allFlag = false;
			for(auto &it : temp) {
				if(it == "all") {
					allFlag = true;
				}
			}

			//if "all" is not present, then add all arguments to the set.
			if(!allFlag) {
				for(auto &it : temp) {
					measures.insert(it);
				}
			}

		} else {
			throw cxxopts::OptionParseException("No Argument(s) for 'measures'");
		}
	} catch (cxxopts::OptionParseException &e) {}

	return measures;
}

/**
  Parse the years command line argument. Years is either a four digit year 
  value, or two four digit year values separated by a hyphen (i.e. either 
  YYYY or YYYY-ZZZZ).

  This should be parsed as two integers and inserted into a std::tuple,
  representing the start and end year (inclusive). If one or both values are 0,
  then there is no filter to be applied. If no year argument is given return
  <0,0> (i.e. to import all years). You will have to search
  the web for how to construct std::tuple objects! 

  @param args
    Parsed program arguments

  @return
    A std::tuple containing two unsigned ints

  @throws
    std::invalid_argument if the argument contains an invalid years value with
    the message: Invalid input for years argument
*/
std::tuple<unsigned int, unsigned int> BethYw::parseYearsArg(cxxopts::ParseResult &args) {

	std::tuple<unsigned int, unsigned int> filter;

	try {
		if(args.count("years")) {
			auto temp = args["years"].as<std::string>();

			std::regex single_year (REGEX_SINGLE_YEAR);
			std::regex year_range (REGEX_YEAR_RANGE);
			std::smatch s;

			unsigned int start, end;

			//check to see if the flag is of type "YYYY"
			if(std::regex_search(temp, s, single_year)) {
				start = std::stoi(s.str(1));
				end = std::stoi(s.str(1));

			//check if the flag is of type "YYYY-YYYY"
			} else if (std::regex_search(temp, s, year_range)) {
				start = std::stoi(s.str(1));
				end = std::stoi(s.str(2));

			//if the argument doesn't match the regex then check to see if the arg is 0 or 0-0
			} else if (temp == "0" || temp == "0-0") {
				start = 0;
				end = 0;
			}else {
				throw std::invalid_argument("Invalid input for years argument");
			}
			filter = std::make_tuple(start, end);

		} else {
			throw cxxopts::OptionParseException("No Argument(s) for 'years'");
		}
	} catch (cxxopts::OptionParseException &e) {
		filter = std::make_tuple(0, 0);
	}

	return filter;
}

/**
  Load the areas.csv file from the directory `dir`. Parse the file and
  create the appropriate Area objects inside the Areas object passed to
  the function in the `areas` argument.

  areas.csv is guaranteed to be formatted as:
    Local authority code,Name (eng),Name (cym)

  Hint: To implement this function. First you will need create an InputFile 
  object with the filename of the areas file, open it, and then pass reference 
  to the stream to the Areas::populate() function.

  Hint 2: you can retrieve the specific filename for a dataset, e.g. for the 
  areas.csv file, from the InputFileSource's FILE member variable

  @param areas
    An Areas instance that should be modified (i.e. the populate() function
    in the instance should be called)

  @param dir
    Directory where the areas.csv file is

  @param areasFilter
    An unordered set of areas to filter, or empty to import all areas

  @return
    void

  @example
    Areas areas();

    BethYw::loadAreas(areas, "data", BethYw::parseAreasArg(args));
*/
void BethYw::loadAreas(Areas &areas, std::string dir, const std::unordered_set<std::string> &areas_filter) {

	//add the correct filename to the directory path and populate the areas object.
	dir += InputFiles::AREAS.FILE;
	InputFile input_file = InputFile(dir);
	areas.populate(input_file.open(), BethYw::AuthorityCodeCSV, InputFiles::AREAS.COLS, &areas_filter);
}

/**
  Import datasets from `datasetsToImport` as files in `dir` into areas, and
  filtering them with the `areasFilter`, `measuresFilter`, and `yearsFilter`.

  The actual filtering will be done by the Areas::populate() function, thus 
  you need to merely pass pointers on to these filters.

  This function should promise not to throw an exception. If there is an
  error/exception thrown in any function called by thus function, catch it and
  output 'Error importing dataset:', followed by a new line and then the output
  of the what() function on the exception.

  @param areas
    An Areas instance that should be modified (i.e. datasets loaded into it)

  @param dir
    The directory where the datasets are

  @param datasetsToImport
    A vector of InputFileSource objects

  @param areasFilter
    An unordered set of areas (as authority codes encoded in std::strings)
    to filter, or empty to import all areas

  @param measuresFilter
    An unordered set of measures (as measure codes encoded in std::strings)
    to filter, or empty to import all measures

  @param yearsFilter
    An two-pair tuple of unsigned ints corresponding to the range of years 
    to import, which should both be 0 to import all years.

  @return
    void

  @example
    Areas areas();

    BethYw::loadDatasets(
      areas,
      "data",
      BethYw::parseDatasetsArgument(args),
      BethYw::parseAreasArg(args),
      BethYw::parseMeasuresArg(args),
      BethYw::parseYearsArg(args));
*/
void BethYw::loadDatasets(Areas &areas,
						  std::string &dir,
						  const std::vector<BethYw::InputFileSource> &datasetsToImport,
						  const std::unordered_set<std::string> &areasFilter,
						  const std::unordered_set<std::string> &measuresFilter,
						  const std::tuple<unsigned int, unsigned int> &yearsFilter) noexcept{

	//load each dataset listed in the filter and add the relevant content to all of the areas.
	for(const auto &it : datasetsToImport) {
		try {
			InputFile f = InputFile(dir + it.FILE);
			areas.populate(f.open(), it.PARSER, it.COLS, &areasFilter, &measuresFilter, &yearsFilter);
		} catch (std::out_of_range &e1) {
			std::cerr << "Error importing dataset:" << std::endl << e1.what() << std::endl;
		} catch (std::runtime_error &e2) {
			std::cerr << "Error importing dataset:" << std::endl << e2.what() << std::endl;
		}
	}
}
