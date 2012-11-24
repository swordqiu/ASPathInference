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
#include "MyUtil.h"
#include "VMEMstd.h"
#include "VMEM.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace bgplib {

CSVMEM::CSVMEM()
{
	//printf("CSVMEM::CSVMEM()\n");
	m_data=NULL;
	length=0;
	datasize=0;
}

CSVMEM::CSVMEM(size_t size, const char *filename, bool rdonly)
{
	//printf("CSVMEM::CSVMEM(size_t size, const char *filename, bool rdonly)\n");
	InitSMEM(size, filename, rdonly);
}

bool CSVMEM::InitSMEM(size_t size, const char *filename, bool rdonly)
{
	//printf("CSVMEM::InitSMEM(size_t size, const char *filename, bool rdonly)\n");
	datasize=size;
	m_data=new u_int8_t[datasize];
	bzero(m_data, sizeof(u_int8_t)*datasize);
	ASSERT(m_data != NULL, "[CSVMEM::InitSMEM] cannot allocate memory for buffer!\n");
	memset(m_data,0,datasize);
	vm.Init(filename, rdonly);
	length = vm.GetSize()/datasize;
	return true;
}

void CSVMEM::Release()
{
	//printf("CSVMEM::Release()\n");
	if(m_data != NULL)
	{
		delete[] m_data;
		m_data = NULL;
		vm.Release();
	}
}

CSVMEM::~CSVMEM()
{
	//printf("CSVMEM::~CSVMEM()\n");
	if(m_data!=NULL)
	{
		this->Release();
	}
}

long CSVMEM::SplitFind(void* data, int (*cmpfunc)(void* param1,void* param2))
{
	long idx1=0;
	long idx2=length-1;
	long tmp;
	int result;

	while(idx1<=idx2)
	{
		tmp=(idx1+idx2)>>1;
		result=(*cmpfunc)(data,Get(tmp));
		if(result==0)
			return tmp;
		if(result<0)
			idx2=tmp-1;
		else
			idx1=tmp+1;
	}
	return -1;
}

long CSVMEM::Find(void* data, int (*cmpfunc)(void* param1,void* param2),long startpos)
{
	for(long i=startpos;i<length;i++)
	{
		if((*cmpfunc)(Get(i),data)==0)
			return i;
	}
	return -1;
}

long CSVMEM::Find(void* data,long startpos)
{
	for(long i=startpos;i<length;i++)
	{
		if(memcmp(Get(i),data,datasize)==0)
			return i;
	}
	return -1;
}

long CSVMEM::Partition(long low,long high,int (*cmpfunc)(void* param1,void* param2))
{
	u_int8_t *pivotkey = new u_int8_t[datasize];
	bzero(pivotkey, sizeof(u_int8_t)*datasize);
	memcpy(pivotkey,Get(low),datasize);
	while(low<high)
	{
		while(low<high&&(*cmpfunc)(Get(high),pivotkey)>=0) high--;
		Set(Get(high),low);
		while(low<high&&(*cmpfunc)(Get(low), pivotkey)<=0) low++;
		Set(Get(low),high);
	}
	Set(pivotkey,low);
	delete[] pivotkey;
	return low;
}

void CSVMEM::QSort(long low,long high,int (*cmpfunc)(void* param1,void* param2))
{
	ASStack asstack;
	ASELE asele, *asele_ptr,asele1,asele2;
	long pivotpos;

	asele.array_idx=low;
	asele.index=high;
	asstack.push(&asele);

	while(!asstack.IsEmpty())
	{
		asele_ptr=asstack.pop();
		asele1.array_idx=asele_ptr->array_idx;
		asele2.index=asele_ptr->index;

		pivotpos=Partition(asele_ptr->array_idx,asele_ptr->index,(*cmpfunc));
		asele1.index=pivotpos-1;
		asele2.array_idx=pivotpos+1;
		if(asele1.array_idx<asele1.index)
			asstack.push(&asele1);
		if(asele2.array_idx<asele2.index)
			asstack.push(&asele2);
	}

	/*if(low<high)
	{
		QSort(low,pivotpos-1,(*cmpfunc));
		QSort(pivotpos+1,high,(*cmpfunc));
	}*/
}

