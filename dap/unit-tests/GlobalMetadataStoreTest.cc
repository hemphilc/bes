// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#include "config.h"

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Array.h>
#include <Byte.h>
#include <ServerFunctionsList.h>
#include <ConstraintEvaluator.h>
#include <DAS.h>
#include <DDS.h>
#include <DDXParserSAX2.h>

#include <GetOpt.h>
#include <GNURegex.h>
#include <util.h>
#include <debug.h>

#include "BESError.h"
#include "TheBESKeys.h"
#include "BESDebug.h"

#include "GlobalMetadataStore.h"

#include "TestFunction.h"
#include "test_utils.h"
#include "test_config.h"

static bool debug = false;
static bool bes_debug = false;
static bool clean = true;

#undef DBG
#define DBG(x) do { if (debug) (x); } while(false);

#define DEBUG_KEY "metadata_store"

using namespace CppUnit;
using namespace std;
using namespace libdap;
using namespace bes;

// Move this into the class when we goto C++-11
const string d_mds_prefix = "mds"; // used when cleaning the cache, etc.
static const string c_mds_name = "/mds";


class GlobalMetadataStoreTest: public TestFixture {
private:
    DDS *test_dds;

    string d_mds_dir;
    GlobalMetadataStore *mds;

public:
    GlobalMetadataStoreTest() :
        test_dds(0), d_mds_dir(string(TEST_BUILD_DIR).append(c_mds_name)), mds(0)
    {
    }

    ~GlobalMetadataStoreTest()
    {
    }

    void setUp()
    {
        DBG(cerr << "setUp() - BEGIN" << endl);
        if (bes_debug) BESDebug::SetUp(string("cerr,") + DEBUG_KEY);
#if 0
        string cid; // This is an unused value-result parameter. jhrg 5/10/16
        test_dds = new DDS(&ttf);
        dp.intern((string) TEST_SRC_DIR + "/input-files/test.05.ddx", test_dds, cid);

        // for these tests, set the filename to the dataset_name. ...keeps the cache names short
        test_dds->filename(test_dds->get_dataset_name());

        DBG(cerr << "DDS Name: " << test_dds->get_dataset_name() << endl);

        if (clean) clean_cache_dir(d_cache);
#endif
        TheBESKeys::ConfigFile = (string) TEST_SRC_DIR + "/input-files/test.keys"; // intentionally empty file. jhrg 10/20/15

        DBG(cerr << "setUp() - END" << endl);
    }

    void tearDown()
    {
        DBG(cerr << "tearDown() - BEGIN" << endl);

        delete test_dds;

        if (clean) clean_cache_dir(d_mds_dir);

        DBG(cerr << "tearDown() - END" << endl);
    }

#if 0
    bool re_match(Regex &r, const string &s)
    {
        DBG(cerr << "s.length(): " << s.length() << endl);
        int pos = r.match(s.c_str(), s.length());
        DBG(cerr << "r.match(s): " << pos << endl);
        return pos > 0 && static_cast<unsigned>(pos) == s.length();
    }

    bool re_match_binary(Regex &r, const string &s)
    {
        DBG(cerr << "s.length(): " << s.length() << endl);
        int pos = r.match(s.c_str(), s.length());
        DBG(cerr << "r.match(s): " << pos << endl);
        return pos > 0;
    }
#endif

    // The call to get_instance should fail since the directory is named but does not exist
    // and cannot be made.
    void ctor_test_1()
    {
        DBG(cerr << "ctor_test_1() - BEGIN" << endl);

        try {
            mds = GlobalMetadataStore::get_instance(string(TEST_BUILD_DIR) + "/new", d_mds_prefix, 1000);
            DBG(cerr << "ctor_test_1() - retrieved GlobalMetadataStore instance: " << mds << endl);
            CPPUNIT_ASSERT(mds);
        }
        catch (BESError &e) {
            CPPUNIT_FAIL("Caught exception: " + e.get_message());
        }

        DBG(cerr << "ctor_test_1() - END" << endl);
    }

#if 0
    // The directory 'd_response_cache' should exist so is_available() should be
    // true.
    void ctor_test_2()
    {
        DBG(cerr << "ctor_test_2() - BEGIN" << endl);

        string cacheDir = d_cache;
        string prefix = "rc";
        long size = 1000;
        cache = GlobalMetadataStore::get_instance(cacheDir, prefix, size);
        DBG(cerr << "ctor_test_1() - retrieved GlobalMetadataStore instance: " << cache << endl);

        CPPUNIT_ASSERT(cache);

        DBG(cerr << "ctor_test_2() - END" << endl);
    }

