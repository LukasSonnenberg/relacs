SUBDIRS = src

attcs3310cfgdir = $(pkgdatadir)/configs/attcs3310
dist_attcs3310cfg_DATA = \
    relacs.cfg \
    relacsplugins.cfg \
    macros.cfg \
    calib0-0.cfg \
    calib0-1.cfg

EXTRA_DIST = doc/attcs3310.doc

pkglibdir = $(libdir)/relacs
pkgincludedir = $(includedir)/relacs
pkgdatadir=$(datadir)/relacs
docdir = $(datarootdir)/doc/relacs

PLUGINSET = attcs3310
DX_PROJECT = attcs3310
DX_CONFIG = ../../../doc/Doxyfile
DX_INSTALL_SUBDIR = /api/$(DX_PROJECT)
DX_DEPENDS = Makefile.am src/Makefile.am
DX_SETTINGS = PROJECT_NAME='"AttCS3310 Relacs Plugins"'
DX_SETTINGS += INPUT='include/relacs/attcs3310 src doc'
DX_SETTINGS += PREDEFINED='$(DOXYGEN_PREDEF)'
DX_SETTINGS += TAGFILES='../../../doc/relacsall.tag=.. ../nieseries/doc/nieseries.tag=../nieseries'

include $(top_srcdir)/doxygen.mk

if DX_COND_doc

include $(srcdir)/../../relacshelp.mk

.PHONY: doc help

doc: doxygen-doc
help: doc help-make

all-local: doxygen-runall help-run
html-local: doxygen-run
ps-local: doxygen-ps
pdf-local: doxygen-pdf
clean-local: doxygen-clean help-clean
install-data-local: doxygen-install help-install
uninstall-local: doxygen-uninstall help-uninstall

endif DX_COND_doc
