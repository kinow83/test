/*
 * Daemon.cpp
 *
 *  Created on: 2016. 9. 23.
 *      Author: root
 */

#include <glob.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <IDaemon.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "IOUtils.h"
#include "IDaemon.h"

namespace kinow {

IDaemon::IDaemon(const char *name, int argc, char **argv) {
	int pnameLen = 0;
	m_name = strdup(name);
	m_argc = argc;
	m_argv = argv;
	m_uptime = 0;
	m_debug = false;
	m_daemonize = true;
	m_multiRunning = false;
	m_pid = 0;

	if (argc > 0 && argv && argv[0]) {
		pnameLen = strlen(m_argv[0]);
		memset(m_argv[0], 0, pnameLen);
		snprintf(m_argv[0], 32, "%s", m_name);
	}
}

IDaemon::~IDaemon() {
	if (m_name) {
		free(m_name);
	}
	m_pid = 0;
}

bool IDaemon::run() {
	bool ok = false;
	m_uptime = time(NULL);

	/**
	 * initialize daemon (config or usage ...)
	 */
	if (!init(m_argc, m_argv)) {
		failure("Daemon[%s]: Failed to init\n", m_name);
		return false;
	}

	if (m_debug) {
		summary();
	}

	if (m_multiRunning == false) {
		if (isRunning()) {
			failure("Daemon[%s]: already running...\n", m_name);
			return false;
		}
	}

	if (m_daemonize) {
		m_pid = setDaemon();
	} else {
		m_pid = getpid();
	}

	if (recordPID("/var/run", m_name, m_pid) == false) {
		failure("Daemon[%s]: Failed to write pid\n", m_name);
		return false;
	}

	/**
	 * set environment configure.
	 */
	setEnv();

	/**
	 * execute deamon
	 */
	ok = exec();

	/**
	 * release resource
	 */
	release();

	return ok;
}

pid_t IDaemon::setDaemon()
{
	pid_t pid = 0;

	// fork error
	if ((pid = fork()) < 0) {
		exit(0);
	}
	// parent process
	else if (pid != 0) {
		exit(0);
	}

	setsid();

	close(0);
	close(1);
	close(2);

	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	return pid;
}


bool IDaemon::isRunning() {
	pid_t pid = 0;

	pid = extractPID("/var/run", m_name);
	if (pid == -1) {
		return false;
	}
	if (pid == 0) {
		// invalid pid
		return false;
	}
	if (existPIDProc(pid)) {
		return true;
	}
	return false;
}

/**
 * get pid number
 * pid file: pidpath/pname.pid
 */
pid_t IDaemon::extractPID(const char *pidPath, const char* pname) {
	char pidfile[256] = {0, };
	char pidstr[256] = {0, };

	snprintf(pidfile, sizeof(pidfile), "%s/%s.pid", pidPath, pname);

	if (IOUtils::extractContent(pidPath, pidstr, sizeof(pidstr)) == -1) {
		return -1;
	}
	return (pid_t)strtoul(pidstr, NULL, 10);
}

bool IDaemon::existPIDProc(pid_t pid) {
	char cmd[256] = {0, };
	char procfile[256] = {0, };

	snprintf(procfile, sizeof(procfile), "/proc/%d/cmdline", pid);
	if (IOUtils::extractContent(procfile, cmd, sizeof(cmd)) == -1) {
		return false;
	}

	// compare daemon name with process name.
	return !(strncmp(cmd, m_name, strlen(m_name)));
}

bool IDaemon::recordPID(const char *pidPath, const char* pname, pid_t pid) {
	char pidfile[256] = {0, };
	char pidstr[256] = {0, };

	snprintf(pidfile, sizeof(pidfile), "%s/%s.pid", pidPath, pname);
	snprintf(pidstr, sizeof(pidstr), "%d", pid);

	if (IOUtils::recordContent(pidfile, pidstr) == -1) {
		return false;
	}
	return true;
}


} /* namespace kinow */
