/*
 * IDaemon.h
 *
 *  Created on: 2016. 9. 23.
 *      Author: root
 */

#ifndef FOOLIB_SRC_DAEMON_DAEMON_H_
#define FOOLIB_SRC_DAEMON_DAEMON_H_

#include <time.h>
#include "Error.h"

namespace kinow {

class IDaemon : public Error {
public:
	IDaemon(const char *name, int argc, char **argv);
	virtual ~IDaemon();
	void daemonize(bool daemonize) { m_daemonize = daemonize; }
	/**
	 * execute daemon process
	 */
	bool run();
	bool isRunning();
	/**
	 * current daemon process id
	 */
	pid_t getPID() { return m_pid; }

public:
	/**
	 * get pid from pid file
	 * ex> /var/run/foo.pid : 'foo' is process name
	 */
	static pid_t extractPID(const char *pidPath, const char* pname);

	/**
	 * get process running name from proc system.
	 * proc location: /proc/<pid>/cmdline
	 */
	static bool existPIDProc(pid_t pid);

	/**
	 * write pid to pid file
	 * ex> /var/run/foo.pid : 'foo' is process name
	 */
	static bool recordPID(const char *pidPath, const char* pname, pid_t pid);

protected:
	int m_argc;
	char **m_argv;
	char *m_name;
	bool m_debug;
	bool m_daemonize;
	bool m_multiRunning;

	virtual bool init(int argc, char** argv) = 0;
	virtual bool exec() = 0;
	virtual void release() = 0;
	virtual void setEnv() = 0;
	virtual void summary() = 0;

private:
	time_t m_uptime;
	pid_t m_pid;
	pid_t setDaemon();
};

} /* namespace kinow */

#endif /* FOOLIB_SRC_DAEMON_DAEMON_H_ */
