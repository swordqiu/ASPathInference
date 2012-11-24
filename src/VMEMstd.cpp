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




// VMEM.cpp: implementation of the CVMEM class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
//#include "MyUtil_templ.h"
#include "MyUtil.h"
#include "VMEMstd.h"
#include "VMEM.h"

namespace bgplib {

CVMEM::CVMEM()
{
	fd      = -1;
	start   = 0;
	file_end_ptr = -1;
	cur_pos = 0;
	IsDirty = false;
	IsTMP   = false;
	IsRDONLY = false;
	bzero(m_fn, sizeof(m_fn));
	bzero(buffer, sizeof(buffer));
}

void CVMEM::Init(const char *filename, bool readonly)
{
	IsRDONLY = readonly;
	//printf("isRDONly: %s\n", readonly?"YES":"NO");
	start = 0;
	file_end_ptr = 0;
	cur_pos = 0;
	IsDirty = false;
	bzero(m_fn, sizeof(m_fn));
	bzero(buffer, sizeof(buffer));

	if(filename == NULL) {
		IsTMP = true;
		IsRDONLY = false;
		sprintf(m_fn, "%s/vmem.XXXXXX", P_tmpdir);
		if((fd = mkstemp(m_fn)) < 0) 
		{
			printf("Error open temporary file for VMEM!\n");
			exit(-1);
		}
	}else {
		IsTMP = false;

		ASSERT(strlen(filename) <= MAX_PATH_LEN, "[CVMEM::Init] too long filename!\n");

		strcpy(m_fn, filename);
		bzero(buffer, sizeof(buffer));

		int flag;
		if(IsRDONLY) {
			flag = O_RDONLY; 
		}else {
			flag = O_RDWR; 
		}
		mode_t mode = S_IRUSR | S_IWUSR;

		struct stat ft;
		if(stat(filename, &ft) == 0) {
			file_end_ptr = ft.st_size;
		}else if(errno == ENOENT) { 
			flag = flag | O_CREAT;
			file_end_ptr = 0L;
		}else {
			printf("Error open VMEM file %s!\n", filename);
			exit(1);
		} 

		if((fd = open(filename, flag, mode)) < 0) 
		{
			printf("Error open VMEM file %s!\n", filename);
			exit(1);
		}
		lseek(fd, 0L, SEEK_SET);
		ssize_t readsize;
		if(file_end_ptr < VMEM_FILE_BUFFER_SIZE) {
			readsize = file_end_ptr;
		} else {
			readsize = VMEM_FILE_BUFFER_SIZE;
		}
		if(readsize > 0) 
		{
			if(read(fd, (void*)buffer, readsize) < readsize)
			{
				printf("[VMEM::Init]: read vmem error!\n");
				exit(-1);
			}
		}
	}
	//printf("INIT: start: %lld eof_ptr: %lld cur_pos: %lld\n", start, file_end_ptr, cur_pos);
}

off_t CVMEM::GetSize()
{
	return file_end_ptr;
}

void CVMEM::Release() {
	//printf("[CVMEM::Release] %d\n", fd);
	if(fd >= 0 && file_end_ptr >= 0) {
		if(IsDirty && !IsTMP) {
			ssize_t writesize;
			if(file_end_ptr < start+VMEM_FILE_BUFFER_SIZE)
				writesize = file_end_ptr-start;
			else
				writesize = VMEM_FILE_BUFFER_SIZE;
			if(writesize>0 && !IsRDONLY)
			{
				if(lseek(fd, start, SEEK_SET) == -1 || write(fd, (const void*)buffer, writesize) < writesize)
				{
					printf("[VMEM::Release]: write vmem error!\n");
					exit(-1);
				}
			}
			IsDirty=false;
		}
		close(fd);
		if(IsTMP || file_end_ptr == 0) {
			unlink(m_fn);
		}
		file_end_ptr = -1;
		fd = -1;
	}
}

CVMEM::~CVMEM()
{
	Release();
}

size_t CVMEM::VRead(void* buf, size_t size)
{
	ASSERT(size <= VMEM_FILE_BUFFER_SIZE, "[CVMEM::VRead]: the read size is larger than the buffer size\n");
	//printf("[CVMEM::VRead]: VRead: start: %lld winsize: %d eof_ptr: %lld cur_pos: %lld size: %d\n", start, VMEM_FILE_BUFFER_SIZE, file_end_ptr, cur_pos, size);
	if((off_t)(cur_pos+size)>=(off_t)(start+VMEM_FILE_BUFFER_SIZE) || cur_pos<(off_t)start || cur_pos > (off_t)(start + VMEM_FILE_BUFFER_SIZE))
	{
		//printf("[CVMEM::VRead]: Read out of window data, shifting ....\n");
		if(IsDirty && !IsRDONLY)
		{
			ssize_t writesize;
			if(file_end_ptr-start<VMEM_FILE_BUFFER_SIZE) {
				writesize=file_end_ptr-start;
			} else {
				writesize=VMEM_FILE_BUFFER_SIZE;
			}
			//printf("[CVMEM::VRead]: write dirty data into disk at %lld with size %d\n", start, writesize);
			if(writesize>0 && !IsRDONLY)
			{
				if(lseek(fd, start, SEEK_SET) == -1 || write(fd, (const void*)buffer, writesize) < writesize)
				{
					printf("[VMEM:VRead]: error write vmem file!\n");
					exit(-1);
				}
			}
			IsDirty=false;
		}
		start=cur_pos;
		ssize_t readsize;
		if(start>file_end_ptr-VMEM_FILE_BUFFER_SIZE) {
			readsize=file_end_ptr-start;
		} else {
			readsize=VMEM_FILE_BUFFER_SIZE;
		}
		//printf("[CVMEM::VRead]: Shift to window at %lld size %d", start, readsize);
		if(readsize>0)
		{
			if(lseek(fd, start, SEEK_SET) == -1 || read(fd, (void*)buffer, readsize) < readsize)
			{
				printf("[VMEM:VRead]: read vmem file error!\n");
				exit(-1);
			}
		}
	}
	if((off_t)(cur_pos+size)<(off_t)(start+VMEM_FILE_BUFFER_SIZE) && cur_pos>=(off_t)start && cur_pos < (off_t)file_end_ptr)
	{
		size_t readsize;
		if((off_t)(cur_pos+size)>file_end_ptr)
			readsize=file_end_ptr-cur_pos;
		else
			readsize=size;
		memcpy(buf,(const void*)(&buffer[cur_pos-start]),readsize);
		cur_pos=cur_pos+readsize;
		return readsize;
	}else
	{
		return 0;
	}
}

size_t CVMEM::VWrite(const void* buf,size_t size)
{
	ASSERT(size <= VMEM_FILE_BUFFER_SIZE, "[CVMEM::VWrite]: the write size is larger than the buffer size\n");
	//printf("[CVMEM::VWrite]: VWrite: start: %lld winsize: %d eof_ptr: %lld cur_pos: %lld size: %d\n", start, VMEM_FILE_BUFFER_SIZE, file_end_ptr, cur_pos, size);
	//printf("VWrite: start: %lld eof_ptr: %lld cur_pos: %lld\n", start, file_end_ptr, cur_pos);
	if((off_t)(cur_pos+size)>=(off_t)(start+VMEM_FILE_BUFFER_SIZE) || cur_pos<(off_t)start || cur_pos>(off_t)(start+VMEM_FILE_BUFFER_SIZE))
	{
		//printf("[CVMEM::VWrite]: Out of window, shifting ...\n");
		if(IsDirty && !IsRDONLY)
		{
			ssize_t writesize;
			if(file_end_ptr-start<VMEM_FILE_BUFFER_SIZE) {
				writesize=file_end_ptr-start;
			} else {
				writesize=VMEM_FILE_BUFFER_SIZE;
			}
			//printf("[CVMEM::VWrite]: Current window is dirty, write back at %lld size %d", start, writesize);
			if(writesize>0 && !IsRDONLY)
			{
				if(lseek(fd, start, SEEK_SET) == -1 || write(fd, (const void*)buffer, writesize) < writesize)
				{
					printf("[VMEM::VWrite]: error write vmem file!\n");
					exit(-1);
				}
			}
			IsDirty=false;
		}
		start=cur_pos;
		ssize_t readsize;
		if(start>file_end_ptr-VMEM_FILE_BUFFER_SIZE) {
			readsize=file_end_ptr-start;
		} else {
			readsize=VMEM_FILE_BUFFER_SIZE;
		}
		//printf("[CVMEM::VWrite]: Shift to window at %lld size %d", start, readsize);
		if(readsize>0)
		{
			if(lseek(fd, start, SEEK_SET) == -1 || read(fd, (void*)buffer, readsize) < readsize)
			{
				printf("[VMEM::VWRite]: read vmem file error!\n");
				exit(-1);
			}
		}
	}
	if((off_t)(cur_pos+size)<(off_t)(start+VMEM_FILE_BUFFER_SIZE) && cur_pos>=(off_t)start && cur_pos <= file_end_ptr)
	{
		if((off_t)(cur_pos+size)>file_end_ptr) {
			file_end_ptr=cur_pos+size;
		}
		memcpy((void*)(&buffer[cur_pos-start]),buf,size);
		cur_pos=cur_pos+size;
		IsDirty=true;
		return size;
	}else
	{
		return 0;
	}
}

void* CVMEM::ReadVMEM(off_t pos, int seekset, size_t size) {
	if(VSeek(pos, seekset, size) != -1) {
		return (void*)(&buffer[cur_pos - start]);
	}else {
		return NULL;
	}
}

void* CVMEM::CurrentMem() {
	return (void*)(&buffer[cur_pos - start]);
}

off_t CVMEM::VSeek(off_t pos, int seekset, size_t expectSize)
{
	ASSERT(expectSize <= VMEM_FILE_BUFFER_SIZE, "[CVMEM::VSeek]: expcted size is larger than buffer size!\n");
	//printf("VSeek: start: %lld eof_ptr: %lld cur_pos: %lld\n", start, file_end_ptr, cur_pos);
	//printf("[CVMEM::VSeek]: start: %lld winsize: %d eof_ptr: %lld cur_pos: %lld expectSize: %d\n", start, VMEM_FILE_BUFFER_SIZE, file_end_ptr, cur_pos, expectSize);
	switch(seekset) {
	case VSEEK_END:
		pos = file_end_ptr - pos;
		break;
	case VSEEK_CUR:
		pos = cur_pos + pos;
		break;
	case VSEEK_SET:
		break;
	default:
		return -1;
	}

	if(pos<0 || (pos>file_end_ptr && IsRDONLY)) { // out of range of file
		return -1;
	}
	if(pos>=(off_t)start && (off_t)(pos + expectSize) < (off_t)(start+VMEM_FILE_BUFFER_SIZE)) { // && pos <= file_end_ptr) { // no need to shift window
		cur_pos=pos;
		return pos;
	}
	//printf("[CVMEM::VSeek] pos %lld is out of window %lld shifting window ...\n", pos, start);
	if(IsDirty && !IsRDONLY)
	{
		ssize_t writesize;
		if(file_end_ptr<start+VMEM_FILE_BUFFER_SIZE) {
			writesize=file_end_ptr-start;
		} else {
			writesize=VMEM_FILE_BUFFER_SIZE;
		}
		//printf("[CVMEM::VSeek] current window is dirty, write back at %lld with size %d\n", start, writesize);
		if(writesize>0 && !IsRDONLY)
		{
			if(lseek(fd, start, SEEK_SET) == -1 || write(fd, (const void*)buffer, writesize) < writesize)
			{
				printf("[VMEM::VSeek]: write vmem error!\n");
				exit(-1);
			}
		}
		IsDirty=false;
	}
	start = pos; //VMEM_FILE_BUFFER_SIZE*(pos/VMEM_FILE_BUFFER_SIZE);
	ssize_t readsize;
	bool resize = false;
	if(!IsRDONLY && pos > file_end_ptr) {
		file_end_ptr = pos;
		resize = true;
	}
	if(start>file_end_ptr-VMEM_FILE_BUFFER_SIZE) {
		readsize = file_end_ptr - start;
	} else {
		readsize = VMEM_FILE_BUFFER_SIZE;
	}
	if(readsize>0)
	{
		if(resize) {
			bzero((void*)buffer, sizeof(u_int8_t)*readsize);
		} else {
			if(lseek(fd, start, SEEK_SET) == -1 || read(fd, (void*)buffer, readsize) < readsize) {
				printf("[VMEM::VSeek]: read vmem file error!\n");
				exit(-1);
			}
		}
	}
	cur_pos=pos;
	return pos;
}

off_t CVMEM::VTell()
{
	return cur_pos;
}

} /* end of namespace bgplib */


