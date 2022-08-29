#ifndef __FATALSIG_H__
#define __FATALSIG_H__

int fatalsig_init(void);
void fatalsig_stacktrace(int signo);

#endif /* __FATALSIG_H__ */