void CSVMEM::QuickSort(int (*cmpfunc)(void* param1,void* param2))
{
	QSort(0,length-1,(*cmpfunc));
}

void CSVMEM::Sort(int sorttype, int (*cmpfunc)(void* param1,void* param2))
{
	long i,j;
	u_int8_t *tmpi,*tmpj;
	tmpi = new u_int8_t[datasize];
	tmpj = new u_int8_t[datasize];
	bzero(tmpi, sizeof(u_int8_t)*datasize);
	bzero(tmpj, sizeof(u_int8_t)*datasize);
	for(i=0;i<length-1;i++)
	{
		for(j=i+1;j<length;j++)
		{
			memcpy(tmpi,Get(i),datasize);
			memcpy(tmpj,Get(j),datasize);
			if(((*cmpfunc)(tmpi,tmpj)>0&&sorttype==SORT_ASCENDENT)
				||((*cmpfunc)(tmpi,tmpj)<0&&sorttype==SORT_DESCENDENT))
			{
				Set(tmpi,j);
				Set(tmpj,i);
			}
		}
	}
	delete[] tmpi;
	delete[] tmpj;
}

void CSVMEM::Sort(int sorttype)
{
	long i,j;
	u_int8_t *tmpi,*tmpj;
	tmpi = new u_int8_t[datasize];
	tmpj = new u_int8_t[datasize];
	bzero(tmpi, sizeof(u_int8_t)*datasize);
	bzero(tmpj, sizeof(u_int8_t)*datasize);
	for(i=0;i<length-1;i++)
	{
		for(j=i+1;j<length;j++)
		{
			memcpy(tmpi,Get(i),datasize);
			memcpy(tmpj,Get(j),datasize);
			if((memcmp(tmpi,tmpj,datasize)>0&&sorttype==SORT_ASCENDENT)
				||(memcmp(tmpi,tmpj,datasize)<0&&sorttype==SORT_DESCENDENT))
			{
				Set(tmpi,j);
				Set(tmpj,i);
			}
		}
	}
	delete[] tmpi;
	delete[] tmpj;
}

bool CSVMEM::Set(void* data, long index)
{
	off_t pos = index;
	pos = pos*datasize;
	vm.VSeek(pos, VSEEK_SET);
	if(vm.VWrite(data,datasize)>0)
		return true;
	else
		return false;
}

bool CSVMEM::Add(void* data)
{
	vm.VSeek(0L,VSEEK_END);
	if(vm.VWrite(data, datasize)>0)
	{
		length++;
		return true;
	}
	else
		return false;
}

void* CSVMEM::Get(long index)
{
	if(index < 0 || index >= length) {
		return NULL;
	}

	off_t pos = index;
	pos = pos*datasize;
	vm.VSeek(pos, VSEEK_SET);
	if(vm.VRead(m_data,datasize)>0)
		return m_data;
	else
		return NULL;
}

bool CSVMEM::Insert(void* data, long index) {
	Add(data);
	for(long i = length - 1; i > index; i --) {
		if(Get(i-1) != NULL && Set(m_data, i)) {
		}else {
			return false;
		}
	}
	if(index < length - 1 && Set(data, index)) {
		return true;
	}else {
		return false;
	}
}

void COrderedSVMEM::registerProcess(int (*func)(void*, void*, void*), void* param) {
	m_cmpfunc = func;
	proc_param = param;
}

	        bool Find(void* data, long *pstart, long* pend);
			 
bool COrderedSVMEM::Add(void* data, int (*cmpfunc)(void*, void*, void*), void* param) {
	long idx1 = -1, idx2 = -1;
	if(!Find(data, &idx1, &idx2, cmpfunc, param)) {
		Insert(data, idx1);
		return true;
	}else {
		return false;
	}
}

