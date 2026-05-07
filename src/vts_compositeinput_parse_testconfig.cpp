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
 * @file vts_compositeinput_parse_testconfig.cpp
 *
 * Implementation of test configuration parsing for Composite Input L3 tests
 */

#include "vts_compositeinput_parse_testconfig.h"
#include "vts_compositeinput_helper_functions.h"

UT_status_t vts_compositeinput_parse_connectionTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputConnectionConfig_t>& configs)
{
    ut_kvp_instance_t *kvpInstance = ut_kvp_createInstance();
    if (!kvpInstance) {
        UT_LOG_ERROR("Failed to create KVP instance\n");
        return UT_STATUS_FAILURE;
    }

    if (ut_kvp_open(kvpInstance, configurationFile) != UT_KVP_STATUS_SUCCESS) {
        UT_LOG_ERROR("Failed to open config file: %s\n", configurationFile);
        ut_kvp_destroyInstance(kvpInstance);
        return UT_STATUS_FAILURE;
    }

    char keyBase[512];
    snprintf(keyBase, sizeof(keyBase), "%s.%s.%s", module, testGroup, testName);

    int portCount = (int)ut_kvp_getListCount(kvpInstance, (std::string(keyBase) + ".ports").c_str());
    for (int i = 0; i < portCount; ++i) {
        compositeInputConnectionConfig_t cfg;
        char portKey[600];
        snprintf(portKey, sizeof(portKey), "%s.ports.%d", keyBase, i);
        cfg.portId                 = (int32_t)ut_kvp_getUInt32Field(kvpInstance, (std::string(portKey) + ".id").c_str());
        cfg.connectionState        = (bool)ut_kvp_getBoolField(kvpInstance,    (std::string(portKey) + ".connection_state").c_str());
        cfg.expectSignalAfterConnect = (bool)ut_kvp_getBoolField(kvpInstance,  (std::string(portKey) + ".expect_signal_after_connect").c_str());
        cfg.maxWaitTimeMs          = ut_kvp_getUInt32Field(kvpInstance,        (std::string(portKey) + ".max_wait_time_ms").c_str());
        {
            std::string seqKey = std::string(portKey) + ".connection_sequence";
            int seqCount = (int)ut_kvp_getListCount(kvpInstance, seqKey.c_str());
            for (int s = 0; s < seqCount; ++s) {
                char idxKey[700];
                snprintf(idxKey, sizeof(idxKey), "%s.%d", seqKey.c_str(), s);
                cfg.connectionSequence.push_back((bool)ut_kvp_getBoolField(kvpInstance, idxKey));
            }
        }
        configs.push_back(cfg);
    }

    ut_kvp_destroyInstance(kvpInstance);
    return UT_STATUS_OK;
}

UT_status_t vts_compositeinput_parse_signalTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputSignalConfig_t>& configs)
{
    ut_kvp_instance_t *kvpInstance = ut_kvp_createInstance();
    if (!kvpInstance) {
        UT_LOG_ERROR("Failed to create KVP instance\n");
        return UT_STATUS_FAILURE;
    }

    if (ut_kvp_open(kvpInstance, configurationFile) != UT_KVP_STATUS_SUCCESS) {
        UT_LOG_ERROR("Failed to open config file: %s\n", configurationFile);
        ut_kvp_destroyInstance(kvpInstance);
        return UT_STATUS_FAILURE;
    }

    char keyBase[512];
    snprintf(keyBase, sizeof(keyBase), "%s.%s.%s", module, testGroup, testName);

    int portCount = (int)ut_kvp_getListCount(kvpInstance, (std::string(keyBase) + ".ports").c_str());
    for (int i = 0; i < portCount; ++i) {
        compositeInputSignalConfig_t cfg;
        char portKey[600];
        snprintf(portKey, sizeof(portKey), "%s.ports.%d", keyBase, i);
        cfg.portId          = (int32_t)ut_kvp_getUInt32Field(kvpInstance, (std::string(portKey) + ".id").c_str());
        cfg.maxWaitTimeMs   = ut_kvp_getUInt32Field(kvpInstance,          (std::string(portKey) + ".max_wait_time_ms").c_str());
        char signalStr[256] = {0};
        ut_kvp_getStringField(kvpInstance, (std::string(portKey) + ".expected_signal_status").c_str(), signalStr, sizeof(signalStr));
        cfg.expectedSignalStatus = convertStringToSignalStatus(std::string(signalStr));
        configs.push_back(cfg);
    }

    ut_kvp_destroyInstance(kvpInstance);
    return UT_STATUS_OK;
}

