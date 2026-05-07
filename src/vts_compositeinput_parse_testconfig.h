/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2026 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file vts_compositeinput_parse_testconfig.h
 *
 * Parse test configuration file for Composite Input L3 tests
 */

#ifndef __VTS_COMPOSITEINPUT_PARSE_TESTCONFIG_H__
#define __VTS_COMPOSITEINPUT_PARSE_TESTCONFIG_H__

#include <string>
#include <vector>

#include <ut.h>
#include <ut_log.h>
#include "ut_kvp_profile.h"
#include "ut_control_plane.h"

#include "com/rdk/hal/compositeinput/SignalStatus.h"
#include "com/rdk/hal/compositeinput/PortProperty.h"

using namespace std;
using namespace com::rdk::hal::compositeinput;

#define VTS_COMPOSITEINPUT_TEST_CONFIG_PATH "./test_configuration/vts_l3_CompositeInput.yaml"

typedef struct compositeInputConnectionConfig
{
    int32_t portId;
    bool connectionState;
    std::vector<bool> connectionSequence;
    bool expectSignalAfterConnect;
    uint32_t maxWaitTimeMs;
} compositeInputConnectionConfig_t;

typedef struct compositeInputSignalConfig
{
    int32_t portId;
    ::com::rdk::hal::compositeinput::SignalStatus expectedSignalStatus;
    uint32_t maxWaitTimeMs;
} compositeInputSignalConfig_t;

typedef struct compositeInputVideoModeEntry
{
    int32_t width;
    int32_t height;
    bool    interlaced;
    float   framerate;
} compositeInputVideoModeEntry_t;

typedef struct compositeInputVideoModeConfig
{
    int32_t portId;
    compositeInputVideoModeEntry_t videomode1;
    compositeInputVideoModeEntry_t videomode2;
    uint32_t maxWaitTimeMs;
} compositeInputVideoModeConfig_t;

typedef struct compositeInputPropertyTestItem
{
    std::string propertyName;
    std::vector<int64_t> values;
} compositeInputPropertyTestItem_t;

typedef struct compositeInputPropertyConfig
{
    int32_t portId;
    std::vector<compositeInputPropertyTestItem_t> testProperties;
    uint32_t maxWaitTimeMs;
} compositeInputPropertyConfig_t;

UT_status_t vts_compositeinput_parse_connectionTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputConnectionConfig_t>& configs);

UT_status_t vts_compositeinput_parse_signalTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputSignalConfig_t>& configs);

UT_status_t vts_compositeinput_parse_videoModeTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputVideoModeConfig_t>& configs);

UT_status_t vts_compositeinput_parse_propertyTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputPropertyConfig_t>& configs);

#endif // __VTS_COMPOSITEINPUT_PARSE_TESTCONFIG_H__