bool COrderedSVMEM::Add(void* data) {
	return Add(data, m_cmpfunc, proc_param);
}


bool COrderedSVMEM::Find(void* data, long *pstart, long* pend) {
	return Find(data, pstart, pend, m_cmpfunc, proc_param);
}

bool COrderedSVMEM::Find(void* data, long *pstart, long* pend, int (*cmpfunc)(void*, void*, void*), void* param) {
	long start = 0;
	long end = length - 1;
	long j;
	int result;
	while(start <= end) {
		j = (start + end)/2;
		result = (*cmpfunc)(data, Get(j), param);
		if(result > 0) {
			start = j + 1;
		}else if(result < 0) {
			end = j - 1;
		}else {
			if(pstart != NULL) {
				*pstart = j;
				while(*pstart > 0) {
					result = (*cmpfunc)(data, Get(*pstart - 1), param);
					if(result == 0) {
						*pstart = *pstart - 1;
					}else {
						break;
					}
				}
			}
			if(pend != NULL) {
				*pend = j;
				while(*pend < length - 1) {
					result = (*cmpfunc)(data, Get(*pend + 1), param);
					if(result == 0) {
						*pend = *pend + 1;
					}else {
						break;
					}
				}
			}
			return true;
		}
	}
	if(pstart!= NULL) {
        	*pstart = start;
		*pend = start;
	}
	return false;
}

CSVMEMArray::CSVMEMArray()
{
	CSVMEM::InitSMEM(sizeof(long));
}

bool CSVMEMArray::Add(long data)
{
	return CSVMEM::Add((void*)&data);
}

long CSVMEMArray::GetData(long index)
{
	return (long)(*((long*)Get(index)));
}

CDVMEM::CDVMEM()
{
	m_data=NULL;
}

CDVMEM::~CDVMEM()
{
}

bool CDVMEM::Add(CDVMEMData* vdata)
{
	if(vm.VSeek(0L,VSEEK_END)>=0)
	{
		long pos=vm.VTell();
		data.Add(pos);
	}else
		return false;
	if(vm.VWrite((const void*)&vdata->size,sizeof(size_t))>0
		&&vm.VWrite((const void*)vdata->array_ptr,vdata->size)>0)
		return true;
	else
		return false;
}

void CDVMEM::ReleaseData()
{
	if(m_data!=NULL)
	{
		delete[] m_data;
		m_data=NULL;
	}
}

void CDVMEM::InitData(size_t size)
{
	if(m_data!=NULL)
	{
		ReleaseData();
	}
	m_data=new unsigned char[size];
	bzero(m_data, sizeof(u_int8_t)*size);
}

CDVMEMData* CDVMEM::Get(long index)
{
	long *pos=(long*)data.Get(index);
	if(vm.VSeek(*pos,VSEEK_SET)<0)
	{
		return NULL;
	}
	static CDVMEMData dvmem;
	if(vm.VRead((void*)&dvmem.size,sizeof(size_t))>0)
	{
		InitData(dvmem.size);
		vm.VRead(m_data,dvmem.size);
		dvmem.array_ptr=m_data;
		return &dvmem;
	}
	else
		return NULL;
}

size_t CDVMEM::GetLength()
{
	return data.length;
}

CDVMEMData::CDVMEMData()
{
	size      = 0;
	array_ptr = NULL;
}

CDynaArray::CDynaArray(size_t sz)
{
	datasize  = sz;
	size      = 0;
	v_size    = DYNA_ARRAY_INIT_SIZE;
	array_ptr = new u_int8_t[datasize*v_size];
	bzero(array_ptr, datasize*v_size);
}

CDynaArray::CDynaArray(const CDynaArray& arr)
{
	datasize     =  arr.datasize;
	size         =  arr.size;
	v_size       =  size/datasize;
	array_ptr    =  new u_int8_t[size];
	bzero(array_ptr, size);
	memcpy(array_ptr, arr.array_ptr, size);
}

