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

#ifndef _CSVMEM_H_
#define _CSVMEM_H_

namespace bgplib {

class CSVMEM  
{
public:
	CSVMEM(size_t size, const char* filename = NULL, bool rdonly = false);
	CSVMEM();
	~CSVMEM();

public:
	long length;

public:
	bool  InitSMEM(size_t size, const char* filename = NULL, bool readonly = false);
	void  Release(); // can be called explicitly or implicitly
	bool  Add(void* data);
	void* Get(long index);
	bool  Set(void* data, long index);
	bool  Insert(void* data, long index);
	long  SplitFind(void* data, int (*cmpfunc)(void* param1,void* param2));
	long  Find(void* data,long startpos=0);
	long  Find(void* data, int (*cmpfunc)(void* param1,void* param2),long startpos=0);
	void  Sort(int sorttype);
	void  Sort(int sorttype,int (*cmpfunc)(void* param1,void* param2));
	void  QuickSort(int (*cmpfunc)(void* param1,void* param2));
private:
	CVMEM vm;
	size_t datasize;
	unsigned char* m_data;
	long Partition(long low,long high,int (*cmpfunc)(void* param1,void* param2));
	void QSort(long low,long high,int (*cmpfunc)(void* param1,void* param2));
};

class COrderedSVMEM: public CSVMEM {
public:
	bool Add(void* data);
	bool Add(void* data, int (*cmpfunc)(void*, void*, void*), void* pram);
	bool Find(void* data, long *pstart, long* pend);
	bool Find(void* data, long *pstart, long* pend, int (*cmpfunc)(void*, void*, void*), void* param);
	void registerProcess(int (*cmpfunc)(void*, void*, void*), void* pram);
private:
	int (*m_cmpfunc)(void* dat1, void* dat2, void* param);
	void* proc_param;
};

class CSVMEMArray: public CSVMEM
{
public:
	CSVMEMArray();
	bool Add(long data);
	long GetData(long index);
};

class CDVMEMData
{
public:
	long size; // size in terms of bytes
	void* array_ptr;
	CDVMEMData();
	//virtual size_t WriteToFile(FILE* fp){return 0;};
	//size_t ReadFromFile(FILE* fp);
};

class CDynaArray
{
private:
	long v_size;  // maximal size in terms of objects
	size_t datasize; // size of each object
	long size; // size in terms of bytes
	u_int8_t* array_ptr;

public:
	//CDynaArray();
	CDynaArray(size_t size);
	//CDynaArray(CDVMEMData* data, size_t size);
	CDynaArray(const CDynaArray& arr);
	~CDynaArray();

	//bool Release();
	//bool ArrayInit(size_t size);

	size_t GetSize();
	//void Reset();
	bool Add(void* data);
	bool RemoveAll();
	void* Get(long index);
	bool Set(void* data,long index);
	void Insert(void* data, long index);
	void Remove(long index);
	void Trim();
	long GetLength();
	long Find(void* data,long startpos=0);
	long Find(void* data, int (*cmpfunc)(void* param1,void* param2),long startpos=0);
	void Sort(int sorttype, int (*cmpfunc)(void* param1,void* param2));
	void QuickSort(int (*cmpfunc)(void* param1,void* param2, void* dat), void *data=NULL);
	long QuickFind(void* data, int (*cmpfunc)(void* param1, void* param2), int sorttype);
	void Append(CDynaArray* parr);
	//size_t WriteToFile(FILE* fp);
private:
	long Partition(long low,long high,int (*cmpfunc)(void* param1,void* param2,void* data));
	void QSort(long low,long high,int (*cmpfunc)(void* param1,void* param2,void* data));
	void *sortdata;
};

class CDVMEM  
{
public:
	CDVMEM();
	virtual ~CDVMEM();

	//bool InitDMEM(size_t size);
	bool Add(CDVMEMData* data);
	CDVMEMData* Get(long index);
	size_t GetLength();

private:
	CVMEM vm;
	CSVMEMArray data;
	unsigned char* m_data;
	void ReleaseData();
	void InitData(size_t size);
};


typedef struct _TDP{
	double x;
	double y;
	double z;
}POINT3D;

typedef struct _aselement{
	long index;
	long array_idx;
}ASELE;

class ASStack{
private:
	ASELE stack[AS_STACK_DEPTH];
	int top;
public:
	ASStack();
	ASELE* gettop();
	ASELE* pop();
	int getdepth();
	void push(ASELE* ele);
	bool IsEmpty();
};

} /* end of namespace bgplib */

#endif 
