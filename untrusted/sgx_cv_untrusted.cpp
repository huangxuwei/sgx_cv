#include "opencv2/core/core.hpp"
#include "OpencvCore_u.h"

void ocall_getTickCount(long int *tickCount)
{
    *tickCount = cv::getTickCount();
}

void ocall_getNumberOfCPUs(int *cpuNum)
{
    *cpuNum = 0;
}