/*
CDynaArray::CDynaArray(CDVMEMData* data, size_t dsize)
{
	datasize  = dsize;
	size      = data->size;
	v_size    = data->size/dsize;
	if(array_ptr != NULL) {
		delete array_ptr;
		array_ptr = NULL;
	}
	array_ptr = new u_int8_t[size];
	bzero(array_ptr, sizeof(u_int8_t)*size);
	memcpy(array_ptr, data->array_ptr, size);
}*/

CDynaArray::~CDynaArray()
{
	delete[] array_ptr;
	array_ptr = NULL;
	v_size = 0;
	datasize = 0;
	size = 0;
}

bool CDynaArray::RemoveAll()
{
	size = 0;
	bzero(array_ptr, v_size*datasize);
	return true;
}

size_t CDynaArray::GetSize()
{
	return datasize;
}


long CDynaArray::GetLength()
{
	return size/datasize;
}

long CDynaArray::Find(void* data,long startpos)
{
	void* dest;
	if(startpos < 0) {
		startpos = 0;
	}
	for(long i=startpos;i<GetLength();i++)
	{
		dest=(void*)(array_ptr+i*datasize);
		if(memcmp(dest,data,datasize)==0)
			return i;
	}
	return -1;
}

long CDynaArray::QuickFind(void* data, int (*cmpfunc)(void* param1, void* param2), int sorttype)
{
	long startpos, endpos, midpos;
	int value;
	startpos = 0;
	endpos = GetLength()-1;

	while(startpos < endpos)
	{
		midpos = (startpos + endpos)/2;
		value = cmpfunc(data, Get(midpos));

		if(value == 0)
		{
			return midpos;
		}else if((sorttype == SORT_ASCENDENT && value < 0) || (sorttype == SORT_DESCENDENT && value > 0))
		{
			endpos = midpos - 1;
		}else
		{
			startpos = midpos + 1;
		}
	}
	return -1;
}

bool CDynaArray::Add(void* data)
{
	if(GetLength() >= v_size)
	{
		v_size = v_size + DYNA_ARRAY_STEP_SIZE;
		size_t newsize = v_size*datasize;
		u_int8_t *newptr= new u_int8_t[newsize];
		bzero(newptr, newsize);
		if(newptr==NULL)
		{
			printf("Error: cannot allocate new memory!");
			return false;
		}
		memcpy(newptr, array_ptr, size);
		delete[] array_ptr;
		//free(array_ptr);
		array_ptr = newptr;
		newptr = NULL;
	}
	memcpy((array_ptr + size), data, datasize);
	size += datasize;
	return true;
}

void CDynaArray::Append(CDynaArray* parr) {
	ASSERT(this->datasize == parr->datasize, "[CDynaArray::Append] Append incompatible CDynaArray!");

	for(long i = 0; i < parr->GetLength(); i++) {
		this->Add(parr->Get(i));
	}
}

void* CDynaArray::Get(long index)
{
	if(index>=GetLength() || index<0L)
		return NULL;
	else
		return (void*)(array_ptr + index*datasize);
}

bool CDynaArray::Set(void* data,long index)
{
	if(index>=GetLength() || index<0L)
		return false;
	else
	{
		memcpy((void*)(array_ptr+index*datasize),data,datasize);
		return true;
	}
}

void CDynaArray::Insert(void* data, long index) {
	Add(data);
	for(long j = GetLength() - 1; j > index; j --) {
		memcpy(array_ptr+j*datasize, array_ptr+(j-1)*datasize, datasize);
	}
	memcpy(array_ptr+index*datasize, data, datasize);
}

void CDynaArray::Remove(long index) {
	for(long j = index; j < GetLength() - 1; j ++) {
		memcpy(array_ptr+j*datasize, array_ptr+(j+1)*datasize, datasize);
	}
	Trim();
}

void CDynaArray::Trim() {
	size -= datasize;
	bzero((array_ptr + size), datasize);
}

