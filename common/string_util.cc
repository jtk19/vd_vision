#include <ctype.h>
#include <sstream>
#include "string_util.h"


string common::trim(std::string str)
{
	// remove trailing white space
	while (!str.empty() && isspace(str.back())) str.pop_back();

	// return residue after leading white space
	std::size_t pos = 0;
	while (pos < str.size() && isspace(str[pos])) ++pos;
	return str.substr(pos);
}

bool common::isspace(char c)
{
	return ((c == ' ') || (c == '\t'));
}

string common::strtolower( string str )
{
	for ( size_t i = 0; i < str.length(); ++i )
	{
		str[i] = tolower(str[i]);
	}
	return str;
}

string common::strrep1( string str, string tgt, string rep )
{
	if ( str.find(tgt) != string::npos )
	{
		str.replace( str.find(tgt), tgt.length(), rep );
	}
	return str;
}

string common::strrep( string str, string tgt, string rep )
{
	while ( str.find(tgt) != string::npos )
	{
		str.replace( str.find(tgt), tgt.length(), rep );
	}
	return str;
}

void common::split( vector<string> &tokens, string line, char sep )
{
	string tok;
	stringbuf sbuf( line );
	istream is(&sbuf);

	while ( is.good() )
	{
		getline( is, tok, sep);
		tokens.push_back( trim(tok) );
	}
}

