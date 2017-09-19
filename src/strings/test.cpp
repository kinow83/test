/*
 * test.cpp
 *
 *  Created on: 2016. 10. 24.
 *      Author: root
 */

#include <stdio.h>
#include <string>
#include<iostream>

using namespace std;



inline string& rtrim(string& str, const string& drop) {
	return str.erase( str.find_last_not_of(drop) + 1 );
}

inline string& ltrim(string& str, const string& drop) {
	return str.erase( 0, str.find_first_not_of(drop) );
}

inline string& trim(string& str, const string& drop) {
	return ltrim(rtrim(str, drop), drop);
}

int main(int argc, char** argv){

//	string s = "	kinow 			";
/*
	cout << "ltrim = " << "." << ltrim(s, " \t") << "." << endl;
	cout << "rtrim = " << "." << rtrim(s, " \t") << "." << endl;
	cout << " trim = " << "." <<  trim(s, " \t") << "." << endl;
	*/
}


