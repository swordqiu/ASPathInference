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




// VMEM.h: interface for the CVMEM class.
//
//////////////////////////////////////////////////////////////////////
//

#ifndef _CVMEM_H_
#define _CVMEM_H_

namespace bgplib {

class CVMEM
{
public:
	CVMEM();
	~CVMEM();
	size_t VRead(void* buf,size_t size);
	size_t VWrite(const void* buf,size_t size);
	off_t VSeek(off_t pos,int seekset, size_t expectSize = VMEM_FILE_BUFFER_SIZE);
	void* ReadVMEM(off_t pos, int seekset, size_t size);
	void* CurrentMem();
	off_t VTell();
	void Init(const char* filename, bool readonly = false);
	void Release();
	off_t GetSize();
private:
	off_t start;
	off_t file_end_ptr;
	off_t cur_pos;
	bool IsDirty;
	bool IsTMP;
	int  fd;
	char m_fn[MAX_PATH_LEN];
	unsigned char buffer[VMEM_FILE_BUFFER_SIZE];
	bool IsRDONLY;
};

} /* end of namespace bgplib */


#endif
