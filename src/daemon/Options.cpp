/*
 * Options.cpp
 *
 *  Created on: 2016. 9. 30.
 *      Author: root
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Options.h"

namespace kinow {

Options::Option::Option(char code, int require, const char *desc) {
	m_code = code;
	m_require = require;
	if (desc) m_desc = strdup(desc);
	else      m_desc = NULL;
	m_value = NULL;
	m_checked = false;
}

Options::Option::~Option() {
	if (m_desc)  free(m_desc);
	if (m_value) free(m_value);
}

int Options::Option::require() {
	return m_require;
}

char Options::Option::code() {
	return m_code;
}

const char* Options::Option::desc() {
	return m_desc;
}

const char* Options::Option::value() {
	return m_value;
}

void Options::Option::value(const char* value) {
	m_value = strdup(value);
}

bool Options::Option::checked() {
	return m_checked;
}

void Options::Option::check(bool checked) {
	m_checked = checked;
}

string Options::Option::toString() {
	string str;
	char buf[256] = { 0, };
	int len = 0;

	len += snprintf(buf, sizeof(buf), "-%c:\t", m_code);

	if (m_require & REQUIRE_OPT_VALUE) {
		len += snprintf(buf + len, sizeof(buf) - len, " value: '%s'", m_value);
	}
	if (m_desc) {
		len += snprintf(buf + len, sizeof(buf) - len, " description: '%s'", m_desc);
	}
	if (m_require & REQUIRE_OPT) {
		len += snprintf(buf + len, sizeof(buf) - len, " [require]");
	}
	if (m_require & REQUIRE_OPT_VALUE) {
		len += snprintf(buf + len, sizeof(buf) - len, " [require value]");
	}
	return buf;
}





Options::~Options() {
	map<char, Option*>::iterator it;
	for (it = m_optionMap.begin(); it != m_optionMap.end(); it++) {
		delete it->second;
	}
	m_optionMap.clear();
}

bool Options::addOption(char code, const char* desc, int require, const char* defValue) {
	if (m_optionMap[code] != NULL) {
		// duplicate code
		return false;
	}
	m_optionMap[code] = new Option(code, require, desc);
	if (defValue && defValue[0] != '\0') {
		m_optionMap[code]->value(defValue);
	}
	return true;
}

void Options::addOptionHelp() {
	static char code = '?';
	if (m_optionMap[code] != NULL) {
		free(m_optionMap[code]);
		m_optionMap[code] = NULL;
	}
	m_optionMap[code] = new Option(code, REQUIRE_DEFAULT, "help");
}

bool Options::validOptions(char *errbuf, size_t errbuflen) {
	int opt;
	Option *option = NULL;
	map<char, Option*>::iterator it;
	const char *optionString = getOptionsString().c_str();

	while ((opt = getopt(m_argc, m_argv, optionString)) != -1) {
		option = m_optionMap[opt];
		if (option == NULL) {
//			snprintf(errbuf, errbuflen, "'-%c' is invalid argument", opt);
			return false;
		}
		// case help
		else if (option->code() == '?') {
			help();
			exit(0);
		}
		else {
			if (option->require() & REQUIRE_OPT_VALUE) {
				option->value(optarg);
			}
			option->check(true);
		}
	}

	for (it = m_optionMap.begin(); it != m_optionMap.end(); it++) {
		option = it->second;
		// Argument is required, but not checked
		if (option->require() & REQUIRE_OPT) {
			if (option->checked() == false) {
				if (errbuf && errbuflen > 0) {
					snprintf(errbuf, errbuflen,
						"'-%c' is required argument", option->code());
				}
				return false;
			}
		}

		if (option->checked()) {
			if ((option->require() & REQUIRE_OPT_VALUE)
					&& ((option->value() == NULL) || (option->value()[0] == '\0')))
			{
				if (errbuf && errbuflen > 0) {
					snprintf(errbuf, errbuflen,
						"'-%c' is required argument value", option->code());
				}
				return false;
			}
		}
	}
	return true;
}

bool Options::isOption(char code) {
	Option *option = m_optionMap[code];
	if (option == NULL) {
		return false;
	}
	return option->checked();
}

bool Options::isRequire(char code) {
	Option *option = m_optionMap[code];
	if (option) {
		return (option->require() & REQUIRE_OPT);
	}
	return false;
}

bool Options::isRequireValue(char code) {
	Option *option = m_optionMap[code];
	if (option) {
		return (option->require() & REQUIRE_OPT_VALUE);
	}
	return false;
}

const char* Options::optionValue(char code) {
	Option *option = m_optionMap[code];
	if (option == NULL) {
		return NULL;
	}
	if (option->require() & REQUIRE_OPT_VALUE) {
		return option->value();
	}
	return NULL;
}

void Options::showOptions(const char* title) {
	Option *option = NULL;
	map<char, Option*>::iterator it;

	printf("%s\n", title ? title : "---------- [show all options] ----------");
	for (it = m_optionMap.begin(); it != m_optionMap.end(); it++) {
		option = it->second;
		printf("%s\n", option->toString().c_str());
	}
}

void Options::usage() {
	Option *option = NULL;
	string opt_str;
	map<char, Option*>::iterator it;

	printf("%s -[OPTIONS] [%s]\n", m_argv[0], getOptionsString().c_str());

	for (it = m_optionMap.begin(); it != m_optionMap.end(); it++) {
		option = it->second;
		if (option->require() & REQUIRE_OPT) {
			if (opt_str.c_str()[0]) opt_str.append(", ");
			opt_str.append("require option");
		}
		if (option->require() & REQUIRE_OPT_VALUE) {
			if (opt_str.c_str()[0]) opt_str.append(", ");
			opt_str.append("require option's value");
		}
		if (opt_str.c_str()[0]) {
			printf(" -%c          %s [%s]\n",
					it->first, option->desc() ? option->desc() : "", opt_str.c_str());
		} else {
			printf(" -%c          %s\n",
					it->first, option->desc() ? option->desc() : "");
		}
		opt_str.clear();
	}
}

void Options::help() {
	usage();
}

string Options::getOptionsString() {
	Option *option = NULL;
	string options;
	char codeStr[2] = {0, };
	map<char, Option*>::iterator it;
	int require;

	for (it = m_optionMap.begin(); it != m_optionMap.end(); it++) {
		option = it->second;
		if (option) {
			require = option->require();
			codeStr[0] = option->code();

			if (require & REQUIRE_OPT_VALUE) {
				options.append(codeStr);
				options.append(":");
			} else if ((require & REQUIRE_DEFAULT) || (require & REQUIRE_OPT)) {
				options.append(codeStr);
			}
		}
	}
	return options.c_str();
}

} /* namespace kinow */

#if 0
using namespace kinow;

int main(int argc ,char **argv) {
	Options options(argc, argv);
	char errbuf[64];
	bool valid = false;

	options.addOption('c', "conf file", Options::REQUIRE_OPT_VALUE, "test.conf");
	options.addOption('d', "daemonize");
	options.addOption('i', "interface", Options::REQUIRE_OPT|Options::REQUIRE_OPT_VALUE);
	options.addOptionHelp();

	valid = options.validOptions(errbuf, sizeof(errbuf));
	printf("check valid options = %s\n", valid ? "GOOD" : "BAD");
	if (valid == false) {
		printf("%s\n", errbuf);
	}

	/*
	options.showOptions();

	printf("c = %s\n", options.optionValue('c'));
	printf("i = %s\n", options.optionValue('i'));

	options.help();
	*/

	return 0;
}
#endif