    // Because setup() and teardown() clean out the cache directory, there should
    // never be a cached item; calling read_cached_dataset() should return a
    // valid DDS with data and store a copy in the cache.
    void cache_a_response()
    {
        DBG(cerr << "cache_a_response() - BEGIN" << endl);
        cache = GlobalMetadataStore::get_instance(d_cache, d_mds_prefix, 1000);

        DBG(cerr << "cache_a_response() - Retrieved GlobalMetadataStore object: " << cache << endl);

        string token;
        try {
            CPPUNIT_ASSERT(test_dds);

            DDS *result = cache->get_or_cache_dataset(test_dds, "test(\"foo\")");

            CPPUNIT_ASSERT(result);

            CPPUNIT_ASSERT(result->var("foo"));
            CPPUNIT_ASSERT(result->var("foo")->type() == dods_array_c);
        }
        catch (BESError &e) {
            CPPUNIT_FAIL(e.get_message());
        }

        DBG(cerr << "cache_a_response() - END" << endl);
    }

    // The first call reads values into the DDS, stores a copy in the cache and
    // returns the DDS. The second call reads the value from the cache.
    //
    // Use load_from_cache() (Private interface) to read the data.
    void cache_and_read_a_response()
    {
        DBG(cerr << "cache_and_read_a_response() - BEGIN" << endl);

        cache = GlobalMetadataStore::get_instance(d_cache, d_mds_prefix, 1000);
        try {
            const string constraint = "test(\"bar\")";

            // This code is here to load the DataDDX response into the cache if it is not
            // there already. If it is there, it reads it from the cache.
            DDS *result = cache->get_or_cache_dataset(test_dds, constraint);

            CPPUNIT_ASSERT(result);
            int var_count = result->var_end() - result->var_begin();
            CPPUNIT_ASSERT(var_count == 1);

            //DDS *result2 = cache->get_or_cache_dataset(test_dds, "test(\"bar\")");
            string resource_id = cache->get_resource_id(test_dds, constraint);
            string cache_file_name = cache->get_hash_basename(resource_id);

            DBG(
                cerr << "cache_and_read_a_response() - resource_id: " << resource_id << ", cache_file_name: "
                    << cache_file_name << endl);

            DDS *result2 = cache->load_from_cache(resource_id, cache_file_name);
            // Better not be null!
            CPPUNIT_ASSERT(result2);
            result2->filename("function_result_SimpleTypes");

            // There are nine variables in test.05.ddx
            var_count = result2->var_end() - result2->var_begin();
            DBG(cerr << "cache_and_read_a_response() - var_count: " << var_count << endl);
            CPPUNIT_ASSERT(var_count == 1);

            ostringstream oss;
            DDS::Vars_iter i = result2->var_begin();
            while (i != result2->var_end()) {
                DBG(cerr << "Variable " << (*i)->name() << endl);
                // this will incrementally add the string rep of values to 'oss'
                (*i)->print_val(oss, "", false /*print declaration */);
                DBG(cerr << "Value " << oss.str() << endl);
                ++i;
            }

            CPPUNIT_ASSERT(oss.str().compare("{{0, 1, 2},{3, 4, 5},{6, 7, 8}}") == 0);
        }
        catch (Error &e) {
            CPPUNIT_FAIL(e.get_error_message());
        }

        DBG(cerr << "cache_and_read_a_response() - END" << endl);
    }

