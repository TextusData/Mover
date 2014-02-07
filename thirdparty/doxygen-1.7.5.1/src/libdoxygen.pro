#
# This file was generated from libdoxygen.pro.in on Fri Aug 19 14:12:47 CEST 2011
#

#
# $Id: libdoxygen.pro.in,v 1.1 2001/03/19 19:27:41 root Exp $
#
# Copyright (C) 1997-2011 by Dimitri van Heesch.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation under the terms of the GNU General Public License is hereby 
# granted. No representations are made about the suitability of this software 
# for any purpose. It is provided "as is" without express or implied warranty.
# See the GNU General Public License for more details.
#
# Documents produced by Doxygen are derivative works derived from the
# input used in their production; they are not affected by this license.
#
# TMake project file for doxygen

TEMPLATE     =	libdoxygen.t
CONFIG       =	console warn_on staticlib debug
HEADERS      =	bufstr.h \
		cite.h \
                classdef.h \
                classlist.h \
                cmdmapper.h \
                code.h \
		commentcnv.h \
		commentscan.h \
                compound_xsd.h \
                config.h \
		constexp.h \
		cppvalue.h \
		debug.h \
                declinfo.h \
                defargs.h \
                defgen.h \
                define.h \
                definition.h \
                diagram.h \
                dirdef.h \
                docparser.h \
		docsets.h \
                doctokenizer.h \
                docvisitor.h \
		dot.h \
		doxygen.h \
		doxygen_bst.h \
		doxygen_css.h \
		eclipsehelp.h \
		entry.h \
		example.h \
		filedef.h \
		filename.h \
		footer_html.h \
		formula.h \
		ftextstream.h \
		ftvhelp.h \
		groupdef.h \
		header_html.h \
		htags.h \
		htmlattrib.h \
                htmldocvisitor.h \
		htmlgen.h \
		htmlhelp.h \
		indexlog.h \
		image.h \
		index.h \
                index_xsd.h \
		instdox.h \
		jquery_js.h \
		jquery_ui_js.h \
		sizzle_js.h \
		svgpan_js.h \
		language.h \
                latexdocvisitor.h \
		latexgen.h \
                layout.h \
                layout_default.h \
                lockingptr.h \
		logos.h \
		mandocvisitor.h \
		mangen.h \
                marshal.h \
		memberdef.h \
		membergroup.h \
		memberlist.h \
		membername.h \
		message.h \
                msc.h \
		namespacedef.h \
		navtree_css.h \
		navtree_js.h \
                objcache.h \
		outputgen.h \
		outputlist.h \
		pagedef.h \
		perlmodgen.h \
		lodepng.h \
		pre.h \
                printdocvisitor.h \
                pycode.h \
                pyscanner.h \
                fortrancode.h \
                fortranscanner.h \
                dbusxmlscanner.h \
		qhp.h \
		qhpxmlwriter.h \
		qtbc.h \
		reflist.h \
		resize_js.h \
                rtfdocvisitor.h \
		rtfgen.h \
		rtfstyle.h \
		scanner.h \
		searchindex.h \
		search_css.h \
		search_js.h \
		search_functions_php.h \
		search_opensearch_php.h \
		section.h \
		sortdict.h \
                store.h \
		tagreader.h \
		tclscanner.h \
                textdocvisitor.h \
		translator.h \
		translator_adapter.h \
		translator_am.h \
		translator_br.h \
		translator_ca.h \
		translator_cn.h \
		translator_cz.h \
		translator_de.h \
		translator_dk.h \
		translator_en.h \
		translator_es.h \
		translator_fi.h \
		translator_fr.h \
		translator_gr.h \
		translator_hr.h \
		translator_hu.h \
		translator_id.h \
		translator_it.h \
		translator_je.h \
		translator_jp.h \
		translator_ke.h \
		translator_kr.h \
		translator_nl.h \
		translator_no.h \
		translator_mk.h \
		translator_pl.h \
		translator_pt.h \
		translator_ro.h \
		translator_ru.h \
		translator_se.h \
		translator_si.h \
		translator_sk.h \
		translator_sr.h \
		translator_tw.h \
		translator_ua.h \
		translator_vi.h \
		translator_za.h \
		unistd.h \
		util.h \
		version.h \
		vhdlcode.h \
		vhdldocgen.h \
		vhdlscanner.h \
		xmldocvisitor.h \
                xmlgen.h

