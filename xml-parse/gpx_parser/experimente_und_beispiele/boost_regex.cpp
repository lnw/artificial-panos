#include <cstdlib>
#include <iostream>
//#include <string>
#include <boost/regex.hpp> // add boost regex library

using namespace std ;


std::string int_to_string(int i)
{
	std::stringstream s;
	std::string result;
	s << i;
	s >> result;
	return result;
}


int main()
{

for(int i=0; i<2; ++i)
{
  // Finally, a test showing capturing
  boost::regex EXPR( "(<food>)([A-Za-z]+)<(/food)>" ) ;
  string xmlData = "2012-02-03T12:23:32Z" ;

//std::string foo = "a" + int_2_string(i);

//std::cout << foo << i << std::endl;

  string replaced = boost::regex_replace( xmlData, EXPR, "\\4" , boost::match_default | boost::format_sed );
  cout << "EXPR: " << EXPR << endl;
  cout << "Data: " << xmlData << endl;
  cout << "The extracted data was: " << replaced << endl ;
}
}
