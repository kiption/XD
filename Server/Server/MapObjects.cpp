#include "MapObjects.h"

float string2data(std::string str)
{
	{

		if (str.find('(') != std::string::npos) {
			str.erase(remove(str.begin(), str.end(), '('));
		}
		if (str.find(',') != std::string::npos) {
			str.erase(remove(str.begin(), str.end(), ','));
		}
		if (str.find(')') != std::string::npos) {
			str.erase(remove(str.begin(), str.end(), ')'));
		}
		return stof(str);
	}
}
