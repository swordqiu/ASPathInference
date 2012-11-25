ASPathInference
===============

Introduction
------------------

This is a tool for inferring the Internet AS level paths from any source AS to any destination prefixes. Please refer to http://rio.ecs.umass.edu/~jqiu/aspath_tech.pdf for further information.




Software requirement
------------------

This tool is running on linux or other unix like systems. The following softwares are required:

 * gcc
 * python 2.4 and the development library
 * perl and LWP::Simple library



Installation
------------------

This software is a combination of binary executive codes and scripts. Sub-directory src contains c++ codes that facilitate storing and retrieving AS paths in known BGP routing tables; Sub-directory script contains scripts for path inferences service and automatic process. To use the software, please follow the three steps:

1) compile binary codes. Issue 

   # make 

  to compile all necessary binary codes.

2) collect and process data and start path inference service. Issue

   # make run
 
   to start the scripts. The script first automatically downloads the most recent BGP tables from routeviews and RIPE RIS data repositories into subdirectory "tables", and store the AS paths in a easily accessible way. Then the scripts infer AS relationships and other information. Before the information is completely inferred, the results are stored in a temporary sub-directory "tmp". After the completion of the inference process, sub-directory "tmp" will be renamed as "data". The process could take hours. Finally, the inference service script "pathInferenceServer.py" will start on the information stored in sub-directory "data".

3) AS path inference.

   The inference server will listen on TCP port 61002 based on http protocol. The service can be queried with the library routines specified in aspathinfer.pl or aspathinfer.py. Sub-directory "example" contains two simple query examples.

   If you find the path information is outdated, please repeat step 2) to retrieve the most up-to-date tables.




Possible compiling issues
------------------

In src/PYGetSurePath.cpp, the #include header file Python.h is pointing to $INCLUDE/python2.4/Python.h. On your system, if the header file is in other position, please change the #include directory accordingly.




License
------------------

ASPathInference is released under the GNU General Public License. A copy of the license is included in the distribution. It is also available from GNU. 


