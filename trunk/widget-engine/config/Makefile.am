INCLUDES = $(all_includes)

noinst_HEADERS = liquidconf.h previewdata.h
kde_module_LTLIBRARIES = kstyle_liquidplus_config.la
kstyle_liquidplus_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_liquidplus_config_la_LIBADD  = $(LIB_KDEUI)
kstyle_liquidplus_config_la_SOURCES = liquidconf.cpp
kstyle_liquidplus_config_la_METASOURCES = AUTO


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_liquid_config.pot
 