long CDynaArray::Find(void* data, int (*cmpfunc)(void* param1,void* param2),long startpos)
{
	for(long i=startpos;i<GetLength();i++)
	{
		if((*cmpfunc)(Get(i),data)==0)
			return i;
	}
	return -1;
}

void CDynaArray::Sort(int sorttype, int (*cmpfunc)(void* param1,void* param2))
{
	///bubble sort
	u_int8_t* tmp= new u_int8_t[datasize];
	bzero(tmp, sizeof(u_int8_t)*datasize);
	for(long i=0;i<GetLength()-1;i++)
	{
		for(long j=i+1;j<GetLength();j++)
		{
			if(((*cmpfunc)(Get(i),Get(j))>0&&sorttype==SORT_ASCENDENT)
				||((*cmpfunc)(Get(i),Get(j))<0&&sorttype==SORT_DESCENDENT))
			{
				memcpy(tmp,Get(i),datasize);
				memcpy(Get(i),Get(j),datasize);
				memcpy(Get(j),tmp,datasize);
			}
		}
	}
	delete[] tmp;
}


ASStack::ASStack()
{
	top=0;
}

int ASStack::getdepth()
{
	return top-1;
}

ASELE* ASStack::pop()
{
	if(IsEmpty())
		return NULL;
	top--;
	return &stack[top];
}

void ASStack::push(ASELE* ele)
{
	if(top==AS_STACK_DEPTH-1)
	{
		printf("FALTAL ERROR: AS STACK OVER FLOW!!!\n");
		exit(1);
		getchar();
	}

	stack[top].index=ele->index;
	stack[top].array_idx=ele->array_idx;
	top++;
}

bool ASStack::IsEmpty()
{
	if(top==0)
		return true;
	else
		return false;
}
	
ASELE* ASStack::gettop()
{
	if(top==0)
		return NULL;
	else
		return &stack[top-1];
}

long CDynaArray::Partition(long low,long high,int (*cmpfunc)(void* param1,void* param2, void* data))
{
	u_int8_t *pivotkey = new u_int8_t[datasize];
	bzero(pivotkey, sizeof(u_int8_t)*datasize);
	memcpy(pivotkey,Get(low),datasize);
	while(low<high)
	{
		while(low<high&&(*cmpfunc)(Get(high),pivotkey, sortdata)>=0) high--;
		Set(Get(high),low);
		while(low<high&&(*cmpfunc)(Get(low), pivotkey, sortdata)<=0) low++;
		Set(Get(low),high);
	}
	Set(pivotkey,low);
	delete[] pivotkey;
	return low;
}

void CDynaArray::QSort(long low,long high,int (*cmpfunc)(void* param1,void* param2, void* data))
{
	ASStack asstack;
	ASELE asele, *asele_ptr,asele1,asele2;
	long pivotpos;

	asele.array_idx=low;
	asele.index=high;
	asstack.push(&asele);

	while(!asstack.IsEmpty())
	{
		asele_ptr=asstack.pop();
		asele1.array_idx=asele_ptr->array_idx;
		asele2.index=asele_ptr->index;

		pivotpos=Partition(asele_ptr->array_idx,asele_ptr->index,(*cmpfunc));
		asele1.index=pivotpos-1;
		asele2.array_idx=pivotpos+1;
		if(asele1.array_idx<asele1.index)
			asstack.push(&asele1);
		if(asele2.array_idx<asele2.index)
			asstack.push(&asele2);
	}

	/*if(low<high)
	{
		pivotloc=Partition(low,high,(*cmpfunc));
		QSort(low,pivotloc-1,(*cmpfunc));
		QSort(pivotloc+1,high,(*cmpfunc));
	}*/
}

void CDynaArray::QuickSort(int (*cmpfunc)(void* param1,void* param2,void*data), void* data)
{
	sortdata = data;
	QSort(0,GetLength()-1,(*cmpfunc));
}

} /* end of namespace bgplib */
