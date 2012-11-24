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




// MyUtil.cpp: implementation of the MyUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MyUtil.h"
#include "BufFile.h"
//#include "MySQLUtil.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UINT128::UINT128()
{
	a=0;
	b=0;
	c=0;
	d=0;
}

UINT128::UINT128(UINT128& uint)
{
	a=uint.a;
	b=uint.b;
	c=uint.c;
	d=uint.d;
}

UINT128::~UINT128()
{
}

MyUtil::MyUtil()
{

}

MyUtil::~MyUtil()
{

}

int getstring(char* dest,char* src,int start, char sep)
{
	int len=strlen(src);
	int i=start;
	int j=0;

	bool firstin=true;
	while(i<len&&(isprint(src[i])))
	{
		if(src[i] == sep)
		{
			if(firstin)
				i++;
			else
				break;
		}else
		{
			firstin=false;
			dest[j]=src[i];
			j++;
			i++;
		}
	}
	dest[j]='\0';
	return i;
}

void QTRACE(const char *fmt, ...)
{
	char buf[MAX_LEN];

	va_list ap;
	va_start(ap,fmt);

	vsprintf(buf,fmt,ap);
	printf("%s: %s",gettick(),buf);

	va_end(ap);
	return;
}

int getnstring(char* dest,char* src,int start,int len)
{
	int slen=strlen(src);
	int j=0,i;

	for(i=start;i<slen&&i<start+len;i++)
	{
		dest[j]=src[i];
		j++;
	}
	dest[j]='\0';
	return i;
}

int strchrcount(char* str, char ch)
{
	int count=0;
	for(unsigned int i=0;i<strlen(str);i++)
	{
		if(str[i]==ch)
			count++;
	}
	return count;
}

char* gettick()
{
	static char temp[50];
	time_t x=time(NULL);
	sprintf(temp,"%ld",x);
	return temp;
}

bool str2long(char* str, long &num)
{
	bool IsNormal=true;
	int len=strlen(str);
	for(int i=0;i<len;i++)
	{
		if(!isdigit(str[i]))
		{
			IsNormal=false;
			str[i]=' ';
		}
	}
	num=atol(str);
	return IsNormal;
}

bool MyUtil::writelog(char* dir, char* filename,char* str)
{
	char temp[MAX_LEN];
	sprintf(temp,"%s/%s",dir,filename);
	FILE* fp;
	if((fp=fopen(temp,"a"))==NULL)
	{
		printf("Error in open log file %s\n",temp);
		if((fp=fopen(temp,"w"))==NULL)
		{
			printf("Error in create log file %s\n",temp);
			return false;
		}
	}
	fprintf(fp,"%s\n",str);
	fclose(fp);
	return true;
}

unsigned int MyUtil::INET_A2N(char* addr)
{
	char* p;
	char temp[5];
	unsigned int a,b,c,d;

	p=strchr(addr,'.');
	memset(temp,0,5);
	strncpy(temp,addr,p-addr);
	a=atoi(temp);
	addr=p+1;
	p=strchr(addr,'.');
	memset(temp,0,5);
	strncpy(temp,addr,p-addr);
	b=atoi(temp);
	addr=p+1;
	p=strchr(addr,'.');
	memset(temp,0,5);
	strncpy(temp,addr,p-addr);
	c=atoi(temp);
	addr=p+1;
	d=atoi(addr);
	return (a<<24)|(b<<16)|(c<<8)|d;
}

char* MyUtil::INET_N2A(unsigned int addr)
{
	static char temp[16];

	memset(temp,0,16);
	int a,b,c,d;
	a=addr>>24;
	b=(addr&(0x00ff0000))>>16;
	c=(addr&(0x0000ff00))>>8;
	d=addr&(0x000000ff);
	sprintf(temp,"%d.%d.%d.%d",a,b,c,d);
	return temp;
}

