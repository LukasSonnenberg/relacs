noinst_PROGRAMS = \
    ao_nonblocking \
    ao_underrun \
    ao_trigger \
    ai_nonblocking \
    ai_maxrate \
    ai_multiboard \
    aitriggersao \
    aotriggersai \
    dio_write \
    checkao

noinst_HEADERS = examples.h

ao_nonblocking_LDFLAGS = $(COMEDI_LDFLAGS)
ao_nonblocking_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
ao_nonblocking_SOURCES = ao_nonblocking.c common.c

ao_underrun_LDFLAGS = $(COMEDI_LDFLAGS)
ao_underrun_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
ao_underrun_SOURCES = ao_underrun.c common.c

ao_trigger_LDFLAGS = $(COMEDI_LDFLAGS)
ao_trigger_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
ao_trigger_SOURCES = ao_trigger.c common.c

ai_nonblocking_LDFLAGS = $(COMEDI_LDFLAGS)
ai_nonblocking_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
ai_nonblocking_SOURCES = ai_nonblocking.c common.c

ai_maxrate_LDFLAGS = $(COMEDI_LDFLAGS)
ai_maxrate_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
ai_maxrate_SOURCES = ai_maxrate.c common.c

ai_multiboard_LDFLAGS = $(COMEDI_LDFLAGS)
ai_multiboard_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
ai_multiboard_SOURCES = ai_multiboard.c common.c

aitriggersao_LDFLAGS = $(COMEDI_LDFLAGS)
aitriggersao_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
aitriggersao_SOURCES = aitriggersao.c common.c

aotriggersai_LDFLAGS = $(COMEDI_LDFLAGS)
aotriggersai_LDADD = $(COMEDI_LIBS) $(MATH_LIBS)
aotriggersai_SOURCES = aotriggersai.c common.c

dio_write_CPPFLAGS = \
    -I$(top_srcdir)/shapes/include \
    -I$(top_srcdir)/daq/include \
    -I$(top_srcdir)/numerics/include \
    -I$(top_srcdir)/options/include \
    -I$(srcdir)/../include $(QT4_CPPFLAGS)
dio_write_LDFLAGS = $(COMEDI_LDFLAGS)
dio_write_LDADD = \
    $(COMEDI_LIBS) \
    $(top_builddir)/numerics/src/librelacsnumerics.la \
    $(top_builddir)/options/src/librelacsoptions.la \
    $(top_builddir)/daq/src/librelacsdaq.la \
    $(top_builddir)/shapes/src/librelacsshapes.la \
    $(top_builddir)/relacs/src/librelacs.la \
    $(GSL_LIBS) \
    ../src/libcomedistreaming.la
dio_write_SOURCES = dio_write.cc

checkao_CPPFLAGS = \
    -I$(top_srcdir)/shapes/include \
    -I$(top_srcdir)/daq/include \
    -I$(top_srcdir)/numerics/include \
    -I$(top_srcdir)/options/include \
    -I$(srcdir)/../include $(QT4_CPPFLAGS)
checkao_LDFLAGS = \
    $(QT3_LDFLAGS)
checkao_LDADD = \
    $(top_builddir)/numerics/src/librelacsnumerics.la \
    $(top_builddir)/options/src/librelacsoptions.la \
    $(top_builddir)/daq/src/librelacsdaq.la \
    $(top_builddir)/shapes/src/librelacsshapes.la \
    $(top_builddir)/relacs/src/librelacs.la \
    $(GSL_LIBS) \
    ../src/libcomedistreaming.la
checkao_SOURCES = checkao.cc
