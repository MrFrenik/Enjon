#ifndef ENJON_FUNCTIONS_H
#define ENJON_FUNCTIONS_H

#include <string>
#include <sstream>
#include <vector>

namespace Enjon { namespace Utils { 

	inline void split(const std::string &s, char delim, std::vector<std::string> &elems) 
	{
	    std::stringstream ss;
	    ss.str(s);
	    std::string item;
	    while (getline(ss, item, delim)) 
	    {
	        elems.push_back(item);
	    }
	}

	inline std::vector<std::string> split(const std::string &s, char delim) 
	{
	    std::vector<std::string> elems;
	    split(s, delim, elems);
	    return elems;
	}

	inline bool is_numeric(const std::string& s)
	{
		// Integers for now
		auto dec_count = 0;

		for (auto& c : s)
		{
			if (c > '9' || c < '0') 
			{
				if (c == '.')
				{
					if (!dec_count) dec_count++;
					else return false;
				}

				else return false;
			}
		}

		return true;
	}
}}



#endif