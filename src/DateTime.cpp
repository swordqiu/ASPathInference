/*
 *  Copyright 2003-2007 Jian Qiu <swordqiu@gmail.com>
 */

/*  This file is part of ASPathInference, a tool inferring AS level paths
 *  from any source AS to any destination prefix.
 *
 *  ASPathInference is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ASPathInference is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ASPathInference; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */




#include "StdAfx.h"
#include "DateTime.h"

namespace bgplib {

time_t gmmktime(struct tm *t) {
	struct tm tm_zero;
	tm_zero.tm_sec = tm_zero.tm_min = tm_zero.tm_hour = tm_zero.tm_mon = tm_zero.tm_wday = tm_zero.tm_yday = tm_zero.tm_isdst = 0;
	tm_zero.tm_mday = 1;
	tm_zero.tm_year = 70;
	time_t offset = mktime(&tm_zero);
	return mktime(t) - offset;
}

TimeInterval::TimeInterval()
{
   _m_time_t=0L;
}

TimeInterval::TimeInterval(double interval, const char* unit)
{
	TimeInterval();
	Set(interval,unit);
}

void TimeInterval::Set(double interval, const char* unit)
{
	if(strcmp(unit,"sec")==0)
		_m_time_t=(time_t)interval;
	else if(strcmp(unit,"min")==0)
		_m_time_t=(time_t)(interval*60);
	else if(strcmp(unit,"hour")==0)
		_m_time_t=(time_t)(interval*60*60);
	else if(strcmp(unit,"day")==0)
		_m_time_t=(time_t)(interval*60*60*24);
	else if(strcmp(unit,"week")==0)
		_m_time_t=(time_t)(interval*60*60*24*7);
	else
		_m_time_t=0;
}

TimeInterval::TimeInterval(const TimeInterval& ti)
{
	_m_time_t=ti._m_time_t;
}

const TimeInterval& operator - (const TimeInterval& ti1,const TimeInterval& ti2)
{
	static TimeInterval ti;
	ti._m_time_t=ti1._m_time_t-ti2._m_time_t;
	return ti;
}

const TimeInterval& operator + (const TimeInterval& ti1,const TimeInterval& ti2)
{
	static TimeInterval ti;
	ti._m_time_t=ti1._m_time_t+ti2._m_time_t;
	return ti;
}

static const char* _mon_name[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char* _mon_name_ful[]={"January","February","March","April","May","June","July","Augest","September","October","November","December"};
static const char* _week_name[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char* _week_name_ful[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

DateTime::DateTime()
{
	_m_time_t=time(NULL);
	memcpy((void*)&_m_time,(void*)gmtime(&_m_time_t),sizeof(struct tm));
}

DateTime::DateTime(const DateTime& dt)
{
   _m_time_t=dt._m_time_t;
   memcpy((void*)&_m_time,(void*)gmtime(&_m_time_t),sizeof(struct tm));
}

DateTime::DateTime(time_t rawtime)
{
	_m_time_t=rawtime;
	memcpy((void*)&_m_time,(void*)gmtime(&_m_time_t),sizeof(struct tm));
}

DateTime::DateTime(int tyear,int tmonth, int tday,int thour,int tminute,int tsecond)
{
	_m_time.tm_year=tyear-1900;
	_m_time.tm_mon=tmonth-1;
	_m_time.tm_mday=tday;
	_m_time.tm_hour=thour;
	_m_time.tm_min=tminute;
	_m_time.tm_sec=tsecond;
	_m_time_t=gmmktime(&_m_time);
}

DateTime::~DateTime()
{

}

char* DateTime::ToString() //char* fmt="%04d-%02d-%02d %02d:%02d:%02d")
{
	//static char temp[MAX_LEN];
	return ctime(&_m_time_t);
}

int DateTime::GetMonthIdx(char* name)
{
	for(int i=0;i<12;i++)
	{
		if(strncmp(_mon_name[i],name,3)==0)
		{
			return i;
		}
	}
	return -1;
}

const char* DateTime::GetMonthName(int idx)
{
	return _mon_name[idx];
}

int DateTime::GetWeekIdx(char* name)
{
	for(int i=0;i<7;i++)
	{
		if(strncmp(_week_name[i],name,3)==0)
		{
			return i;
		}
	}
	return -1;
}

const char* DateTime::GetWeekName(int idx)
{
	return _week_name[idx];
}

void DateTime::ParseStr(char* str, char* pattern) //y, Y, m, M, d, h, i, s, w, W, ?
{
	if(GetMatch(str,pattern,'y')!=NULL) //year
	{
		_m_time.tm_year=atoi(GetMatch(str,pattern,'y'));
	}
	if(GetMatch(str,pattern,'Y')!=NULL) //year
	{
		_m_time.tm_year=atoi(GetMatch(str,pattern,'Y'))-1900;
	}
	if(GetMatch(str,pattern,'m')!=NULL) //month
	{
		_m_time.tm_mon=atoi(GetMatch(str,pattern,'m'))-1;
	}
	if(GetMatch(str,pattern,'M')!=NULL) //month in verbose
	{
		_m_time.tm_mon=GetMonthIdx(GetMatch(str,pattern,'M'));
	}
	if(GetMatch(str,pattern,'d')!=NULL) //day
	{
		_m_time.tm_mday=atoi(GetMatch(str,pattern,'d'));
	}
	if(GetMatch(str,pattern,'h')!=NULL) //hour
	{
		_m_time.tm_hour=atoi(GetMatch(str,pattern,'h'));
	}
	if(GetMatch(str,pattern,'i')!=NULL) //minute
	{
		_m_time.tm_min=atoi(GetMatch(str,pattern,'i'));
	}
	if(GetMatch(str,pattern,'s')!=NULL) //second
	{
		_m_time.tm_sec=atoi(GetMatch(str,pattern,'s'));
	}
	if(GetMatch(str,pattern,'w')!=NULL) //week
	{
	}
	if(GetMatch(str,pattern,'W')!=NULL) //week in verbose
	{
	}
	//printf("%d %d %d %d %d %d\n", _m_time.tm_year, _m_time.tm_mon, _m_time.tm_mday, _m_time.tm_hour, _m_time.tm_min, _m_time.tm_sec);
	_m_time_t=gmmktime(&_m_time);
}

void DateTime::SetYear(int year) //1999-2010
{
	_m_time.tm_year=year-1900;
	_m_time_t=gmmktime(&_m_time);
}

void DateTime::SetMonth(int month) //1-12
{
   _m_time.tm_mon=month-1;
   _m_time_t=gmmktime(&_m_time);
}

void DateTime::SetDay(int day) //1-31
{
   _m_time.tm_mday=day;
   _m_time_t=gmmktime(&_m_time);
}

void DateTime::SetHour(int hour)
{
   _m_time.tm_hour=hour;
   _m_time_t=gmmktime(&_m_time);
}

void DateTime::SetMinute(int minite)
{
   _m_time.tm_min=minite;
   _m_time_t=gmmktime(&_m_time);
}

void DateTime::SetSecond(int second)
{
   _m_time.tm_sec=second;
   _m_time_t=gmmktime(&_m_time);
}

void DateTime::AddYears(int years)
{
	SetYear(GetYear()+1);
}

void DateTime::AddMonths(int months)
{
	int month=GetMonth()+months;
	if(month>12)
	{
		AddYears((int)ceil(month/12.0)-1);
		SetMonth(month-12*((int)ceil(month/12.0)-1));
	}else
	{
		SetMonth(month);
	}
}

void DateTime::AddDays(int days)
{
	_m_time_t+=days*24*3600;
	memcpy((void*)&_m_time,(void*)gmtime(&_m_time_t),sizeof(struct tm));
}

void DateTime::AddHours(int hours)
{
	_m_time_t+=hours*3600;
	memcpy((void*)&_m_time,(void*)gmtime(&_m_time_t),sizeof(struct tm));
}

void DateTime::AddSeconds(int seconds)
{
	_m_time_t+=seconds;
	memcpy((void*)&_m_time,(void*)gmtime(&_m_time_t),sizeof(struct tm));
}

int DateTime::GetYear()
{
	return _m_time.tm_year+1900;
}

int DateTime::GetMonth()
{
	return _m_time.tm_mon+1;
}

int DateTime::GetDay()
{
	return _m_time.tm_mday;
}

int DateTime::GetHour()
{
	return _m_time.tm_hour;
}

int DateTime::GetMinute()
{
	return _m_time.tm_min;
}

int DateTime::GetSecond()
{
	return _m_time.tm_sec;
}

int DateTime::GetWeekday()
{
	return _m_time.tm_wday;
}

char* DateTime::GetMatch(char* str, char* pattern, char machar)
{
	static char temp[MAX_LEN];
	int len1=strlen(pattern);
	int len2=strlen(pattern);
	int i=0,j=0;

	while(i<len1&&i<len2)
	{
		if(pattern[i]==machar)
		{
			temp[j]=str[i];
			j++;
		}
		i++;
	}
	temp[j]='\0';
	if(j==0)
		return NULL;
	else
		return temp;
}

const TimeInterval& operator- (const DateTime& dt1, const DateTime& dt2)
{
	static const TimeInterval ti(dt1._m_time_t-dt2._m_time_t, "sec");
	return ti;

}

const DateTime& operator+ (const DateTime& dt,const TimeInterval& ti)
{
	static DateTime dt1=dt;
	dt1._m_time_t+=ti._m_time_t;
	return dt1;
}

time_t DateTime::GetRawTime()
{
	return this->_m_time_t;
}

void DateTime::SetRawTime(time_t rawtime)
{
	_m_time_t = rawtime;
	//memcpy((void*)&_m_time, (void*)gmtime(&_m_time_t), sizeof(struct tm));
	gmtime_r(&_m_time_t, &_m_time);
}

char* DateTime::GetYearStr()
{
	static char temp[10];
	sprintf(temp,"%d",GetYear());
	return temp;
}

char* DateTime::GetMonthStr()
{
	static char temp[10];
	sprintf(temp,"%s", GetMonthName(GetMonth()-1));
	return temp;
}

char* DateTime::GetDayStr()
{
	static char temp[10];
	sprintf(temp,"%d",GetDay());
	return temp;
}

char* DateTime::GetHourStr()
{
	static char temp[10];
	sprintf(temp,"%d",GetHour());
	return temp;
}

char* DateTime::tostr()
{
	static char temp[100];
	sprintf(temp,"%04d-%02d-%02d %02d:%02d:%02d",GetYear(),GetMonth(),GetDay(),GetHour(),GetMinute(),GetSecond());
	return temp;
}

TimeVal::TimeVal() {
	m_tvsec = 0;
	m_tvusec = 0;
}

TimeVal::TimeVal(const TimeVal& clone) {
	m_tvsec = clone.m_tvsec;
	m_tvusec = clone.m_tvusec;
	adjust();
}

TimeVal::TimeVal(const struct timeval* tv) {
	m_tvsec  = tv->tv_sec;
	m_tvusec = tv->tv_usec;
	adjust();
}

TimeVal::TimeVal(const double tm) {
	m_tvsec = long(tm);
	m_tvusec = long((tm - m_tvsec)*TIMEVAL_USEC);
	adjust();
}

const double TimeVal::toSeconds() {
	return m_tvsec + m_tvusec*1.0/TIMEVAL_USEC;
}

const TimeVal TimeVal::operator - (const TimeVal& tv2) const{
	TimeVal tv(*this);
	tv.m_tvusec -= tv2.m_tvusec;
	tv.m_tvsec  -= tv2.m_tvsec;
	tv.adjust();
	return tv;
}

const TimeVal TimeVal::operator + (const TimeVal& tv2) const{
	TimeVal tv(*this);
	tv.m_tvusec += tv2.m_tvusec;
	tv.m_tvsec  += tv2.m_tvsec;
	tv.adjust();
	return tv;
}

int TimeVal::compareTo(const TimeVal& tv) {
	if(m_tvsec != tv.m_tvsec) {
		return m_tvsec - tv.m_tvsec;
	}else {
		return m_tvusec - tv.m_tvusec;
	}
}

bool TimeVal::operator > (const TimeVal& tv) {
	return (compareTo(tv) > 0);
}

bool TimeVal::operator >= (const TimeVal& tv) {
	return (compareTo(tv) >= 0);
}

bool TimeVal::operator < (const TimeVal& tv) {
	return (compareTo(tv) < 0);
}

bool TimeVal::operator <= (const TimeVal& tv) {
	return (compareTo(tv) <= 0);
}

bool TimeVal::operator == (const TimeVal& tv) {
	return (compareTo(tv) == 0);
}

bool TimeVal::operator != (const TimeVal& tv) {
	return (compareTo(tv) != 0);
}

void TimeVal::adjust() {
	long sec = long(m_tvusec*1.0/TIMEVAL_USEC);
	if(sec != 0) {
		m_tvsec  += sec;
		m_tvusec -= sec*TIMEVAL_USEC;
	}
}

char *TimeVal::tostr() const {
	static char temp[32];
	DateTime dt(m_tvsec);
	sprintf(temp, "%02d%02d%02d.%06ld", dt.GetHour(), dt.GetMinute(), dt.GetSecond(), m_tvusec);
	return temp;
}



}