char* MyUtil::ConverStd6(char* addr)
{
	static char temp[1024];
	char* p;

	p=strstr(addr, "::");
	if(p==NULL)
		strcpy(temp,addr);
	else
	{
		if(p==addr)
			strcpy(temp,"0");
		else
		{
			strncpy(temp,addr,p-addr);
			temp[p-addr]='\0';
		}
		p=p+2;
		int n=strchrcount(temp,':')+strchrcount(p,':');
		for(int i=0;i<6-n;i++)
		{
			strcat(temp,":0");
		}
		if(strlen(p)==0)
		{
			strcat(temp,":0");
		}else
		{
			strcat(temp,":");
			strcat(temp,p);
		}
	}
	return temp;
}

UINT_64 MyUtil::INET6_A2N(char* addr)
{
        char* p;
        char temp[5];
        UINT_64 a,b,c,d;

	addr=ConverStd6(addr);

        p=strchr(addr,':');
        memset(temp,0,5);
        strncpy(temp,addr,p-addr);
        a=hexchar2long(temp);
        addr=p+1;
        p=strchr(addr,':');
        memset(temp,0,5);
        strncpy(temp,addr,p-addr);
        b=hexchar2long(temp);
        addr=p+1;
        p=strchr(addr,':');
        memset(temp,0,5);
        strncpy(temp,addr,p-addr);
        c=hexchar2long(temp);
        addr=p+1;
        p=strchr(addr,':');
        memset(temp,0,5);
        strncpy(temp,addr,p-addr);
        d=hexchar2long(temp);
        return (a<<48)|(b<<32)|(c<<16)|d;
}

unsigned long MyUtil::hexchar2long(char* hex)
{
	unsigned long temp;
	if(strlen(hex)==0)
		temp=0;
	else
		sscanf(hex,"%lx", &temp);
	return temp;
}


char* MyUtil::INET6_N2A(UINT_64 addr)
{
        static char temp[100];

        UINT_64 MASK;
        MASK=0x0000ffff;
	u_int32_t a, b, c, d;
        a = (u_int32_t) (addr>>48);
        b = (u_int32_t) ((addr&(MASK<<32)) >> 32);
        c = (u_int32_t) ((addr&(MASK<<16)) >> 16);
        d = (u_int32_t) (addr& (MASK));
        if(d!=0)
                sprintf(temp,"%x:%x:%x:%x::",a,b,c,d);
        else if(d==0 && c!=0)
                sprintf(temp,"%x:%x:%x::",a,b,c);
        else if(d==0 && c==0 && b!=0)
                sprintf(temp,"%x:%x::",a,b);
        else if(d==0 && c==0 && b==0 && a!=0)
                sprintf(temp,"%x::",a);
        else if(d==0 && c==0 && b==0 && a==0)
                sprintf(temp,"::1");

        return temp;
}

unsigned int MyUtil::GetOriginASN(char* aspath)
{
	char temp[1024];
	int start=0;
	unsigned int tmpas=0;
	bool cont=true;

	while(cont)
	{
		start=getstring(temp,aspath,start);
		start++;
		if(strlen(temp)==1 && (temp[0]=='i' || temp[0]=='e' || temp[0]=='?'))
		{
			if(temp[0]=='i')
			{
			}else if(temp[0]=='e')
			{
			}else if(temp[0]=='?')
			{
			}
			cont=false;
		}else
		{
			tmpas=atoi(temp);
		}
	}
	return tmpas;
}

char* MyUtil::ConvNStd6(char* address)
{
	static char temp[MAX_ADDRESS_LEN];
	char mode[64];
	
	for(int i=8;i>0;i--)
	{
		strcpy(mode,"0");
		for(int j=1;j<i;j++)
		{
			strcat(mode,":0");
		}
		char* p=strstr(address,mode);
		if(p!=NULL)
		{
			if(p==address)
				strcpy(temp,":");
			else
			{
				strncpy(temp,address,p-address);
				temp[p-address]='\0';
			}
			p+=strlen(mode);
			if(p<address+strlen(address))
			{
				strcat(temp,p);
			}else
			{
				strcat(temp,":");
			}
			if(strcmp(temp,"::")==0)
				strcat(temp,"0");
			return temp;
		}
	}
	strcpy(temp,address);
	return temp;
}

