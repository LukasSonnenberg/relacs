HEADER_PATH = ../include/relacs/patchclampprojects
include ${top_srcdir}/moc4.mk

pluginlibdir = $(pkglibdir)/plugins

pluginlib_LTLIBRARIES = \
    libpatchclampprojectsthresholdlatencies.la \
    libpatchclampprojectsfindthreshold.la \
    libpatchclampprojectsvoltagereconstruction.la \
    libpatchclampprojectsfeedforwardnetwork.la \
    libpatchclampprojectssynapticresistance.la



libpatchclampprojectsthresholdlatencies_la_CPPFLAGS = \
    -I$(top_srcdir)/shapes/include \
    -I$(top_srcdir)/daq/include \
    -I$(top_srcdir)/numerics/include \
    -I$(top_srcdir)/options/include \
    -I$(top_srcdir)/datafile/include \
    -I$(top_srcdir)/plot/include \
    -I$(top_srcdir)/widgets/include \
    -I$(top_srcdir)/relacs/include \
    -I$(top_srcdir)/plugins/ephys/include \
    -I$(srcdir)/../include \
    $(QT_CPPFLAGS) $(NIX_CPPFLAGS)

libpatchclampprojectsthresholdlatencies_la_LDFLAGS = \
    -module -avoid-version \
    $(QT_LDFLAGS) $(NIX_LDFLAGS)

libpatchclampprojectsthresholdlatencies_la_LIBADD = \
    $(top_builddir)/plugins/ephys/src/libephystraces.la \
    $(top_builddir)/relacs/src/librelacs.la \
    $(top_builddir)/widgets/src/librelacswidgets.la \
    $(top_builddir)/plot/src/librelacsplot.la \
    $(top_builddir)/datafile/src/librelacsdatafile.la \
    $(top_builddir)/options/src/librelacsoptions.la \
    $(top_builddir)/daq/src/librelacsdaq.la \
    $(top_builddir)/shapes/src/librelacsshapes.la \
    $(top_builddir)/numerics/src/librelacsnumerics.la \
    $(QT_LIBS) $(NIX_LIBS)

$(libpatchclampprojectsthresholdlatencies_la_OBJECTS) : moc_thresholdlatencies.cc

libpatchclampprojectsthresholdlatencies_la_SOURCES = thresholdlatencies.cc

libpatchclampprojectsthresholdlatencies_la_includedir = $(pkgincludedir)/patchclampprojects

libpatchclampprojectsthresholdlatencies_la_include_HEADERS = $(HEADER_PATH)/thresholdlatencies.h



libpatchclampprojectsfindthreshold_la_CPPFLAGS = \
    -I$(top_srcdir)/shapes/include \
    -I$(top_srcdir)/daq/include \
    -I$(top_srcdir)/numerics/include \
    -I$(top_srcdir)/options/include \
    -I$(top_srcdir)/datafile/include \
    -I$(top_srcdir)/plot/include \
    -I$(top_srcdir)/widgets/include \
    -I$(top_srcdir)/relacs/include \
    -I$(top_srcdir)/plugins/ephys/include \
    -I$(srcdir)/../include \
    $(QT_CPPFLAGS) $(NIX_CPPFLAGS)

libpatchclampprojectsfindthreshold_la_LDFLAGS = \
    -module -avoid-version \
    $(QT_LDFLAGS) $(NIX_LDFLAGS)

libpatchclampprojectsfindthreshold_la_LIBADD = \
    $(top_builddir)/plugins/ephys/src/libephystraces.la \
    $(top_builddir)/relacs/src/librelacs.la \
    $(top_builddir)/widgets/src/librelacswidgets.la \
    $(top_builddir)/plot/src/librelacsplot.la \
    $(top_builddir)/datafile/src/librelacsdatafile.la \
    $(top_builddir)/options/src/librelacsoptions.la \
    $(top_builddir)/daq/src/librelacsdaq.la \
    $(top_builddir)/shapes/src/librelacsshapes.la \
    $(top_builddir)/numerics/src/librelacsnumerics.la \
    $(QT_LIBS) $(NIX_LIBS)

