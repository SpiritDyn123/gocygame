/*
 * TimeUtils.cc
 *
 *  Created on: 2017年3月22日
 *      Author: antigloss
 */
#include <sstream>
#include <iomanip>
#include "TimeUtils.h"

using namespace std;

tm* GetNowTm()
{
	time_t t = time(nullptr);
	return localtime(&t);
}

void ResetMidnightTm(tm& tmNow)
{
	tmNow.tm_hour = 0;
	tmNow.tm_min = 0;
	tmNow.tm_sec = 0;
}

time_t NextDay()
{
	tm tmNow(*GetNowTm());
	ResetMidnightTm(tmNow);
	return mktime(&tmNow) + 86400;
}
time_t LastDay()
{
	tm tmNow(*GetNowTm());
	ResetMidnightTm(tmNow);
	return mktime(&tmNow);
}

time_t NextMonday()
{
	tm tmNow(*GetNowTm());
	ResetMidnightTm(tmNow);
	int diffDay = 0;
	switch (tmNow.tm_wday) {
	case 0:
		diffDay = 1;
		break;
	default:
		diffDay = 8 - tmNow.tm_wday;
		break;
	}
	return mktime(&tmNow) + 86400 * diffDay;
}
time_t LastMonday()
{
	return NextMonday() - 86400 * 7;
}

time_t NextMonth()
{
	tm tmNow(*GetNowTm());
	ResetMidnightTm(tmNow);
	tmNow.tm_mday = 1;
	if (tmNow.tm_mon == 11) {
		tmNow.tm_mon = 0;
		tmNow.tm_year += 1;
	} else {
		tmNow.tm_mon += 1;
	}
	return mktime(&tmNow);
}

time_t LastMonth()
{
	tm tmNow(*GetNowTm());
	ResetMidnightTm(tmNow);
	tmNow.tm_mday = 1;
	if (tmNow.tm_mon == 0) {
		tmNow.tm_mon = 11;
		tmNow.tm_year -= 1;
	} else {
		tmNow.tm_mon -= 1;
	}
	return mktime(&tmNow);
}

uint32_t TransTime(vector<int> timeVec)
{
	tm tmTime;
	tmTime.tm_year = timeVec[0] - 1900;
	tmTime.tm_mon = timeVec[1] - 1;
	tmTime.tm_mday = timeVec[2];
	tmTime.tm_hour = timeVec[3];
	tmTime.tm_min = timeVec[4];
	tmTime.tm_sec = timeVec[5];
	return mktime(&tmTime);
}

double GetTimeTail()
{
	return (kMaxTimeStamp - time(nullptr)) / 1000000000.0;
}

int WeekCycle()
{
	return (time(nullptr) - kBaseTimeStamp) / (7*24*3600) + 1;
}

string GetTodayStr()
{
	tm t(*GetNowTm());
	stringstream ss;
	ss << 1900 + t.tm_year << '-' << setw(2) << setfill('0') << t.tm_mon + 1 << '-' << setw(2) << setfill('0') << t.tm_mday;
	return ss.str();
}

string GetYesterdayStr()
{
	time_t lastday = LastDay();
	tm t = *gmtime(&lastday);
	stringstream ss;
	ss << 1900 + t.tm_year << '-' << setw(2) << setfill('0') << t.tm_mon + 1 << '-' << setw(2) << setfill('0') << t.tm_mday;
	return ss.str();
}

int DayDiff(time_t time1, time_t time2)
{
	tm pTm1 = *gmtime(&time1);
	tm pTm2 = *gmtime(&time2);
	ResetMidnightTm(pTm1);
	ResetMidnightTm(pTm2);

	time1 = mktime(&pTm1);
	time2 = mktime(&pTm2);

	return (time1 - time2) / 86400;
}

int64_t TimeDiffForMs(timeval& tv1, timeval& tv2)
{
	int64_t tv1ms = tv1.tv_sec * 1000 + tv1.tv_usec / 1000;
	int64_t tv2ms = tv2.tv_sec * 1000 + tv2.tv_usec / 1000;
	return tv2ms - tv1ms;
}

uint32_t DayNumber()
{
	time_t tmStamp = time(nullptr) - 5 * 3600;
	tm t;
	localtime_r(&tmStamp, &t);

	uint32_t number = (1900 + t.tm_year) * 10000 + t.tm_mon * 100 + t.tm_mday;
	return number;
}
