AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = liquidplus.desktop

noinst_HEADERS = liquidclient.h bitmapdata.h

kde_module_LTLIBRARIES = kwin3_liquidplus.la
kwin3_liquidplus_la_SOURCES = liquidclient.cpp
kwin3_liquidplus_la_LIBADD = $(kde_libraries)/libkdecorations.la
kwin3_liquidplus_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkdecore -module
kwin3_liquidplus_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin3_liquidplus_la_METASOURCES)

messages: $(XGETTEXT) *.cpp *.h -o $(podir)/liquid.pot 
