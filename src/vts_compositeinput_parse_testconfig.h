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
#include "com/rdk/hal/compositeinput/ICompositeInputPort.h"
#include <com/rdk/hal/PropertyValue.h>
#include <utils/StrongPointer.h>

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
    std::vector<compositeInputVideoModeEntry_t> videomodes;
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

/**
 * Expected SIGNAL_STRENGTH (dBm, LONG) and SIGNAL_QUALITY (0..100, INTEGER)
 * read via getProperty() and validated against a single expected value per port.
 */
typedef struct compositeInputSignalDataConfig
{
    int32_t portId;
    int64_t signalStrength;  /**< expected SIGNAL_STRENGTH in dBm */
    int32_t signalQuality;   /**< expected SIGNAL_QUALITY (0..100) */
    uint32_t maxWaitTimeMs;
} compositeInputSignalDataConfig_t;

/**
 * Expected baseline metric values read via getProperty() for METRIC_* keys.
 * Validated after port open (before start) or after resetMetrics().
 */
typedef struct compositeInputMetricsConfig
{
    int32_t portId;
    int64_t metricSignalLockTime;   /**< expected METRIC_SIGNAL_LOCK_TIME */
    int64_t metricSignalDrops;      /**< expected METRIC_SIGNAL_DROPS */
    int64_t metricSignalLockCount;  /**< expected METRIC_SIGNAL_LOCK_COUNT */
    uint32_t maxWaitTimeMs;
} compositeInputMetricsConfig_t;

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

UT_status_t vts_compositeinput_parse_signalDataTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputSignalDataConfig_t>& configs);

UT_status_t vts_compositeinput_parse_metricsTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputMetricsConfig_t>& configs);

/**
 * @brief Validate SIGNAL_STRENGTH and SIGNAL_QUALITY via getProperty().
 *
 * Calls getProperty(SIGNAL_STRENGTH) and getProperty(SIGNAL_QUALITY) on the port
 * and asserts the returned values match cfg.signalStrength and cfg.signalQuality.
 *
 * @param port   The open composite input port to query.
 * @param cfg    Parsed signal data config containing expected values.
 */
void vts_compositeinput_validate_signalData(
    const android::sp<::com::rdk::hal::compositeinput::ICompositeInputPort>& port,
    const compositeInputSignalDataConfig_t& cfg);

/**
 * @brief Validate METRIC_SIGNAL_LOCK_TIME, METRIC_SIGNAL_DROPS, and METRIC_SIGNAL_LOCK_COUNT
 *        via getProperty() against the expected baseline values in cfg.
 *
 * @param port   The open composite input port to query.
 * @param cfg    Parsed metrics config containing expected baseline values.
 */
void vts_compositeinput_validate_metrics(
    const android::sp<::com::rdk::hal::compositeinput::ICompositeInputPort>& port,
    const compositeInputMetricsConfig_t& cfg);

#endif // __VTS_COMPOSITEINPUT_PARSE_TESTCONFIG_H__