    // The first call reads values into the DDS, stores a copy in the cache and
    // returns the DDS. The second call reads the value from the cache.
    //
    // Use the public interface to read the data (cache_dataset()), but w/o a
    // constraint
    void cache_and_read_a_response2()
    {
        DBG(cerr << "cache_and_read_a_response() - BEGIN" << endl);

        cache = GlobalMetadataStore::get_instance(d_cache, d_mds_prefix, 1000);
        try {
            // This code is here to load the DataDDX response into the cache if it is not
            // there already. If it is there, it reads it from the cache.
            DDS *result = cache->get_or_cache_dataset(test_dds, "test(\"bar\")");

            CPPUNIT_ASSERT(result);
            int var_count = result->var_end() - result->var_begin();
            CPPUNIT_ASSERT(var_count == 1);

            DDS *result2 = cache->get_or_cache_dataset(test_dds, "test(\"bar\")");
            // Better not be null!
            CPPUNIT_ASSERT(result2);
            result2->filename("function_result_SimpleTypes");

            // There are nine variables in test.05.ddx
            var_count = result2->var_end() - result2->var_begin();
            DBG(cerr << "cache_and_read_a_response() - var_count: " << var_count << endl);
            CPPUNIT_ASSERT(var_count == 1);

            ostringstream oss;
            DDS::Vars_iter i = result2->var_begin();
            while (i != result2->var_end()) {
                DBG(cerr << "Variable " << (*i)->name() << endl);
                // this will incrementally add the string rep of values to 'oss'
                (*i)->print_val(oss, "", false /*print declaration */);
                DBG(cerr << "Value " << oss.str() << endl);
                ++i;
            }

            CPPUNIT_ASSERT(oss.str().compare("{{0, 1, 2},{3, 4, 5},{6, 7, 8}}") == 0);
        }
        catch (Error &e) {
            CPPUNIT_FAIL(e.get_error_message());
        }

        DBG(cerr << "cache_and_read_a_response() - END" << endl);
    }
#endif

    CPPUNIT_TEST_SUITE( GlobalMetadataStoreTest );

    CPPUNIT_TEST(ctor_test_1);
#if 0
    CPPUNIT_TEST(ctor_test_2);
    CPPUNIT_TEST(cache_a_response);
    CPPUNIT_TEST(cache_and_read_a_response);
    CPPUNIT_TEST(cache_and_read_a_response2);
#endif
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GlobalMetadataStoreTest);

int main(int argc, char*argv[])
{

    GetOpt getopt(argc, argv, "dbkh");
    int option_char;
    while ((option_char = getopt()) != -1)
        switch (option_char) {
        case 'd':
            debug = 1;  // debug is a static global
            break;
        case 'b':
            bes_debug = true;  // bes_debug is a static global
            cerr << "##### BES DEBUG is ON" << endl;
            break;
        case 'k':   // -k turns off cleaning the metadata store dir
            clean = false;
            break;
        case 'h': {     // help - show test names
            cerr << "Usage: GlobalMetadataStoreTest has the following tests:" << endl;
            const std::vector<Test*> &tests = GlobalMetadataStoreTest::suite()->getTests();
            unsigned int prefix_len = GlobalMetadataStoreTest::suite()->getName().append("::").length();
            for (std::vector<Test*>::const_iterator i = tests.begin(), e = tests.end(); i != e; ++i) {
                cerr << (*i)->getName().replace(0, prefix_len, "") << endl;
            }
            break;
        }
        default:
            break;
        }

    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    bool wasSuccessful = true;
    string test = "";
    int i = getopt.optind;
    if (i == argc) {
        // run them all
        wasSuccessful = runner.run("");
    }
    else {
        while (i < argc) {
            if (debug) cerr << "Running " << argv[i] << endl;
            test = GlobalMetadataStoreTest::suite()->getName().append("::").append(argv[i]);
            wasSuccessful = wasSuccessful && runner.run(test);

            ++i;
        }
    }

    return wasSuccessful ? 0 : 1;
}
