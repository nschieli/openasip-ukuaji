pkglib_LTLIBRARIES = LowerIntrinsics.la LinkBitcode.la MachineDCE.la \
InnerLoopFinder.la

LowerIntrinsics_la_SOURCES = LowerIntrinsics.cc 
# Static linking here is needed in FreeBSD and possibly other
# targets were --enable-shared does not compile llvm. In that
# case the TCE script is used to generate the LLVM dynamic library.
# This means that opt or llc do not have a reference to that
# dynamic library thus cannot find the symbols needed when the
# LowerIntrinsics.so is loaded in tcecc.
LowerIntrinsics_la_LDFLAGS = -module ${LLVM_LDFLAGS}

LinkBitcode_la_SOURCES = LinkBitcode.cc 
LinkBitcode_la_LDFLAGS = -module

MachineDCE_la_SOURCES = MachineDCE.cc MachineDCE.hh
MachineDCE_la_LDFLAGS = -module

InnerLoopFinder_la_SOURCES = InnerLoopFinder.cc
InnerLoopFinder_la_LDFLAGS = -module

TOOLS_DIR = ${SRC_ROOT}/tools

PROJECT_ROOT = $(top_srcdir)
SRC_ROOT = ${PROJECT_ROOT}/src

AM_CPPFLAGS = ${LLVM_CPPFLAGS} -I${TOOLS_DIR}

all-local: ${pkglib_LTLIBRARIES}
# Create links to the .so to the current directory just to make 
# running ProGe from the source tree to find them.
	@@(for soname in $(shell ls -1 ${srcdir}/.libs/*.so); do \
		ln -sf $$soname ${srcdir}; \
	done)

clean-local:
	rm -f ${srcdir}/*.so

## headers start
LowerIntrinsics_la_SOURCES += \
	MachineDCE.hh 
## headers end
