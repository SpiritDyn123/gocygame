/*
 * TimeUtils.h
 *
 *  Created on: 2017年3月22日
 *      Author: antigloss
 */

#ifndef DIGIMON_TIMEUTILS_H_
#define DIGIMON_TIMEUTILS_H_

#include <string>
#include <vector>

static const std::string kBaseTimeStr("1502640000"); // 2017-08-14 00:00:00
static const time_t kBaseTimeStamp(1502640000); // 2017-08-14 00:00:00
static const time_t kMaxTimeStamp(2000000000); // 2033-05-18 11:33:20,随便写的,用来加进排行榜

tm* GetNowTm();

time_t NextDay();
time_t NextMonday();
time_t NextMonth();
time_t LastDay();
time_t LastMonday();
time_t LastMonth();

uint32_t TransTime(std::vector<int> timeVec);
int DayDiff(time_t time1, time_t time2);
int64_t TimeDiffForMs(timeval& tv1, timeval& tv2);

double GetTimeTail();
int WeekCycle();
std::string GetTodayStr();
std::string GetYesterdayStr();
// 20171109
uint32_t DayNumber();

#endif /* DIGIMON_TIMEUTILS_H_ */
