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




#ifndef _DATE_TIME_CLASS_H
#define _DATE_TIME_CLASS_H

#include "StdAfx.h"

namespace bgplib {

time_t gmmktime(struct tm *tm);

class DateTime;

class TimeInterval
{
public:
	TimeInterval();
	TimeInterval(double interval, const char* unit);
	TimeInterval(const TimeInterval& ti);
private:
	time_t _m_time_t;
public:
	void Set(double interval, const char* unit);
	friend const TimeInterval& operator - (const TimeInterval& ti1,const TimeInterval& ti2);
	friend const TimeInterval& operator + (const TimeInterval& ti1,const TimeInterval& ti2);
	friend const DateTime& operator + (const DateTime& dt, const TimeInterval& ti);

	TimeInterval& operator -= (const TimeInterval& ti)
	{
		_m_time_t-=ti._m_time_t;
		return *this;
	}

	TimeInterval& operator += (const TimeInterval& ti)
	{
		_m_time_t+=ti._m_time_t;
		return *this;
	}

	bool operator == (const TimeInterval& ti)
	{
		return _m_time_t==ti._m_time_t;
	}
	bool operator> (const TimeInterval& ti)
	{
		return _m_time_t>ti._m_time_t;
	}
	bool operator>= (const TimeInterval& ti)
	{
		return _m_time_t>=ti._m_time_t;
	}
	bool operator< (const TimeInterval& ti)
	{
		return _m_time_t<ti._m_time_t;
	}
	bool operator<= (const TimeInterval& ti)
	{
		return _m_time_t<=ti._m_time_t;
	}
};

class DateTime
{
public:
	DateTime();
	DateTime(time_t raw_time);
	DateTime(const DateTime& dt);
	DateTime(int year,int month=1,int day=1,int hour=0,int minute=0,int second=0);
	virtual ~DateTime();
public:
	char* ToString();
	char* tostr();
	void ParseStr(char* str, char* pattern=NULL);
	int GetMonthIdx(char* name);
	const char* GetMonthName(int idx);
	int GetWeekIdx(char* name);
	const char* GetWeekName(int idx);
	void AddYears(int years);
	void AddMonths(int months);
	void AddDays(int days);
	void AddHours(int hours);
	void AddSeconds(int seconds);
	void SetYear(int year); //1999-2010
	void SetMonth(int month); //1-12
        void SetDay(int day); //1-31
	void SetHour(int hour);
	void SetMinute(int minite);
	void SetSecond(int second);     
	int GetYear();
	int GetMonth();
	int GetDay();
	int GetHour();
	int GetMinute();
	int GetSecond();
	int GetWeekday();
	char* GetYearStr();
	char* GetMonthStr();
	char* GetDayStr();
	char* GetHourStr();
	time_t GetRawTime();
	void SetRawTime(time_t settm);
public:
	friend const TimeInterval& operator- (const DateTime& dt1,const DateTime& dt2);
	friend const DateTime& operator+ (const DateTime& dt,const TimeInterval& ti);

	bool operator== (const DateTime& dt)
	{
		return _m_time_t==dt._m_time_t;
	}
	bool operator!= (const DateTime& dt)
	{
		return _m_time_t!=dt._m_time_t;
	}
	bool operator> (const DateTime& dt)
	{
		return _m_time_t>dt._m_time_t;
	}
	bool operator>= (const DateTime& dt)
	{
		return _m_time_t>=dt._m_time_t;
	}
	bool operator< (const DateTime& dt)
	{
		return _m_time_t<dt._m_time_t;
	}
	bool operator<= (const DateTime& dt)
	{
		return _m_time_t<=dt._m_time_t;
	}
private:
	struct tm _m_time;
	time_t _m_time_t;
private:
	char* GetMatch(char* str, char* pattern, char machar);

};

#define TIMEVAL_USEC 1000000

class TimeVal {
public:
	TimeVal();
	TimeVal(const TimeVal& clone);
	TimeVal(const struct timeval* tv);
	TimeVal(const double tv);
public:
	long m_tvsec;
	long m_tvusec;
public:
	const double toSeconds();
	const TimeVal operator - (const TimeVal& tv2) const;
	const TimeVal operator + (const TimeVal& tv2) const;
	bool operator > (const TimeVal& tv);
	bool operator >= (const TimeVal& tv);
	bool operator < (const TimeVal& tv);
	bool operator <= (const TimeVal& tv);
	bool operator == (const TimeVal& tv);
	bool operator != (const TimeVal& tv);

	int compareTo(const TimeVal& tv);

	char* tostr() const;

private:
	void adjust();
};

} /* end of namespace bgplib */

#endif
