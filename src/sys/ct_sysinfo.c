/**
 * @file ct_sysinfo.c
 * @brief 系统信息
 * @author tayne3@dingtalk.com
 * @date 2023.12.25
 */
#include "ct_sysinfo.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_sysinfo]"

// -------------------------[GLOBAL DEFINITION]-------------------------

bool ct_sysinfo_process_name(char* name, size_t max)
{
	if (!name || !max) {
		return false;
	}

#ifdef CT_OS_LINUX
	{
		FILE* const file = fopen("/proc/self/comm", "r");
		if (!file) {
			return false;
		}
		if (!fgets(name, max - 1, file)) {
			return false;
		}
		fclose(file);
	}

	name[strlen(name) - 1] = '\0';
	return true;
#else
	return false;
#endif
}

int ct_sysinfo_cpu_cores(void)
{
	int cores = 0;
	{
#ifdef CT_OS_WIN
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		cores = sysinfo.dwNumberOfProcessors;
#else
		cores = get_nprocs();
#endif
	}
	return cores;
}

pid_t ct_sysinfo_process_id(void)
{
	return getpid();
}

// -------------------------[STATIC DEFINITION]-------------------------
