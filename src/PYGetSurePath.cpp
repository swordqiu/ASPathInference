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




/**
 * GetSurePath.cpp
 *
 * Author: Jian Qiu (jqiu@ecs.umass.edu)
 *
 * Date: 5/16/2005
 **/

#include <python2.6/Python.h>
#include "StdAfx.h"
#include "VMEMstd.h"
#include "VMEM.h"
#include "Address.h"
#include "MyUtil.h"
#include "SimpleRoutingTable2.h"

using namespace bgplib;

static PyObject * bgp_getSurePath(PyObject *self, PyObject *args)
{
	const char *path;
	const char *asn_str;
	u_int32_t asn;
	const char *pref;
	char temp[4096];
	bzero(temp, sizeof(temp));

	if (!PyArg_ParseTuple(args, "sss", &path, &asn_str, &pref)) {
		return NULL;
	}
	asn = asn_a2n(asn_str);
	CPrefix prefix((char*)pref);
	//printf("%s:%d:%s\n", path, asn, prefix.toString());
	CSimpleRoutingTable crt((char*)path, asn, true);
	if(crt.GetAllPathStr(&prefix, 0, temp)) {
		//printf("return: %s", temp);
	}else {
		//printf("fail to get path\n");
	}
	crt.Release();
	return Py_BuildValue("s", temp);
}

static PyMethodDef BGPMethods[] = {
	{"getSurePath",  bgp_getSurePath, METH_VARARGS, "Get sure path to a prefix"},
	{NULL, NULL, 0, NULL}       /* Sentinel */
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initbgp(void)
{
	(void) Py_InitModule("bgp", BGPMethods);
}

