SUBDIRS = src

patchclampprojectscfgdir = $(pkgdatadir)/configs/patchclampprojects
dist_patchclampprojectscfg_DATA = \
    relacs.cfg \
    relacsplugins.cfg \
    macros.cfg

EXTRA_DIST = \
    doc/patchclampprojects.doc \
    doc/thresholdlatenciesstimuli.png \
   doc/voltagereconstruction.png

DISTCLEANFILES = relacslocal relacs.log relacs.log.old

pkglibdir = $(libdir)/relacs
pkgincludedir = $(includedir)/relacs
pkgdatadir=$(datadir)/relacs
docdir = $(datarootdir)/doc/relacs

PLUGINSET = patchclampprojects
DX_PROJECT = patchclampprojects
DX_CONFIG = ../../doc/Doxyfile
DX_INSTALL_SUBDIR = /api/$(DX_PROJECT)
DX_DEPENDS = Makefile.am src/Makefile.am
DX_SETTINGS = PROJECT_NAME='"patchclampprojects Relacs Plugins"'
DX_SETTINGS += INPUT='include/relacs/patchclampprojects src doc'
DX_SETTINGS += PREDEFINED='$(DOXYGEN_PREDEF)'
DX_SETTINGS += TAGFILES='../../doc/relacsall.tag=..'

include $(top_srcdir)/doxygen.mk

if DX_COND_doc

include $(srcdir)/../relacshelp.mk

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