$(libpatchclampprojectsfindthreshold_la_OBJECTS) : moc_findthreshold.cc

libpatchclampprojectsfindthreshold_la_SOURCES = findthreshold.cc

libpatchclampprojectsfindthreshold_la_includedir = $(pkgincludedir)/patchclampprojects

libpatchclampprojectsfindthreshold_la_include_HEADERS = $(HEADER_PATH)/findthreshold.h



libpatchclampprojectsvoltagereconstruction_la_CPPFLAGS = \
    -I$(top_srcdir)/shapes/include \
    -I$(top_srcdir)/daq/include \
    -I$(top_srcdir)/numerics/include \
    -I$(top_srcdir)/options/include \
    -I$(top_srcdir)/datafile/include \
    -I$(top_srcdir)/plot/include \
    -I$(top_srcdir)/widgets/include \
    -I$(top_srcdir)/relacs/include \
    -I$(top_srcdir)/plugins/ephys/include \
    -I$(srcdir)/../include \
    $(QT_CPPFLAGS) $(NIX_CPPFLAGS)

libpatchclampprojectsvoltagereconstruction_la_LDFLAGS = \
    -module -avoid-version \
    $(QT_LDFLAGS) $(NIX_LDFLAGS)

libpatchclampprojectsvoltagereconstruction_la_LIBADD = \
    $(top_builddir)/plugins/ephys/src/libephystraces.la \
    $(top_builddir)/relacs/src/librelacs.la \
    $(top_builddir)/widgets/src/librelacswidgets.la \
    $(top_builddir)/plot/src/librelacsplot.la \
    $(top_builddir)/datafile/src/librelacsdatafile.la \
    $(top_builddir)/options/src/librelacsoptions.la \
    $(top_builddir)/daq/src/librelacsdaq.la \
    $(top_builddir)/shapes/src/librelacsshapes.la \
    $(top_builddir)/numerics/src/librelacsnumerics.la \
    $(QT_LIBS) $(NIX_LIBS)

$(libpatchclampprojectsvoltagereconstruction_la_OBJECTS) : moc_voltagereconstruction.cc

libpatchclampprojectsvoltagereconstruction_la_SOURCES = voltagereconstruction.cc

libpatchclampprojectsvoltagereconstruction_la_includedir = $(pkgincludedir)/patchclampprojects

libpatchclampprojectsvoltagereconstruction_la_include_HEADERS = $(HEADER_PATH)/voltagereconstruction.h



libpatchclampprojectsfeedforwardnetwork_la_CPPFLAGS = \
    -I$(top_srcdir)/shapes/include \
    -I$(top_srcdir)/daq/include \
    -I$(top_srcdir)/numerics/include \
    -I$(top_srcdir)/options/include \
    -I$(top_srcdir)/datafile/include \
    -I$(top_srcdir)/plot/include \
    -I$(top_srcdir)/widgets/include \
    -I$(top_srcdir)/relacs/include \
    -I$(top_srcdir)/plugins/ephys/include \
    -I$(srcdir)/../include \
    $(QT_CPPFLAGS) $(NIX_CPPFLAGS)

libpatchclampprojectsfeedforwardnetwork_la_LDFLAGS = \
    -module -avoid-version \
    $(QT_LDFLAGS) $(NIX_LDFLAGS)

libpatchclampprojectsfeedforwardnetwork_la_LIBADD = \
    $(top_builddir)/plugins/ephys/src/libephystraces.la \
    $(top_builddir)/relacs/src/librelacs.la \
    $(top_builddir)/widgets/src/librelacswidgets.la \
    $(top_builddir)/plot/src/librelacsplot.la \
    $(top_builddir)/datafile/src/librelacsdatafile.la \
    $(top_builddir)/options/src/librelacsoptions.la \
    $(top_builddir)/daq/src/librelacsdaq.la \
    $(top_builddir)/shapes/src/librelacsshapes.la \
    $(top_builddir)/numerics/src/librelacsnumerics.la \
    $(QT_LIBS) $(NIX_LIBS)

