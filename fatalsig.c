#define UNW_LOCAL_ONLY

#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ucontext.h>
#include <libunwind.h>

#define FATALSIG_UNWIND_NAME_MAX_				256

static void
fatalsig_backtrace(void)
{
	unw_cursor_t cursor;
	unw_context_t uc;
	int ret = 0;

	unw_flush_cache(unw_local_addr_space, 0, 0);

	if ((ret = unw_getcontext(&uc)) != 0) {
		syslog(LOG_ERR,
			"unable to get a stack unwind context: %s",
			unw_strerror(ret));
		return;
	}

	if ((ret = unw_init_local(&cursor, &uc)) != 0) {
		syslog(LOG_ERR,
			"stack unwind initialization failed: %s",
			unw_strerror(ret));
		return;
	}

	while ((ret = unw_step(&cursor)) > 0) {
		char name[FATALSIG_UNWIND_NAME_MAX_];
		unw_word_t ip, off;

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		ret = unw_get_proc_name(&cursor, name, sizeof(name), &off);

		syslog(LOG_ERR,
			"  0x%0*" PRIxPTR ": %s%s+0x%" PRIxPTR "\n",
			(int) (2 * sizeof(void *)),
			(uintptr_t) ip,
			(ret == 0) ? name : "<unknown>",
			(ret == 0) ? "()" : "",
			(uintptr_t) off);
	}

	if (ret < 0) {
		syslog(LOG_ERR, "stack unwind step failed: %s", unw_strerror(ret));
	}
}

static void
fatalsig_action(int signo, siginfo_t *info, void *ctx)
{
	struct sigaction sa;

	syslog(LOG_ERR, "program caught a fatal signal: %s (%d)",
		strsignal(signo) != NULL ? strsignal(signo) : "Unknown",
		signo);

	fatalsig_backtrace();

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0;

	if (sigemptyset(&sa.sa_mask) != 0 ||
		sigaction(signo, &sa, NULL) != 0 ||
		kill(getpid(), signo) < 0)
	{
		syslog(LOG_ERR, "failed to propagate a signal");
	}
}

int fatalsig_init(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = &fatalsig_action;
	sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;

	if ((sigemptyset(&sa.sa_mask) != 0) ||
		(sigaction(SIGSEGV, &sa, NULL) != 0) ||
		(sigaction(SIGBUS,  &sa, NULL) != 0) ||
		(sigaction(SIGILL,  &sa, NULL) != 0) ||
		(sigaction(SIGABRT, &sa, NULL) != 0) ||
		(sigaction(SIGFPE,  &sa, NULL) != 0) ||
		(sigaction(SIGSYS,  &sa, NULL) != 0))
	{
		return -1;
	}

	return 0;
}

