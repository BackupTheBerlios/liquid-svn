SUBDIRS = config

AM_CPPFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_srcdir)/kdefx $(all_includes)
noinst_HEADERS = liquid.h embeddata.h
kde_style_LTLIBRARIES = liquidplus.la
liquidplus_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module -lkdecore
liquidplus_la_LIBADD = -lkdefx 
liquidplus_la_SOURCES = liquid.cpp polish.cpp utils.cpp
liquidplus_la_METASOURCES = AUTO

lnkdir = $(kde_datadir)/kstyle/themes
lnk_DATA = liquidplus.themerc
EXTRA_DIST = $(lnk_DATA)