$(libpatchclampprojectsfeedforwardnetwork_la_OBJECTS) : moc_feedforwardnetwork.cc

libpatchclampprojectsfeedforwardnetwork_la_SOURCES = feedforwardnetwork.cc

libpatchclampprojectsfeedforwardnetwork_la_includedir = $(pkgincludedir)/patchclampprojects

libpatchclampprojectsfeedforwardnetwork_la_include_HEADERS = $(HEADER_PATH)/feedforwardnetwork.h



libpatchclampprojectssynapticresistance_la_CPPFLAGS = \
    -I$(top_srcdir)/shapes/include \
    -I$(top_srcdir)/daq/include \
    -I$(top_srcdir)/numerics/include \
    -I$(top_srcdir)/options/include \
    -I$(top_srcdir)/datafile/include \
    -I$(top_srcdir)/plot/include \
    -I$(top_srcdir)/widgets/include \
    -I$(top_srcdir)/relacs/include \
    -I$(top_srcdir)/plugins/ephys/include \
    -I$(srcdir)/../include \
    $(QT_CPPFLAGS) $(NIX_CPPFLAGS)

libpatchclampprojectssynapticresistance_la_LDFLAGS = \
    -module -avoid-version \
    $(QT_LDFLAGS) $(NIX_LDFLAGS)

libpatchclampprojectssynapticresistance_la_LIBADD = \
    $(top_builddir)/plugins/ephys/src/libephystraces.la \
    $(top_builddir)/relacs/src/librelacs.la \
    $(top_builddir)/widgets/src/librelacswidgets.la \
    $(top_builddir)/plot/src/librelacsplot.la \
    $(top_builddir)/datafile/src/librelacsdatafile.la \
    $(top_builddir)/options/src/librelacsoptions.la \
    $(top_builddir)/daq/src/librelacsdaq.la \
    $(top_builddir)/shapes/src/librelacsshapes.la \
    $(top_builddir)/numerics/src/librelacsnumerics.la \
    $(QT_LIBS) $(NIX_LIBS) $(GSL_LIBS)

$(libpatchclampprojectssynapticresistance_la_OBJECTS) : moc_synapticresistance.cc

libpatchclampprojectssynapticresistance_la_SOURCES = synapticresistance.cc

libpatchclampprojectssynapticresistance_la_includedir = $(pkgincludedir)/patchclampprojects

libpatchclampprojectssynapticresistance_la_include_HEADERS = $(HEADER_PATH)/synapticresistance.h



check_PROGRAMS = \
    linktest_libpatchclampprojectsthresholdlatencies_la \
    linktest_libpatchclampprojectsfindthreshold_la \
    linktest_libpatchclampprojectsvoltagereconstruction_la \
    linktest_libpatchclampprojectsfeedforwardnetwork_la \
    linktest_libpatchclampprojectssynapticresistance_la

linktest_libpatchclampprojectsthresholdlatencies_la_SOURCES = linktest.cc
linktest_libpatchclampprojectsthresholdlatencies_la_LDADD = libpatchclampprojectsthresholdlatencies.la

linktest_libpatchclampprojectsfindthreshold_la_SOURCES = linktest.cc
linktest_libpatchclampprojectsfindthreshold_la_LDADD = libpatchclampprojectsfindthreshold.la

linktest_libpatchclampprojectsvoltagereconstruction_la_SOURCES = linktest.cc
linktest_libpatchclampprojectsvoltagereconstruction_la_LDADD = libpatchclampprojectsvoltagereconstruction.la

linktest_libpatchclampprojectsfeedforwardnetwork_la_SOURCES = linktest.cc
linktest_libpatchclampprojectsfeedforwardnetwork_la_LDADD = libpatchclampprojectsfeedforwardnetwork.la

linktest_libpatchclampprojectssynapticresistance_la_SOURCES = linktest.cc
linktest_libpatchclampprojectssynapticresistance_la_LDADD = libpatchclampprojectssynapticresistance.la

TESTS = $(check_PROGRAMS)