SOURCES      =	ce_lex.cpp \
                ce_parse.cpp \
		cite.cpp \
		classdef.cpp \
		classlist.cpp \
                cmdmapper.cpp \
		code.cpp \
		commentcnv.cpp \
		commentscan.cpp \
		cppvalue.cpp \
                dbusxmlscanner.cpp \
		debug.cpp \
                defgen.cpp \
		declinfo.cpp \
		defargs.cpp \
		define.cpp \
		definition.cpp \
		diagram.cpp \
                dirdef.cpp \
                docparser.cpp \
		docsets.cpp \
                doctokenizer.cpp \
		dot.cpp \
		doxygen.cpp \
		eclipsehelp.cpp \
		entry.cpp \
		filedef.cpp \
		filename.cpp \
		formula.cpp \
		ftextstream.cpp \
		ftvhelp.cpp \
		fortrancode.cpp \
		fortranscanner.cpp \    
		groupdef.cpp \
		htags.cpp \
                htmldocvisitor.cpp \
		htmlgen.cpp \
		htmlhelp.cpp \
		indexlog.cpp \
		image.cpp \
		index.cpp \
		instdox.cpp \
		language.cpp \
                latexdocvisitor.cpp \
		latexgen.cpp \
                layout.cpp \
		lodepng.cpp \
		logos.cpp \
		mandocvisitor.cpp \
		mangen.cpp \
                marshal.cpp \
		memberdef.cpp \
		membergroup.cpp \
		memberlist.cpp \
		membername.cpp \
		message.cpp \
                msc.cpp \
		namespacedef.cpp \
                objcache.cpp \
		outputgen.cpp \
		outputlist.cpp \
		pagedef.cpp \
		perlmodgen.cpp \
		pre.cpp \
		pycode.cpp \
		pyscanner.cpp \    
		qhp.cpp \
		qhpxmlwriter.cpp \
		reflist.cpp \
                rtfdocvisitor.cpp \
		rtfgen.cpp \
		rtfstyle.cpp \
		scanner.cpp \
		searchindex.cpp \
                store.cpp \
		tagreader.cpp \
                tclscanner.cpp \
                textdocvisitor.cpp \
		translator.cpp \
		util.cpp \
		version.cpp \
		vhdlcode.cpp \
		vhdldocgen.cpp \
		vhdlscanner.cpp \
		xmldocvisitor.cpp \
                xmlgen.cpp

win32:TMAKE_CXXFLAGS       += -DQT_NODLL 
win32-msvc:TMAKE_CXXFLAGS  += -Zm200
win32-g++:TMAKE_CXXFLAGS   += -fno-exceptions -fno-rtti
linux-g++:TMAKE_CXXFLAGS    += -fno-exceptions -fno-rtti
TMAKE_CXXFLAGS += -DYY_TYPEDEF_YY_SIZE_T -Dyy_size_t=int
INCLUDEPATH                += ../qtools
#INCLUDEPATH                += ../libpng
INCLUDEPATH                += ../libmd5
win32:INCLUDEPATH          += .
#win32-g++:INCLUDEPATH      = ../qtools /usr/include/libpng12 ../libmd5
win32-g++:INCLUDEPATH      = ../qtools ../libmd5
DESTDIR                    =  ../lib
TARGET                     =  doxygen
OBJECTS_DIR                =  ../objects

TMAKE_MOC = /usr/bin/moc
