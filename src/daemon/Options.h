/*
 * Options.h
 *
 *  Created on: 2016. 9. 30.
 *      Author: root
 */

#ifndef DAEMON_OPTIONS_H_
#define DAEMON_OPTIONS_H_

#include <map>
#include <string>
#include <stdio.h>

namespace kinow {

using namespace std;

class Options {
public:
	static const int REQUIRE_DEFAULT      = 0x01;
	static const int REQUIRE_OPT          = 0x02;
	static const int REQUIRE_OPT_VALUE    = 0x04;


public:
	Options(int argc, char** argv) : m_argc(argc), m_argv(argv) {}
	virtual ~Options();
	void addOptionHelp();
	bool addOption(char code, const char* desc, int require = REQUIRE_DEFAULT, const char* defValue = NULL);
	bool validOptions(char *errbuf = NULL, size_t errbuflen = 0);
	bool isRequire(char code);
	bool isRequireValue(char code);
	const char* optionValue(char code);
	void showOptions(const char* title = NULL);
	void usage();
	void help();
	bool isOption(char code);

private:
	class Option {
	public:
		Option(char code, int require = REQUIRE_DEFAULT, const char *desc = NULL);
		~Option();
		int require();
		char code();
		const char* desc();
		const char* value();
		void value(const char* value);
		bool checked();
		void check(bool checked);
		string toString();

	private:
		char m_code;
		int m_require;
		char *m_desc;
		char *m_value;
		bool m_checked;
	};

private:
	int m_argc;
	char **m_argv;
	map<char, Option*> m_optionMap;
	string getOptionsString();
};

} /* namespace kinow */

#endif /* DAEMON_OPTIONS_H_ */
