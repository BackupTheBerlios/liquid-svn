AUTOMAKE_OPTIONS = foreign

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes)

noinst_HEADERS = liquidconfig.h

kde_module_LTLIBRARIES = kwin_liquidplus_config.la
kwin_liquidplus_config_la_SOURCES = liquidconfig.cpp configdialog.ui
kwin_liquidplus_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin_liquidplus_config_la_LIBADD = $(LIB_KDEUI)
kwin_liquidplus_config_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin_liquid_configplus_la_METASOURCES)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kwin_config.pot
