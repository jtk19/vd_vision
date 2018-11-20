/*-------------------------------------------------------------------------------
 * string_util.h: String processing utility functions
 *
 */
#ifndef __CRYPT_STRING_UTIL_H__
#define __CRYPT_STRING_UTIL_H__

#include <iostream>
#include <vector>

using namespace std;


namespace common
{

/** @brief Removes leading and trailing white spaces in str */
extern string trim( string str);

extern string strtolower( string str );

extern bool isspace( char c);

/** @brief Returns whether str contains s1 independent of upper or lower case */
inline bool contains( string str, string s1 )
{
	str = common::strtolower(str);
	s1 = common::strtolower(s1);
	return ( str.find(s1) != string::npos );
}

/** @brief Replaces multiple instances of tgt with rep in str. */
extern string strrep( string str, string tgt, string rep );

/** @brief Replaces the first instances of tgt with rep in str. */
extern string strrep1( string str, string tgt, string rep );

/** @brief Splits the line into tokens along the separator sep. */
extern void split( vector<string> &tokens, string line, char sep );

}


#endif