static void parseVideoModeEntryFromKVP(ut_kvp_instance_t *kvpInstance,
                                       const std::string& modeKey,
                                       compositeInputVideoModeEntry_t& mode)
{
    mode.width     = (int32_t)ut_kvp_getUInt32Field(kvpInstance, (modeKey + ".width").c_str());
    mode.height    = (int32_t)ut_kvp_getUInt32Field(kvpInstance, (modeKey + ".height").c_str());
    mode.interlaced = ut_kvp_getBoolField(kvpInstance, (modeKey + ".interlaced").c_str());
    mode.framerate  = ut_kvp_getFloatField(kvpInstance, (modeKey + ".framerate").c_str());
}

UT_status_t vts_compositeinput_parse_videoModeTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputVideoModeConfig_t>& configs)
{
    ut_kvp_instance_t *kvpInstance = ut_kvp_createInstance();
    if (!kvpInstance) {
        UT_LOG_ERROR("Failed to create KVP instance\n");
        return UT_STATUS_FAILURE;
    }

    if (ut_kvp_open(kvpInstance, configurationFile) != UT_KVP_STATUS_SUCCESS) {
        UT_LOG_ERROR("Failed to open config file: %s\n", configurationFile);
        ut_kvp_destroyInstance(kvpInstance);
        return UT_STATUS_FAILURE;
    }

    char keyBase[512];
    snprintf(keyBase, sizeof(keyBase), "%s.%s.%s", module, testGroup, testName);

    int portCount = (int)ut_kvp_getListCount(kvpInstance, (std::string(keyBase) + ".ports").c_str());
    for (int i = 0; i < portCount; ++i) {
        compositeInputVideoModeConfig_t cfg;
        char portKey[600];
        snprintf(portKey, sizeof(portKey), "%s.ports.%d", keyBase, i);
        cfg.portId        = (int32_t)ut_kvp_getUInt32Field(kvpInstance, (std::string(portKey) + ".id").c_str());
        cfg.maxWaitTimeMs = ut_kvp_getUInt32Field(kvpInstance, (std::string(portKey) + ".max_wait_time_ms").c_str());
        parseVideoModeEntryFromKVP(kvpInstance, std::string(portKey) + ".videomode1", cfg.videomode1);
        parseVideoModeEntryFromKVP(kvpInstance, std::string(portKey) + ".videomode2", cfg.videomode2);
        configs.push_back(cfg);
    }

    ut_kvp_destroyInstance(kvpInstance);
    return UT_STATUS_OK;
}

UT_status_t vts_compositeinput_parse_propertyTest(
    char *configurationFile, const char *module, const char *testGroup,
    const char *testName, std::vector<compositeInputPropertyConfig_t>& configs)
{
    ut_kvp_instance_t *kvpInstance = ut_kvp_createInstance();
    if (!kvpInstance) {
        UT_LOG_ERROR("Failed to create KVP instance\n");
        return UT_STATUS_FAILURE;
    }

    if (ut_kvp_open(kvpInstance, configurationFile) != UT_KVP_STATUS_SUCCESS) {
        UT_LOG_ERROR("Failed to open config file: %s\n", configurationFile);
        ut_kvp_destroyInstance(kvpInstance);
        return UT_STATUS_FAILURE;
    }

    char keyBase[512];
    snprintf(keyBase, sizeof(keyBase), "%s.%s.%s", module, testGroup, testName);

    int portCount = (int)ut_kvp_getListCount(kvpInstance, (std::string(keyBase) + ".ports").c_str());
    for (int i = 0; i < portCount; ++i) {
        compositeInputPropertyConfig_t cfg;
        char portKey[600];
        snprintf(portKey, sizeof(portKey), "%s.ports.%d", keyBase, i);
        cfg.portId        = (int32_t)ut_kvp_getUInt32Field(kvpInstance, (std::string(portKey) + ".id").c_str());
        cfg.maxWaitTimeMs = ut_kvp_getUInt32Field(kvpInstance,          (std::string(portKey) + ".max_wait_time_ms").c_str());
        configs.push_back(cfg);
    }

    ut_kvp_destroyInstance(kvpInstance);
    return UT_STATUS_OK;
}
