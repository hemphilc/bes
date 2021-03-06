
# Tests

AUTOMAKE_OPTIONS = foreign

# TODO set these with configure. jhrg 11/26/19
OPENSSL_LIBS=-lcrypto
OPENSSL_INC=-I/usr/local/opt/openssl/include
OPENSSL_LDFLAGS=-L/usr/local/opt/openssl/lib

AM_CPPFLAGS = $(H5_CPPFLAGS) -I$(top_srcdir)/modules -I$(top_srcdir)/modules/dmrpp_module \
-I$(top_srcdir)/dispatch -I$(top_srcdir)/dap $(DAP_CFLAGS) $(OPENSSL_INC)

# Added -lz for ubuntu
LIBADD = $(BES_DISPATCH_LIB) $(BES_DAP_LIB) $(BES_EXTRA_LIBS) \
$(H5_LDFLAGS) $(H5_LIBS) $(DAP_SERVER_LIBS) $(DAP_CLIENT_LIBS) \
$(OPENSSL_LDFLAGS) $(OPENSSL_LIBS) $(XML2_LIBS) -lz


if CPPUNIT
AM_CPPFLAGS += $(CPPUNIT_CFLAGS) 
LIBADD += $(CPPUNIT_LIBS)
endif

if USE_VALGRIND
TESTS_ENVIRONMENT=valgrind --quiet --trace-children=yes --error-exitcode=1  --dsymutil=yes --leak-check=yes
endif

# These are not used by automake but are often useful for certain types of
# debugging. Set CXXFLAGS to this in the nightly build using export ...
CXXFLAGS_DEBUG = -g3 -O0  -Wall -Wcast-align
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

# This determines what gets built by make check
check_PROGRAMS = $(UNIT_TESTS)

# This determines what gets run by 'make check.'
TESTS = $(UNIT_TESTS)

CREDS_CONF = $(builddir)/credentials.conf
CREDS_CONF_IN = $(srcdir)/input-files/credentials.conf.in

noinst_HEADERS = test_config.h

noinst_DATA = bes.conf $(CREDS_CONF) input-files/weak.conf

EXTRA_DIST = bes.conf.in test_config.h.in baselines input-files awsv4

CLEANFILES = *.gcda *.gcno *.strm *.file test_config.h tmp.txt

DISTCLEANFILES = bes.conf bes.log $(CREDS_CONF)

BUILT_SOURCES = test_config.h

BES_CONF_IN = bes.conf.in

test_config.h: $(srcdir)/test_config.h.in Makefile
	@mod_abs_srcdir=`echo ${abs_srcdir} | sed 's%\(.*\)/\(.[^/]*\)/[.][.]%\1%g'`; \
	mod_abs_top_srcdir=`echo ${abs_top_srcdir} | sed 's%\(.*\)/\(.[^/]*\)/[.][.]%\1%g'`; \
	mod_abs_builddir=`echo ${abs_builddir} | sed 's%\(.*\)/\(.[^/]*\)/[.][.]%\1%g'`; \
	sed -e "s%[@]abs_srcdir[@]%$${mod_abs_srcdir}%" \
	    -e "s%[@]abs_top_srcdir[@]%$${mod_abs_top_srcdir}%" \
	    -e "s%[@]abs_builddir[@]%$${mod_abs_builddir}%" $< > test_config.h

# Build the bes.conf used for testing so that the value substituted for
# @abs_top_srcdir@ does not contain '../'. This happens when using 
# configure's value for the parameter when running the distcheck target.
bes.conf: $(srcdir)/$(BES_CONF_IN) $(top_srcdir)/configure.ac
	@clean_abs_top_srcdir=`echo ${abs_top_srcdir} | sed 's/\(.*\)\/\(.[^\/]*\)\/\.\./\1/g'`; \
	sed -e "s%[@]abs_top_srcdir[@]%$$clean_abs_top_srcdir%" \
	    -e "s%[@]abs_top_builddir[@]%${abs_top_builddir}%" $< > bes.conf

$(CREDS_CONF):
	cp $(CREDS_CONF_IN) $(CREDS_CONF)
	chmod 600 $(CREDS_CONF)

############################################################################
# Unit Tests
#

if CPPUNIT
UNIT_TESTS = ChunkTest DmrppParserTest DmrppCommonTest DmrppMetadataStoreTest CredentialsManagerTest awsv4_test
else
UNIT_TESTS =

check-local:
	@echo ""
	@echo "**********************************************************"
	@echo "You must have cppunit 1.12.x or greater installed to run *"
	@echo "check target in unit-tests directory                     *"
	@echo "**********************************************************"
	@echo ""
endif

clean-local:
	-rm -rf mds mds_ledger.txt

OBJS = ../DMRpp.o ../DmrppCommon.o ../Chunk.o ../CurlHandlePool.o	\
../DmrppByte.o ../DmrppArray.o ../DmrppFloat32.o ../DmrppFloat64.o	\
../DmrppInt16.o ../DmrppInt32.o ../DmrppInt64.o ../DmrppInt8.o		\
../DmrppUInt16.o ../DmrppUInt32.o ../DmrppUInt64.o ../DmrppStr.o	\
../DmrppStructure.o ../DmrppUrl.o ../DmrppD4Enum.o ../DmrppD4Group.o	\
../DmrppD4Opaque.o ../DmrppD4Sequence.o ../DmrppTypeFactory.o		\
../DmrppParserSax2.o ../DmrppMetadataStore.o ../DmrppRequestHandler.o \
../CredentialsManager.o ../awsv4.o ../url_parser.o


ChunkTest_SOURCES = ChunkTest.cc
ChunkTest_LDADD = $(OBJS) $(LIBADD)

CredentialsManagerTest_SOURCES = CredentialsManagerTest.cc
CredentialsManagerTest_LDADD = $(OBJS) $(LIBADD)

awsv4_test_SOURCES = awsv4_test.cc
awsv4_test_LDADD = $(OBJS) $(LIBADD)

DmrppParserTest_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/modules/hdf5_handler
DmrppParserTest_SOURCES = DmrppParserTest.cc
DmrppParserTest_LDADD = $(OBJS) $(LIBADD) 

DmrppCommonTest_SOURCES = DmrppCommonTest.cc $(top_srcdir)/modules/read_test_baseline.cc
DmrppCommonTest_LDADD = $(OBJS) $(LIBADD)

DmrppMetadataStoreTest_SOURCES = DmrppMetadataStoreTest.cc $(top_srcdir)/modules/read_test_baseline.cc
DmrppMetadataStoreTest_LDADD = $(OBJS) $(LIBADD)
