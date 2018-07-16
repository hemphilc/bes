// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of the BES component of the Hyrax Data Server.

// Copyright (c) 2018 OPeNDAP, Inc.
// Author: Nathan Potter <ndp@opendap.org>
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

#include <memory>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>
#include <cstring>
#include <iostream>

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <curl/curl.h>

#include <XMLWriter.h>
#include <GetOpt.h>
#include <util.h>
#include <debug.h>

#include <BESError.h>
#include <BESDebug.h>
#include <BESUtil.h>
#include <TheBESKeys.h>
#include "test_config.h"

#include "RemoteHttpResource.h"
#include "CmrApi.h"
#include "CmrError.h"

#define MODULE "cmr"

using namespace libdap;
using namespace rapidjson;

static bool debug = false;
static bool bes_debug = false;

#undef DBG
#define DBG(x) do { if (debug) x; } while(false)

namespace cmr {

class CmrTest: public CppUnit::TestFixture {
private:

    // char curl_error_buf[CURL_ERROR_SIZE];

    void show_file(string filename)
    {
        ifstream t(filename.c_str());

        if (t.is_open()) {
            string file_content((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
            t.close();
            cout << endl << "##################################################################" << endl;
            cout << "file: " << filename << endl;
            cout << ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . " << endl;
            cout << file_content << endl;
            cout << ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . " << endl;
        }
    }

public:
    // Called once before everything gets tested
    CmrTest()
    {
    }

    // Called at the end of the test
    ~CmrTest()
    {
    }

    // Called before each test
    void setUp()
    {
        string bes_conf = BESUtil::assemblePath(TEST_SRC_DIR,"CmrTest.ini");
        TheBESKeys::ConfigFile = bes_conf;

        if (bes_debug) BESDebug::SetUp("cerr,cmr");

        if (bes_debug) show_file(bes_conf);
    }

    // Called after each test
    void tearDown()
    {
    }


    void get_years_test() {
        string prolog = string(__func__) + "() - ";

        string collection_name = "C179003030-ORNL_DAAC";
        string expected[] = { string("1984"), string("1985"), string("1986"),
                string("1987"), string("1988") };
        unsigned long  expected_size = 5;
        vector<string> years;
        try {
            CmrApi cmr;
            cmr.get_years(collection_name, years);
            BESDEBUG(MODULE, prolog << "Checking expected size ("<< expected_size << ") vs received size ( " << years.size() << ")" << endl);

            CPPUNIT_ASSERT(expected_size == years.size());

            stringstream msg;
            msg << prolog << "The collection '" << collection_name << "' spans "
                    << years.size() << " years: ";
            for (size_t i = 0; i < years.size(); i++) {
                if (i > 0)
                    msg << ", ";
                msg << years[i];
            }
            BESDEBUG(MODULE, msg.str() << endl);

            for (size_t i = 0; i < years.size(); i++) {
                msg.str(std::string());
                msg << prolog << "Checking:  expected: " << expected[i]
                        << " received: " << years[i];
                BESDEBUG(MODULE, msg.str() << endl);
                CPPUNIT_ASSERT(expected[i] == years[i]);
            }

        }
        catch (BESError &be) {
            string msg = "Caught BESError! Message: " + be.get_message();
            cerr << endl << msg << endl;
            CPPUNIT_ASSERT(!"Caught BESError");
        }

    }

    void get_months_test() {
        string prolog = string(__func__) + "() - ";

        string collection_name = "C179003030-ORNL_DAAC";
        string expected[] = {
                string("01"),
                string("02"),
                string("03"),
                string("04"),
                string("05"),
                string("06"),
                string("07"),
                string("08"),
                string("09"),
                string("10"),
                string("11"),
                string("12") };
        unsigned long  expected_size = 12;
        vector<string> months;
        try {
            CmrApi cmr;

            string year ="1985";

            cmr.get_months(collection_name, year, months);
            BESDEBUG(MODULE, prolog << "Checking expected size ("<< expected_size << ") vs received size (" << months.size() << ")" << endl);
            CPPUNIT_ASSERT(expected_size == months.size());

            stringstream msg;
            msg << prolog << "In the year " << year << " the collection '" << collection_name << "' spans "
                    << months.size() << " months: ";
            for (size_t i = 0; i < months.size(); i++) {
                if (i > 0)
                    msg << ", ";
                msg << months[i];
            }
            BESDEBUG(MODULE, msg.str() << endl);

            for (size_t i = 0; i < months.size(); i++) {
                msg.str(std::string());
                msg << prolog << "Checking:  expected: " << expected[i]
                        << " received: " << months[i];
                BESDEBUG(MODULE, msg.str() << endl);
                CPPUNIT_ASSERT(expected[i] == months[i]);
            }

        }
        catch (BESError &be) {
            string msg = "Caught BESError! Message: " + be.get_message();
            cerr << endl << msg << endl;
            CPPUNIT_ASSERT(!"Caught BESError");
        }

    }

    void get_days_test() {
        string prolog = string(__func__) + "() - ";

        //string collection_name = "C179003030-ORNL_DAAC";
        string collection_name = "C1276812863-GES_DISC";
        string expected[] = {
                string("01"),string("02"),string("03"),string("04"),string("05"),string("06"),string("07"),string("08"),string("09"),string("10"),
                string("11"),string("12"),string("13"),string("14"),string("15"),string("16"),string("17"),string("18"),string("19"),string("20"),
                string("21"),string("22"),string("23"),string("24"),string("25"),string("26"),string("27"),string("28"),string("29"),string("30"),
                string("31")
        };
        unsigned long  expected_size = 31;
        vector<string> days;
        try {
            CmrApi cmr;

            string year ="1985";
            string month = "03";

            cmr.get_days(collection_name, year, month, days);
            BESDEBUG(MODULE, prolog << "Checking expected size ("<< expected_size << ") vs received size (" << days.size() << ")" << endl);
            CPPUNIT_ASSERT(expected_size == days.size());

            stringstream msg;
            msg << prolog << "In the year " << year << ", month " << month << " the collection '" << collection_name << "' spans "
                    << days.size() << " days: ";
            for (size_t i = 0; i < days.size(); i++) {
                if (i > 0)
                    msg << ", ";
                msg << days[i];
            }
            BESDEBUG(MODULE, msg.str() << endl);

            for (size_t i = 0; i < days.size(); i++) {
                msg.str(std::string());
                msg << prolog << "Checking:  expected: " << expected[i]
                        << " received: " << days[i];
                BESDEBUG(MODULE, msg.str() << endl);
                CPPUNIT_ASSERT(expected[i] == days[i]);
            }

        }
        catch (BESError &be) {
            string msg = "Caught BESError! Message: " + be.get_message();
            cerr << endl << msg << endl;
            CPPUNIT_ASSERT(!"Caught BESError");
        }

    }

    void get_granules_day_test() {
        string prolog = string(__func__) + "() - ";

        //string collection_name = "C179003030-ORNL_DAAC";
        string collection_name = "C1276812863-GES_DISC";

        string expected[] = {
                string("MERRA2_100.tavg1_2d_slv_Nx.19850313.nc4")
        };

        unsigned long  expected_size = 1;
        vector<string> granules;
        try {
            CmrApi cmr;

            string year ="1985";
            string month = "03";
            string day = "13";

            cmr.get_granule_ids(collection_name, year, month, day, granules);
            BESDEBUG(MODULE, prolog << "Checking expected size ("<< expected_size << ") vs received size (" << granules.size() << ")" << endl);
            CPPUNIT_ASSERT(expected_size == granules.size());

            stringstream msg;
            msg << prolog << "In the year " << year << ", month " << month <<  ", day " << day << " the collection '" << collection_name << "' contains "
                    << granules.size() << " granules: ";
            for (size_t i = 0; i < granules.size(); i++) {
                if (i > 0)
                    msg << ", ";
                msg << granules[i];
            }
            BESDEBUG(MODULE, msg.str() << endl);

            for (size_t i = 0; i < granules.size(); i++) {
                msg.str(std::string());
                msg << prolog << "Checking:  expected: " << expected[i]
                        << " received: " << granules[i];
                BESDEBUG(MODULE, msg.str() << endl);
                CPPUNIT_ASSERT(expected[i] == granules[i]);
            }

        }
        catch (BESError &be) {
            string msg = "Caught BESError! Message: " + be.get_message();
            cerr << endl << msg << endl;
            CPPUNIT_ASSERT(!"Caught BESError");
        }




    }
    void get_granules_month_test() {
        string prolog = string(__func__) + "() - ";

        //string collection_name = "C179003030-ORNL_DAAC";
        string collection_name = "C1276812863-GES_DISC";

        string expected[] = {
                string("MERRA2_100.tavg1_2d_slv_Nx.19850301.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850302.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850303.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850304.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850305.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850306.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850307.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850308.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850309.nc4"),
                string("MERRA2_100.tavg1_2d_slv_Nx.19850310.nc4")
        };

        unsigned long  expected_size = 10;
        vector<string> granules;
        try {
            CmrApi cmr;

            string year ="1985";
            string month = "03";

            cmr.get_granule_ids(collection_name, year, month, granules);
            BESDEBUG(MODULE, prolog << "Checking expected size ("<< expected_size << ") vs received size (" << granules.size() << ")" << endl);
            CPPUNIT_ASSERT(expected_size == granules.size());

            stringstream msg;
            msg << prolog << "In the year " << year << ", month " << month <<  " the collection '" << collection_name << "' contains "
                    << granules.size() << " granules: ";
            for (size_t i = 0; i < granules.size(); i++) {
                if (i > 0)
                    msg << ", ";
                msg << granules[i];
            }
            BESDEBUG(MODULE, msg.str() << endl);

            for (size_t i = 0; i < granules.size(); i++) {
                msg.str(std::string());
                msg << prolog << "Checking:  expected: " << expected[i]
                        << " received: " << granules[i];
                BESDEBUG(MODULE, msg.str() << endl);
                CPPUNIT_ASSERT(expected[i] == granules[i]);
            }

        }
        catch (BESError &be) {
            string msg = "Caught BESError! Message: " + be.get_message();
            cerr << endl << msg << endl;
            CPPUNIT_ASSERT(!"Caught BESError");
        }




    }

    CPPUNIT_TEST_SUITE( CmrTest );

    CPPUNIT_TEST(get_years_test);
    CPPUNIT_TEST(get_months_test);
    CPPUNIT_TEST(get_days_test);
    CPPUNIT_TEST(get_granules_day_test);
    CPPUNIT_TEST(get_granules_month_test);

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CmrTest);

} // namespace dmrpp

int main(int argc, char*argv[])
{
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    GetOpt getopt(argc, argv, "db");
    int option_char;
    while ((option_char = getopt()) != -1)
        switch (option_char) {
        case 'd':
            debug = true;  // debug is a static global
            break;
        case 'b':
            bes_debug = true;  // debug is a static global
            break;
        default:
            break;
        }

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
            test = cmr::CmrTest::suite()->getName().append("::").append(argv[i]);
            wasSuccessful = wasSuccessful && runner.run(test);
            ++i;
        }
    }

    return wasSuccessful ? 0 : 1;
}