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




// Buffer file class 

#include "StdAfx.h"
#include "BufFile.h"

namespace bgplib {

BufFile::BufFile()
{
	count=0;
}

bool BufFile::Open(char* filename,int wmode)
{
	m_mode=wmode;
	char mode[10];
	if(wmode==BUF_FILE_WRITE_MODE)
		strcpy(mode,"w");
	else if(wmode==BUF_FILE_READ_MODE)
		strcpy(mode,"r");
	else if(wmode==BUF_FILE_READ_BINARY_MODE)
		strcpy(mode,"rb");
	else if(wmode==BUF_FILE_WRITE_BINARY_MODE)
		strcpy(mode,"wb");

	if((fp=fopen(filename,mode))==NULL)
	{
		printf("Open buffer file error!\n");
		return false;
	}

	endoffile=false;
	notsufficientbytes = false;
	if(m_mode==BUF_FILE_WRITE_MODE || m_mode==BUF_FILE_WRITE_BINARY_MODE)
	{
		count=0;
	}
	else if(m_mode==BUF_FILE_READ_MODE || m_mode==BUF_FILE_READ_BINARY_MODE)
	{
		count=FILE_BUFFER_SIZE;
		memset((void*)Buffer, 0, count);
		unsigned int countread=fread((void*)Buffer, sizeof(unsigned char), count, fp);
		if(countread < count)
			endoffile=true;
		m_end_ptr=countread;
		count = 0;
	}
	return true;
}

bool BufFile::Close()
{
	if((m_mode==BUF_FILE_WRITE_MODE||m_mode==BUF_FILE_WRITE_BINARY_MODE) && count>0)
	{
		fwrite(Buffer,count,1,fp);
	}
	if(fclose(fp)==0)
		return true;
	else
		return false;
}

char* BufFile::getnextstr()
{
	if(m_mode==BUF_FILE_WRITE_MODE)
		return NULL;
	if(count>=FILE_BUFFER_SIZE||searchreturn(count)>=FILE_BUFFER_SIZE)
	{
		if(searchreturn(count)>=FILE_BUFFER_SIZE&&endoffile) {
			notsufficientbytes = true;
			return NULL;
		}
		int tmp=FILE_BUFFER_SIZE-count;
		if(tmp>0)
			memcpy(Buffer,&Buffer[count],tmp);
		char* buffer_ptr=Buffer+tmp;
		m_end_ptr=tmp;
		tmp=FILE_BUFFER_SIZE-tmp;
		memset((void*)buffer_ptr,0,tmp);
		int countread=fread((void*)buffer_ptr,sizeof(unsigned char),tmp,fp);
		if(countread<tmp)
			endoffile=true;
		m_end_ptr+=countread;
		count=0;
	}
	if(count>=m_end_ptr) return NULL;
	int endchar=searchreturn(count);
	Buffer[endchar]='\0';
	int temp;
	temp=count;
	count=endchar+1;
	return (char*)&Buffer[temp];
}

bool BufFile::rewind(unsigned int len) {
	if(count >= len) {
		count = count - len;
		return true;
	}else {
		//printf("rewind: %d\n", - (len + m_end_ptr - count));
		if(fseek(fp, - (len + m_end_ptr - count), SEEK_CUR) == 0) {
			unsigned int tmp=FILE_BUFFER_SIZE;
			memset((void*)Buffer, 0, tmp);
			unsigned int countread=fread((void*)Buffer, sizeof(unsigned char), tmp, fp);
			if(countread<tmp) {
				endoffile = true;
			}else {
				endoffile = false;
			}
			count = 0;
			m_end_ptr = countread;
			return true;
		}
	}
	return false;
}

void* BufFile::read(unsigned int len)
{
	if(m_mode==BUF_FILE_WRITE_MODE || len > FILE_BUFFER_SIZE) {
		return NULL;
	}
	if(count>=FILE_BUFFER_SIZE || (count+len)>=FILE_BUFFER_SIZE)
	{
		if((count+len)>=FILE_BUFFER_SIZE&&endoffile) {
			notsufficientbytes = true;
			return NULL;
		}
		unsigned int tmp=FILE_BUFFER_SIZE-count;
		if(tmp>0)
			memcpy(Buffer,&Buffer[count],tmp);
		char* buffer_ptr=Buffer+tmp;
		m_end_ptr=tmp;
		tmp=FILE_BUFFER_SIZE-tmp;
		memset((void*)buffer_ptr,0,tmp);
		unsigned int countread=fread((void*)buffer_ptr,sizeof(unsigned char),tmp,fp);
		if(countread<tmp) {
			endoffile = true;
		}else {
			endoffile = false;
		}
		m_end_ptr+=countread;
		count=0;
	}
	if(count>=m_end_ptr) return NULL;
	unsigned int endchar=count+len;
	//Buffer[endchar]='\0';
	unsigned int temp;
	temp=count;
#ifdef _SET_DEBUG_
	printf("count=%d endchar=%d\n", count, endchar);fflush(stdout);
#endif
	count=endchar;
	return (void*)&Buffer[temp];
}

bool BufFile::setnextstr(char* str)
{
	if(m_mode==BUF_FILE_READ_MODE)
		return false;

	if(count>=FILE_BUFFER_SIZE || count+strlen(str)+1>FILE_BUFFER_SIZE)
	{
		fwrite(Buffer,count,1,fp);
		count=0;
	}
	strcpy((char*)(Buffer+count),str);
	count+=strlen(str);
	Buffer[count]='\n';
	count++;
	return true;
}

bool BufFile::write(void* data, unsigned int len)
{
	if(m_mode==BUF_FILE_READ_MODE)
		return false;

	if(count>=FILE_BUFFER_SIZE || count+len>FILE_BUFFER_SIZE)
	{
		fwrite(Buffer,count,1,fp);
		count=0;
	}
	memcpy((void*)(Buffer+count),data,len);
	count+=len;
	
	return true;
}

unsigned int BufFile::searchreturn(unsigned int start)
{
	int i=start;
	while(i<FILE_BUFFER_SIZE)
	{
		if(Buffer[i] == '\n' || Buffer[i] == '\0' || Buffer[i] == (char)EOF)
		{
			return i;
		}
		i++;
	}
	return i;
}

BufFile::BufFile(BufFile &clone)
{

}

bool BufFile::isEnd()
{
	if(endoffile==true && (count>=m_end_ptr || notsufficientbytes)) {
		return true;
	}else {
		return false;
	}
}

}
