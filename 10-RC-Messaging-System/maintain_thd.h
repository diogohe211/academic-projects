
#ifndef MAINTIAN_THD_H
#define MAINTIAN_THD_H
#include "protocol.h"

int ParseLineData(char *line, StUserInfo *stUserInfo);
void *RunMaintainThread(void);
#endif

