# $Id: Makefile,v 0.0.1 CBB $

#
# Author:  Zengchao
# Contact: zengchao@cqu.edu.cn
# Licence: Belong to the author 
# Date:    2018.01.02
#

CBB_BASE:=$(shell if [ -n "$$CBB_BASE" ] ; then\
	echo $$CBB_BASE;\
	else\
	cd $(dir $(lastword $(MAKEFILE_LIST))); while /usr/bin/test ! -e RELEASE ; do \
	dir=`cd ../;pwd`;			\
	if [ "$$dir" = "/" ] ; then		\
	   echo Cannot find CBB_BASE in $(lastword $(MAKEFILE_LIST)) 1>&2;		\
	   exit 1;				\
	fi ;					\
	cd $$dir;				\
	done ;					\
	pwd;					\
	fi)


ifeq ($(CBB_BASE),)
$(error Please run this in a tree)
endif

export CBB_BASE

ifeq ($(MAKELEVEL),0)
TOPMAKE=1
endif

include $(CBB_BASE)/make/Make.generic
include $(CBB_BASE)/make/Make.local

subdirs += core
subdirs += appl

include $(CBB_BASE)/make/Make.lib

include $(CBB_BASE)/make/Make.subdirs