void MyUtil::ConvType(void* data, unsigned int size)
{
	unsigned char* temp=(unsigned char*)data;
	unsigned char dummy;

	for(unsigned int i=0;i<size/2;i++)
	{
		dummy=temp[i];
		temp[i]=temp[size-1-i];
		temp[size-1-i]=dummy;
	}
}

void MyUtil::Sort(void* data, size_t len, unsigned int count, int (*compfunc)(void* data1,void* data2), bool asdent)
{
	unsigned char *datai;
	unsigned char *dataj;
	unsigned char *tmpdata;
	tmpdata=new unsigned char[len];
	int result;

	for(unsigned int i=0;i<count-1;i++)
	{
		datai=((unsigned char*)data)+len*i;
		for(unsigned int j=i+1;j<count;j++)
		{
			dataj=((unsigned char*)data)+len*j;	
			result=(*compfunc)(datai,dataj);
			if((asdent&&result>0) || (!asdent && result<0))
			{
				memcpy(tmpdata,datai,len);
				memcpy(datai,dataj,len);
				memcpy(dataj,tmpdata,len);
			}
		}
	}

	delete tmpdata;
}

void MyUtil::ShowPath(u_int32_t* pathstr, u_int8_t pathlen) 
{
	if(pathlen > 0)
	{
		printf("%s", asn_n2a(pathstr[0]));
		for(int i = 1; i < pathlen; i ++) {
			printf(" %s", asn_n2a(pathstr[i]));
		}
		printf("\n");
	}
}

bool MyUtil::isPathStatus(char* str) {
	if(str[0] != '*' && str[0] != ' ') {
		return false;
	}
	for(int i = 1; i < 3; i ++) {
		if(strchr("sdh>iS ", str[i]) == NULL) {
			return false;
		}
	}
	if(str[0] == ' ' && str[1] == ' ' && str[1] == ' ') {
		return false;
	}
	return true;
}

void ASSERT(bool val, const char* format, ...) {
        char myformat[1024] = "[ASSERT ERROR] ";
        va_list args;

        if(!val) {
                strcat(myformat, format);
                va_start(args, format);
                vfprintf(stderr, myformat, args);
                va_end(args);
                exit(-1);
        }
}

bool safe_sprintf(char* dest, size_t size, char* format, ...) {
        char temp[65535];
        va_list args;
        va_start(args, format);
        vsprintf(temp, format, args);
        va_end(args);
        if(strlen(temp) >= size) {
                memcpy(dest, temp, size - 1);
                dest[size-1] = '\0';
                return false;
        }else {
                strcpy(dest, temp);
                return true;
        }
}

u_int32_t asn_a2n(const char* asn_str) {
	const char* dot_ptr = strchr(asn_str, '.');
	if(dot_ptr != NULL) {
		char temp[32];
		strncpy(temp, asn_str, dot_ptr - asn_str);
		u_int32_t asn = atoi(temp);
		strcpy(temp, dot_ptr+1);
		asn = (asn << 16) + atoi(temp);
		return asn;
	}else {
		return (u_int32_t)atoi(asn_str);
	}
}

const char* asn_n2a(u_int32_t asn) {
	static char temp[32];
	if (asn > 65535) {
		u_int32_t a = (asn >> 16);
		u_int32_t b = (asn & 0xFFFF);
		sprintf(temp, "%d.%d", a, b);
		return temp;
	}else {
		sprintf(temp, "%d", asn);
		return temp;
	}
}

//// IANA: http://www.iana.org/assignments/as-numbers
bool is_valid_asn(u_int32_t asn) {
    if (asn == 0 ||
            asn == 23456 ||
            (asn >= 61440 && asn <= 65535) ||
            (asn >= 65536 && asn <= 131071) ||
            (asn >= 133120 && asn <= 196607) ||
            (asn >= 199680 && asn <= 262143) ||
            (asn >= 263168 && asn <= 327679) ||
            (asn >= 328704 && asn <= 393215) ||
            asn > 394240) {
        return false;
	}else{
		return true;
	}
}

