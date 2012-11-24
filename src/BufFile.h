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




#ifndef _BUFFER_FILE_H
#define _BUFFER_FILE_H

namespace bgplib {

#define BUF_FILE_WRITE_MODE 0
#define BUF_FILE_READ_MODE  1
#define BUF_FILE_READ_BINARY_MODE 2
#define BUF_FILE_WRITE_BINARY_MODE 3

//#define FILE_BUFFER_SIZE (4096*8)
#define FILE_BUFFER_SIZE (4096)

class BufFile{
public:
	BufFile();
	BufFile(BufFile& clone);
	bool Open(char* filename,int mode=BUF_FILE_READ_MODE);
	bool Close();
	char* getnextstr();
	bool setnextstr(char* str);
	bool isEnd();
	void* read(unsigned int len);
	bool write(void* data, unsigned int len);
	bool rewind(unsigned int len);
private:
	FILE* fp;
	char Buffer[FILE_BUFFER_SIZE];
	unsigned int count;
	unsigned int searchreturn(unsigned int start);
	bool endoffile;
	bool notsufficientbytes;
	unsigned int m_mode; 
	unsigned int m_end_ptr;
};

}

#endif
