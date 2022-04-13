# BethYw? StatsWales Data Analysis Tool
___

## Information
Compiled using gcc C++ compiler version 8.3.0 and C++ standard 14

External Libraries used : [CXXOpts](https://github.com/jarro2783/cxxopts), 
[JSON for Modern C++](https://github.com/nlohmann/json), 
[Catch2 Unit Testing Framework](https://github.com/catchorg/Catch2).

This program was built to read and query a number of datasets from the StatsWales website in both csv and JSON formats.
All of the datasets group a series of measures over a number of years. Each of these measures is organised by the 
local authority where the readings were taken.

___
## Arguments

* ### _--directory_
  This argument allows the user to specify a path to a directory containing the StatsWales datasets.
  By default this points to /datasets.

  #### Usage:
  ```bethyw --directory /files/directory```

* ### _--dataset / -d_
  This argument allows the user to specify the dataset(s) they wish to load into the program.
  By default the program loads all datasets in the given directory.

  #### Usage:
  Single dataset: 
  `bethyw -d biz`
  
  Multiple datasets:
  `bethyw -d popden,aqi,complete-area`

* ### _--area / -a_

  This argument allows the user to specify what local authorities should be included in the output.
  By default all local authorities will be included. This argument is case insensitive and will 
  include areas with a full or partial match with the argument.

  #### Usage:
  Single area:
  `bethyw -a swansea`
  
  Multiple areas:
  `bethyw -a swansea,cardiff`

* ### _--measure / -m_

  This argument allows the user to specify what measures should be included in the output.

  #### Usage:
  Single measure:
  `bethyw -m pop`
  
  Multiple measures:
  `bethyw -m pop,dens`

* ### _--year / -y_

  This argument allows the user to specify a year or range of years to be imported from the datasets.
  #### Usage:
  Single year :
  `bethyw -y 2000`

  Year range:
  `bethyw -y 2000-2020`

* ### _--json / -j_

  This argument specifies whether the output will be formatted as a JSON string rather than tables.

  #### Usage:
  `bethyw -j`

___
## Datasets
* **popu1009.json**

  Contains population density information by area.

  Load with `popden` flag.

  Contains the measures : `Population (pop), Population Density (dens), Land Area (area)`.
  
* **econ0080.json**

  Contains data about active businesses by area and year.

  Load with `biz` flag.

  Contains the measures : `Active Enterprises (a), Newly Opened Enterprises (b), Closing Enterprises (d),`

  `Active Enterprises per 10,000 People aged 16-64 (pa), Births per 10,000 People aged 16-64 (pb), `

  `Deaths per 10,000 People ages 16-64 (pd), Birth Rate as a Percentage of Active Enterprises (rb),`

  `Death rate as a Percentage of Active Enterprises (rd)`.

* **envi0201.json**

  Contains data about air quality indicators by local authority.

  Load with `aqi` flag.

  Contains the measures : `PM10 Readings (pm10), PM2.5 Readings (pm2-5), NO2 Readings (no2)`.

* **tran0152.json**

  Contains population density by area.

  Load with `trains` flag.

  Contains the measures : `Number of Railway Passengers (rail)`.

* **complete-popu1009-[area|pop|popden].csv**

  Contains data about the area, population and population density of every local authority.

  Load with `complete-[area|pop|popden]` flag.

  Contains the measures : `area|pop|dens`.
___
## Examples

Retrieve air quality information for Swansea between the years of 2007 and 2014:

`bethyw -d aqi -a abertawe -y 1990-2000`

Output:

```
Swansea / Abertawe (W06000011)
NO2 (no2)
     2007      2008      2009      2010      2011      2012      2013      2014   Average     Diff.    % Diff.
14.312794 13.930919 13.079848 13.920170 12.501802 12.938052 12.343438 10.840821 12.983480 -3.471973 -24.257827

PM10 (pm10)
     2007      2008      2009      2010      2011      2012      2013      2014   Average     Diff.    % Diff.
16.337369 14.000593 12.957386 13.114746 13.697394 12.149206 13.415444 13.486073 13.644776 -2.851296 -17.452601

PM2-5 (pm2-5)
    2007     2008     2009     2010     2011     2012     2013     2014  Average    Diff.  % Diff.
8.758945 8.941869 8.553838 9.054882 9.208958 8.536595 9.405871 9.448386 8.988668 0.689441 7.871283
```

Retrieve population and railway information for Neath Port Talbot between 200 and 2005 as a JSON string:

`bethyw -d popden,trains -a neath -m rail,pop -y 2000-2005 -j`

Output:

```
{
  "W06000012": {
    "measures": {
      "pop": {
        "2001": 134380.0,
        "2002": 134931.0,
        "2004": 137144.0,
        "2005": 137670.0
      },
      "rail": {
        "2002": 331150.5,
        "2003": 334916.5,
        "2004": 332312.0,
        "2005": 332574.5
      }
    },
    "names": {
      "cym": "Castell-nedd Port Talbot",
      "eng": "Neath Port Talbot"
    }
  }
}
```
