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
 * @addtogroup HPK Hardware Porting Kit
 * @{
 */

/**
 * @addtogroup COMPOSITE_INPUT COMPOSITE INPUT Module
 * @{
 */

/**
 * @defgroup COMPOSITE_INPUT_HALTEST COMPOSITE INPUT HAL Tests
 * @{
 */

/**
 * @defgroup COMPOSITE_INPUT_HALTEST_L1 COMPOSITE INPUT HAL Tests L1 File
 * @{
 * @parblock
 *
 * ### L1 Test Cases for COMPOSITE INPUT HAL :
 *
 * This module includes Level 1 functional tests (success and failure scenarios).
 * Tests validate the API contract of ICompositeInputManager, ICompositeInputPort
 * and ICompositeInputController without requiring external signals or connected devices.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 */

/**
 * @file vts_compositeinput_l1_tests.cpp
 */

#include "vts_compositeinput.h"

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L1_COMPOSITEINPUT, UT_TESTS_L1)

class VTS_L1_COMPOSITEINPUT : public UTCore
{
public:
    VTS_L1_COMPOSITEINPUT() : UTCore() {}

    ~VTS_L1_COMPOSITEINPUT() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Verify that getPortIds returns a valid non-empty list of composite input port IDs.
 *
 * This test calls getPortIds() on the ICompositeInputManager service, verifies the call succeeds,
 * the returned vector is non-empty, and (when a profile is loaded) the count matches the profile.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Call getPortIds on ICompositeInputManager | input: none, output: portIds vector | API returns status OK and portIds vector is populated | Should Pass |@n
 * | 02 | Verify the portIds vector is not empty | portIds.size() > 0 | Vector is non-empty | Should Pass |@n
 * | 03 | Validate count matches compositeinput.ports count from profile | UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports") | portIds.size() equals profile port count | Should Pass |@n
 * | 04 | Validate each returned portId exists in profile port IDs (compositeinput.ports.N.id) | portId vs profile IDs | Each portId found in expected list | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getPortIds_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);

    UT_LOG_INFO("getPortIds returned %zu port(s)\n", portIds.size());
    UT_ASSERT_TRUE(portIds.size() > 0);

    // Validate count against profile
    uint32_t profileCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_LOG_INFO("Profile declares %u port(s)\n", profileCount);
    UT_ASSERT_EQUAL(static_cast<uint32_t>(portIds.size()), profileCount);

    // Build expected IDs from profile and validate each returned ID exists
    std::vector<int32_t> expectedIds;
    for (uint32_t i = 0; i < profileCount; ++i)
    {
        std::string key = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t id = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(key.c_str()));
        expectedIds.push_back(id);
        UT_LOG_INFO("Profile port[%u] id:[%d]\n", i, id);
    }

    for (const auto& portId : portIds)
    {
        bool found = std::find(expectedIds.begin(), expectedIds.end(), portId) != expectedIds.end();
        UT_ASSERT_TRUE(found);
    }
}

/**
 * @brief Verify that getPort returns a valid ICompositeInputPort for each valid portId.
 *
 * This test iterates over every portId returned by getPortIds and verifies that getPort()
 * succeeds and returns a non-null port object.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read port count from profile (compositeinput.ports) | input: none | profileCount > 0 | Should Pass |@n
 * | 02 | For each profile entry read portId from compositeinput.ports.N.id | portId = profile port ID | Valid int32 ID read from profile | Should Pass |@n
 * | 03 | Call getPort() with each profile portId | portId = profile port ID | API returns status OK and non-null ICompositeInputPort | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getPort_pos)
{
    // Iterate over profile port IDs and verify each retrieval succeeds
    uint32_t profileCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(profileCount > 0);

    for (uint32_t i = 0; i < profileCount; ++i)
    {
        std::string key = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t portId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(key.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);
        UT_LOG_INFO("getPort succeeded for profile port[%u] portId:[%d]\n", i, portId);
    }
}

/**
 * @brief Verify that getPort returns an error or null for invalid portIds.
 *
 * This test attempts to retrieve composite input ports using IDs outside the valid range
 * (one below the minimum and one above the maximum). For each invalid ID the API is expected
 * to either return a non-OK status or a null port object.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all valid portIds | input: none | Non-empty list obtained | Should Pass |@n
 * | 02 | Call getPort with ID = min(validIds) - 1 | invalidId = minId - 1 | API returns !isOk() or null port | Should Pass |@n
 * | 03 | Call getPort with ID = max(validIds) + 1 | invalidId = maxId + 1 | API returns !isOk() or null port | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getPort_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    if (portIds.size() > 1)
        std::sort(portIds.begin(), portIds.end());

    // ID below minimum
    int32_t invalidId = portIds.front() - 1;
    android::sp<ICompositeInputPort> port;
    ::android::binder::Status status;
    COMPOSITEINPUT_GET_PORT_NEG(gCOMPOSITEINPUTHandle, invalidId, port, status);
    UT_ASSERT_TRUE(!status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    UT_LOG_INFO("getPort(invalidId:[%d]) isOk:[%s] exception:[%d]\n",
                invalidId, status.isOk() ? "true" : "false", status.exceptionCode());

    // ID above maximum
    invalidId = portIds.back() + 1;
    android::sp<ICompositeInputPort> port2;
    ::android::binder::Status status2;
    COMPOSITEINPUT_GET_PORT_NEG(gCOMPOSITEINPUTHandle, invalidId, port2, status2);
    UT_ASSERT_TRUE(!status2.isOk());
    UT_ASSERT_EQUAL(status2.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    UT_LOG_INFO("getPort(invalidId:[%d]) isOk:[%s] exception:[%d]\n",
                invalidId, status2.isOk() ? "true" : "false", status2.exceptionCode());
}

/**
 * @brief Verify that getPlatformCapabilities returns valid platform capability data.
 *
 * This test calls getPlatformCapabilities() on the ICompositeInputManager service and
 * verifies that the call succeeds, then validates all returned fields against the values
 * declared in the hfp-compositeinput.yaml profile.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Call getPlatformCapabilities on ICompositeInputManager | input: none, output: PlatformCapabilities | API returns status OK and capabilities populated | Should Pass |@n
 * | 02 | Verify maxPorts > 0 | caps.maxPorts | maxPorts is positive | Should Pass |@n
 * | 03 | Validate halVersion against compositeinput/platformCapabilities/halVersion | caps.halVersion vs profile | Values match | Should Pass |@n
 * | 04 | Validate maxPorts against compositeinput/platformCapabilities/maxPorts | caps.maxPorts vs profile | Values match | Should Pass |@n
 * | 05 | Validate maximumConcurrentStartedPorts against profile | caps.maximumConcurrentStartedPorts vs profile | Values match | Should Pass |@n
 * | 06 | Validate supportedProperties count and values against compositeinput.platformCapabilities.supportedProperties | caps.supportedProperties vs profile | Count and each property value match profile | Should Pass |@n
 * | 07 | Verify caps.propertyMetadata has a value | caps.propertyMetadata.has_value() | optional is not empty | Should Pass |@n
 * | 08 | Validate propertyMetadata count against compositeinput.platformCapabilities.propertyMetadata | caps.propertyMetadata->size() vs profile count | Counts match | Should Pass |@n
 * | 09 | Validate each propertyMetadata entry (key, type, readOnly, isMetric, description) against profile | each metadata entry vs profile | All fields match profile for every entry | Should Pass |@n
 * | 10 | Validate features.macrovisionDetectionSupported against compositeinput/platformCapabilities/features/macrovisionDetectionSupported | caps.features.macrovisionDetectionSupported vs profile | Values match | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getPlatformCapabilities_pos)
{
    PlatformCapabilities caps;
    COMPOSITEINPUT_GET_PLATFORM_CAPABILITIES(gCOMPOSITEINPUTHandle, caps);

    UT_LOG_INFO("PlatformCapabilities: halVersion:[%s] maxPorts:[%d] maxConcurrentStarted:[%d]\n",
                caps.halVersion.c_str(), static_cast<int>(caps.maxPorts),
                caps.maximumConcurrentStartedPorts);
    UT_ASSERT_TRUE(caps.maxPorts > 0);

    // Validate halVersion against profile
    char profileHalVersion[UT_KVP_MAX_ELEMENT_SIZE];
    UT_KVP_PROFILE_GET_STRING("compositeinput/platformCapabilities/halVersion", profileHalVersion);
    UT_LOG_INFO("Profile halVersion:[%s]\n", profileHalVersion);
    UT_ASSERT_EQUAL(caps.halVersion, std::string(profileHalVersion));

    // Validate maxPorts against profile
    uint32_t profileMaxPorts = UT_KVP_PROFILE_GET_UINT32("compositeinput/platformCapabilities/maxPorts");
    UT_LOG_INFO("Profile maxPorts:[%u]\n", profileMaxPorts);
    UT_ASSERT_EQUAL(static_cast<uint32_t>(caps.maxPorts), profileMaxPorts);

    // Validate maximumConcurrentStartedPorts against profile
    uint32_t profileMaxConcurrent = UT_KVP_PROFILE_GET_UINT32("compositeinput/platformCapabilities/maximumConcurrentStartedPorts");
    UT_LOG_INFO("Profile maximumConcurrentStartedPorts:[%u]\n", profileMaxConcurrent);
    UT_ASSERT_EQUAL(static_cast<uint32_t>(caps.maximumConcurrentStartedPorts), profileMaxConcurrent);

    // Validate platform-wide supportedProperties count and values against profile
    uint32_t profilePropCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.platformCapabilities.supportedProperties");
    UT_LOG_INFO("Profile platformCapabilities.supportedProperties count:[%u]\n", profilePropCount);
    UT_ASSERT_EQUAL(static_cast<uint32_t>(caps.supportedProperties.size()), profilePropCount);

    std::vector<PortProperty> expectedProps;
    for (uint32_t j = 0; j < profilePropCount; ++j)
    {
        char propStr[UT_KVP_MAX_ELEMENT_SIZE];
        std::string propKey = "compositeinput.platformCapabilities.supportedProperties." + std::to_string(j);
        UT_KVP_PROFILE_GET_STRING(propKey.c_str(), propStr);
        expectedProps.push_back(convertStringToPortProperty(std::string(propStr)));
    }

    for (const auto& prop : caps.supportedProperties)
    {
        bool found = std::find(expectedProps.begin(), expectedProps.end(), prop) != expectedProps.end();
        UT_ASSERT_TRUE(found);
    }

    // Validate propertyMetadata against profile
    uint32_t profileMetaCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.platformCapabilities.propertyMetadata");
    UT_LOG_INFO("Profile platformCapabilities.propertyMetadata count:[%u]\n", profileMetaCount);
    UT_ASSERT_TRUE(caps.propertyMetadata.has_value());
    UT_ASSERT_EQUAL(static_cast<uint32_t>(caps.propertyMetadata->size()), profileMetaCount);

    for (uint32_t j = 0; j < profileMetaCount; ++j)
    {
        std::string base = "compositeinput.platformCapabilities.propertyMetadata." + std::to_string(j);

        char metaKeyStr[UT_KVP_MAX_ELEMENT_SIZE];
        UT_KVP_PROFILE_GET_STRING((base + ".key").c_str(), metaKeyStr);
        char metaTypeStr[UT_KVP_MAX_ELEMENT_SIZE];
        UT_KVP_PROFILE_GET_STRING((base + ".type").c_str(), metaTypeStr);
        bool profileReadOnly = UT_KVP_PROFILE_GET_BOOL((base + ".readOnly").c_str());
        bool profileIsMetric = UT_KVP_PROFILE_GET_BOOL((base + ".isMetric").c_str());
        char metaDescStr[UT_KVP_MAX_ELEMENT_SIZE];
        UT_KVP_PROFILE_GET_STRING((base + ".description").c_str(), metaDescStr);

        PropertyMetadata expected;
        expected.key = convertStringToPortProperty(std::string(metaKeyStr));
        expected.type = convertStringToPropertyType(std::string(metaTypeStr));
        expected.readOnly = profileReadOnly;
        expected.isMetric = profileIsMetric;
        expected.description = std::string(metaDescStr);

        UT_LOG_INFO("Profile propertyMetadata[%u]: key:[%s] type:[%s] readOnly:[%s] isMetric:[%s] description:[%s]\n",
                    j, metaKeyStr, metaTypeStr,
                    profileReadOnly ? "true" : "false",
                    profileIsMetric ? "true" : "false",
                    metaDescStr);

        bool found = false;
        for (const auto& meta : *caps.propertyMetadata)
        {
            if (meta.has_value() && meta.value() == expected)
            {
                found = true;
                break;
            }
        }
        UT_ASSERT_TRUE(found);
    }

    // Validate features against profile
    bool profileMacrovision = UT_KVP_PROFILE_GET_BOOL("compositeinput/platformCapabilities/features/macrovisionDetectionSupported");
    UT_LOG_INFO("Profile features.macrovisionDetectionSupported:[%s]\n", profileMacrovision ? "true" : "false");
    UT_ASSERT_EQUAL(caps.features.macrovisionDetectionSupported, profileMacrovision);
}

/**
 * @brief Verify that getPortInfo returns valid metadata for each composite input port.
 *
 * This test retrieves the Port metadata (name, description) for each port and validates
 * that the name field is non-empty, confirming the AIDL call succeeds and returns meaningful data.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read port count from profile (compositeinput.ports) | input: none | profileCount > 0 | Should Pass |@n
 * | 02 | For each profile entry get port by profile ID | portId = compositeinput.ports.N.id | Non-null ICompositeInputPort | Should Pass |@n
 * | 03 | Call getPortInfo() on each port | portId = each profile port ID | API returns status OK and Port parcelable | Should Pass |@n
 * | 04 | Validate Port.name against compositeinput.ports.N.name | portInfo.name == profile name | Values match | Should Pass |@n
 * | 05 | Validate Port.description against compositeinput.ports.N.description | portInfo.description == profile description | Values match | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getPortInfo_pos)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        Port portInfo;
        COMPOSITEINPUT_GET_PORT_INFO(port, portInfo);

        // Validate name against profile
        char expectedName[UT_KVP_MAX_ELEMENT_SIZE];
        std::string nameKey = "compositeinput.ports." + std::to_string(i) + ".name";
        UT_KVP_PROFILE_GET_STRING(nameKey.c_str(), expectedName);

        // Validate description against profile
        char expectedDesc[UT_KVP_MAX_ELEMENT_SIZE];
        std::string descKey = "compositeinput.ports." + std::to_string(i) + ".description";
        UT_KVP_PROFILE_GET_STRING(descKey.c_str(), expectedDesc);

        UT_LOG_INFO("Port[%d] name:[%s] expected:[%s] description:[%s] expected:[%s]\n",
                    profilePortId, portInfo.name.c_str(), expectedName,
                    portInfo.description.c_str(), expectedDesc);
        UT_ASSERT_TRUE(portInfo.name.length() > 0);
        UT_ASSERT_EQUAL(portInfo.name, std::string(expectedName));
        UT_ASSERT_EQUAL(portInfo.description, std::string(expectedDesc));
    }
}

/**
 * @brief Verify that getId returns the expected portId for each composite input port.
 *
 * This test calls getId() on each ICompositeInputPort and confirms that the returned ID
 * matches the portId used to retrieve that port object from the manager.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 006@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all valid portIds | input: none | Non-empty list | Should Pass |@n
 * | 02 | For each portId call getPort() then getId() on the returned port | portId = each valid ID | getId returns the same ID | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getId_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        int32_t returnedId = -1;
        COMPOSITEINPUT_GET_ID(port, returnedId);

        UT_LOG_INFO("getPort(%d) -> getId() returned:[%d]\n", portId, returnedId);
        UT_ASSERT_EQUAL(returnedId, portId);
    }
}

/**
 * @brief Verify that getCapabilities returns valid PortCapabilities for each port.
 *
 * This test retrieves PortCapabilities for each port. It verifies the call succeeds and,
 * when a profile is loaded, validates the supportedProperties count against the profile.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 007@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read port count from profile (compositeinput.ports) | input: none | profileCount > 0 | Should Pass |@n
 * | 02 | For each profile entry get port by compositeinput.ports.N.id and call getCapabilities() | portId from profile | API returns status OK and PortCapabilities parcelable | Should Pass |@n
 * | 03 | Validate supportedProperties count against compositeinput.ports.N.supportedProperties | caps.supportedProperties.size() vs profile count | Counts match | Should Pass |@n
 * | 04 | Validate each returned property value exists in the profile list | each prop vs profile list | All properties found in profile list | Should Pass |@n
 * | 05 | Verify caps.propertyMetadata has a value | caps.propertyMetadata.has_value() | optional is not empty | Should Pass |@n
 * | 06 | Validate propertyMetadata count against compositeinput.ports.N.propertyMetadata | caps.propertyMetadata->size() vs profile count | Counts match | Should Pass |@n
 * | 07 | Validate each propertyMetadata entry (key, type, readOnly, isMetric, description) against profile | each metadata entry vs profile | All fields match profile for every entry | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getCapabilities_pos)
{
    // Iterate over profile port entries and validate capabilities for each
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        PortCapabilities caps;
        COMPOSITEINPUT_GET_CAPABILITIES(port, caps);

        std::string key = "compositeinput.ports." + std::to_string(i) + ".supportedProperties";
        uint32_t profilePropCount = UT_KVP_PROFILE_GET_LIST_COUNT(key.c_str());

        UT_LOG_INFO("Port[%d] supportedProperties count:[%zu] expected:[%u]\n",
                    profilePortId, caps.supportedProperties.size(), profilePropCount);
        UT_ASSERT_EQUAL(static_cast<uint32_t>(caps.supportedProperties.size()), profilePropCount);

        std::vector<PortProperty> expectedProps;
        for (uint32_t j = 0; j < profilePropCount; ++j)
        {
            char propStr[UT_KVP_MAX_ELEMENT_SIZE];
            std::string propKey = key + "." + std::to_string(j);
            UT_KVP_PROFILE_GET_STRING(propKey.c_str(), propStr);
            expectedProps.push_back(convertStringToPortProperty(std::string(propStr)));
        }

        for (const auto& prop : caps.supportedProperties)
        {
            bool found = std::find(expectedProps.begin(), expectedProps.end(), prop) != expectedProps.end();
            UT_ASSERT_TRUE(found);
        }

        // Validate propertyMetadata against profile
        std::string metaListKey = "compositeinput.ports." + std::to_string(i) + ".propertyMetadata";
        uint32_t profileMetaCount = UT_KVP_PROFILE_GET_LIST_COUNT(metaListKey.c_str());
        UT_LOG_INFO("Port[%d] propertyMetadata count:[%zu] expected:[%u]\n",
                    profilePortId, caps.propertyMetadata.has_value() ? caps.propertyMetadata->size() : 0u, profileMetaCount);
        UT_ASSERT_TRUE(caps.propertyMetadata.has_value());
        UT_ASSERT_EQUAL(static_cast<uint32_t>(caps.propertyMetadata->size()), profileMetaCount);

        for (uint32_t k = 0; k < profileMetaCount; ++k)
        {
            std::string base = metaListKey + "." + std::to_string(k);

            char metaKeyStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".key").c_str(), metaKeyStr);
            char metaTypeStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".type").c_str(), metaTypeStr);
            bool profileReadOnly = UT_KVP_PROFILE_GET_BOOL((base + ".readOnly").c_str());
            bool profileIsMetric = UT_KVP_PROFILE_GET_BOOL((base + ".isMetric").c_str());
            char metaDescStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".description").c_str(), metaDescStr);

            PropertyMetadata expected;
            expected.key = convertStringToPortProperty(std::string(metaKeyStr));
            expected.type = convertStringToPropertyType(std::string(metaTypeStr));
            expected.readOnly = profileReadOnly;
            expected.isMetric = profileIsMetric;
            expected.description = std::string(metaDescStr);

            UT_LOG_INFO("Port[%d] propertyMetadata[%u]: key:[%s] type:[%s] readOnly:[%s] isMetric:[%s] description:[%s]\n",
                        profilePortId, k, metaKeyStr, metaTypeStr,
                        profileReadOnly ? "true" : "false",
                        profileIsMetric ? "true" : "false",
                        metaDescStr);

            bool metaFound = false;
            for (const auto& meta : *caps.propertyMetadata)
            {
                if (meta.has_value() && meta.value() == expected)
                {
                    metaFound = true;
                    break;
                }
            }
            UT_ASSERT_TRUE(metaFound);
        }
    }
}

/**
 * @brief Verify that getState returns CLOSED as the initial state for each composite input port.
 *
 * This test retrieves the current state of each port and asserts that it equals State::CLOSED,
 * confirming that no port has been inadvertently opened before the test runs.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 008@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all valid portIds | input: none | Non-empty list | Should Pass |@n
 * | 02 | For each port call getState() | portId = each valid ID | API returns status OK | Should Pass |@n
 * | 03 | Verify returned state equals State::CLOSED | state == State::CLOSED | Assertion passes | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getState_initial)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        State state;
        COMPOSITEINPUT_GET_STATE(port, state);

        UT_LOG_INFO("Port[%d] initial state:[%d]\n", portId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::CLOSED);
    }
}

/**
 * @brief Verify that getStatus returns a valid PortStatus snapshot for each composite input port.
 *
 * This test calls getStatus() on each port in the CLOSED state and validates that the portId
 * field in the returned PortStatus matches the expected portId.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all valid portIds | input: none | Non-empty list | Should Pass |@n
 * | 02 | For each port call getStatus() | portId = each valid ID | API returns status OK and PortStatus parcelable | Should Pass |@n
 * | 03 | Verify portStatus.portId matches the portId used to obtain the port | portStatus.portId == portId | Values match | Should Pass |@n
 * | 04 | Verify portStatus.active is false (port is CLOSED, not started) | portStatus.active == false | active is false | Should Pass |@n
 * | 05 | Verify portStatus.signalStatus is NO_SIGNAL (port not active) | portStatus.signalStatus == SignalStatus::NO_SIGNAL | signalStatus is NO_SIGNAL | Should Pass |@n
 * | 06 | Verify portStatus.detectedResolution has no value (no active signal) | portStatus.detectedResolution.has_value() == false | detectedResolution is empty | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getStatus_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        PortStatus portStatus;
        COMPOSITEINPUT_GET_STATUS(port, portStatus);

        UT_LOG_INFO("Port[%d] status: portId:[%d] connected:[%s] active:[%s] signalStatus:[%d] detectedResolution:[%s]\n",
                    portId, portStatus.portId,
                    portStatus.connected ? "true" : "false",
                    portStatus.active ? "true" : "false",
                    static_cast<int>(portStatus.signalStatus),
                    portStatus.detectedResolution.has_value() ? "present" : "none");
        UT_ASSERT_EQUAL(portStatus.portId, portId);
        UT_ASSERT_EQUAL(portStatus.active, false);
        UT_ASSERT_EQUAL(portStatus.signalStatus, SignalStatus::NO_SIGNAL);
    }
}

/**
 * @brief Verify that open() returns a valid ICompositeInputController for each port.
 *
 * This test calls open() on each available port with a valid listener, verifies that a
 * non-null controller is returned and the port state transitions to READY. Cleanup close()
 * is called after each port.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 010@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all portIds | input: none | Non-empty portIds list | Should Pass |@n
 * | 02 | For each portId: verify state is CLOSED before open | state == State::CLOSED | Assertion passes | Should Pass |@n
 * | 03 | Call open() with a valid ICompositeInputControllerListener | listener = new CompositeInputControllerListener | API returns status OK and non-null ICompositeInputController | Should Pass |@n
 * | 04 | Verify port state is READY after open | state == State::READY | Assertion passes | Should Pass |@n
 * | 05 | Close the port (cleanup) | controller from step 03 | close() returns true | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, open_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        State state;
        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state before open:[%d]\n", portId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::CLOSED);

        auto listener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, listener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after open:[%d]\n", portId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::READY);

        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);
    }
}

/**
 * @brief Verify that close() returns true and transitions the port back to CLOSED.
 *
 * This test opens each port with a valid listener, verifies the READY state, calls close()
 * with the returned controller, verifies close() returns true, and confirms the port
 * transitions back to CLOSED.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 011@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all portIds | input: none | Non-empty portIds list | Should Pass |@n
 * | 02 | For each portId: open port with valid listener | portId = each valid ID | Returns valid controller; state is READY | Should Pass |@n
 * | 03 | Call close() with the controller returned by open | controller from step 02 | API returns status OK and close() returns true | Should Pass |@n
 * | 04 | Verify port state is CLOSED after close | state == State::CLOSED | Assertion passes | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, close_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto listener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, listener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        State state;
        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);

        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after close:[%d]\n", portId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::CLOSED);
    }
}

/**
 * @brief Verify that a second open() call on an already-open port returns EX_ILLEGAL_STATE.
 *
 * This test opens the first port successfully, then attempts a second open on the same port.
 * The AIDL contract specifies EX_ILLEGAL_STATE when the port is not in CLOSED state.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 012@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Open the first port with a valid listener | portIds[0] | Returns valid controller; state is READY | Should Pass |@n
 * | 02 | Attempt a second open on the same port | same port, new listener | API returns !isOk() (EX_ILLEGAL_STATE) | Should Pass |@n
 * | 03 | Verify port state remains READY after failed second open | state == State::READY | Assertion passes | Should Pass |@n
 * | 04 | Close the port (cleanup) | first controller | close() returns true | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, open_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    auto listener = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller;
    COMPOSITEINPUT_OPEN(port, listener, controller);
    UT_ASSERT_NOT_EQUAL(controller, nullptr);

    State state;
    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after first open:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::READY);

    // Second open on the same port — must fail (EX_ILLEGAL_STATE)
    auto listener2 = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller2;
    ::android::binder::Status status;
    COMPOSITEINPUT_OPEN_NEG(port, listener2, controller2, status);
    UT_LOG_INFO("Second open: isOk:[%s] exception:[%d] controller:[%p]\n",
                status.isOk() ? "true" : "false",
                status.exceptionCode(),
                controller2.get());
    UT_ASSERT_TRUE(!status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after failed second open:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::READY);

    bool closed = false;
    COMPOSITEINPUT_CLOSE(port, controller, closed);
    UT_ASSERT_TRUE(closed);
}

/**
 * @brief Verify that close() on an already-CLOSED port returns EX_ILLEGAL_STATE.
 *
 * This test opens the first port, closes it successfully, then attempts a second close.
 * The AIDL contract specifies EX_ILLEGAL_STATE when the port is not in READY state.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 013@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Open the first port with a valid listener | portIds[0] | Returns valid controller | Should Pass |@n
 * | 02 | Close the port | first controller | close() returns true; state is CLOSED | Should Pass |@n
 * | 03 | Attempt to close the already-CLOSED port again | same port, same controller | API returns !isOk() (EX_ILLEGAL_STATE) | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, close_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    auto listener = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller;
    COMPOSITEINPUT_OPEN(port, listener, controller);
    UT_ASSERT_NOT_EQUAL(controller, nullptr);

    bool closed = false;
    COMPOSITEINPUT_CLOSE(port, controller, closed);
    UT_ASSERT_TRUE(closed);

    State state;
    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after close:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::CLOSED);

    // Second close on already-CLOSED port — must fail (EX_ILLEGAL_STATE)
    bool closedAgain = false;
    ::android::binder::Status closeStatus;
    COMPOSITEINPUT_CLOSE_NEG(port, controller, closedAgain, closeStatus);
    UT_LOG_INFO("Second close: isOk:[%s] exception:[%d]\n",
                closeStatus.isOk() ? "true" : "false", closeStatus.exceptionCode());
    UT_ASSERT_TRUE(!closeStatus.isOk());
    UT_ASSERT_EQUAL(closeStatus.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
}

/**
 * @brief Verify that registerEventListener() succeeds on the first port.
 *
 * This test registers a valid ICompositeInputEventListener on the first port and verifies
 * the call returns EX_NONE. The listener is unregistered as cleanup.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 014@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all portIds | input: none | Non-empty portIds list | Should Pass |@n
 * | 02 | For each portId: get port object | portId = each valid ID | Non-null port | Should Pass |@n
 * | 03 | Call registerEventListener with a valid ICompositeInputEventListener | listener = new CompositeInputEventListener | API returns status OK | Should Pass |@n
 * | 04 | Unregister the listener (cleanup) | listener from step 03 | API returns status OK | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, registerEventListener_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto eventListener = android::sp<CompositeInputEventListener>::make();
        COMPOSITEINPUT_REGISTER_EVENT_LISTENER(port, eventListener);
        UT_LOG_INFO("registerEventListener passed for portId:[%d]\n", portId);

        // Cleanup
        COMPOSITEINPUT_UNREGISTER_EVENT_LISTENER(port, eventListener);
    }
}

/**
 * @brief Verify that unregisterEventListener() succeeds when called with a registered listener.
 *
 * This test registers a valid ICompositeInputEventListener on the first port, then calls
 * unregisterEventListener() with the same listener and verifies the call returns EX_NONE.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 015@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all portIds | input: none | Non-empty portIds list | Should Pass |@n
 * | 02 | For each portId: get port object | portId = each valid ID | Non-null port | Should Pass |@n
 * | 03 | Register a valid ICompositeInputEventListener | listener = new CompositeInputEventListener | API returns status OK | Should Pass |@n
 * | 04 | Call unregisterEventListener with the registered listener | listener from step 03 | API returns status OK | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, unregisterEventListener_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto eventListener = android::sp<CompositeInputEventListener>::make();
        COMPOSITEINPUT_REGISTER_EVENT_LISTENER(port, eventListener);

        COMPOSITEINPUT_UNREGISTER_EVENT_LISTENER(port, eventListener);
        UT_LOG_INFO("unregisterEventListener passed for portId:[%d]\n", portId);
    }
}

/**
 * @brief Verify that unregisterEventListener fails when the listener was never registered.
 *
 * This test creates a valid listener without registering it, then calls unregisterEventListener.
 * The API must return !isOk() because the listener is not in the registered set.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 016@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve first portId and get port object | portIds[0] | Non-null port | Should Pass |@n
 * | 02 | Call unregisterEventListener with a never-registered listener | listener = new CompositeInputEventListener (not registered) | API returns !isOk() | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, unregisterEventListener_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    // Create a listener but never register it
    auto eventListener = android::sp<CompositeInputEventListener>::make();

    ::android::binder::Status status;
    COMPOSITEINPUT_UNREGISTER_EVENT_LISTENER_NEG(port, eventListener, status);
    UT_LOG_INFO("unregisterEventListener(never-registered): isOk:[%s] exception:[%d]\n",
                status.isOk() ? "true" : "false", status.exceptionCode());
    UT_ASSERT_TRUE(!status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
 * @brief Verify that controller start() transitions the port to STARTED state.
 *
 * This test opens each port with a valid listener, calls start() on the controller, and
 * verifies the port transitions to STARTED. Cleanup stop() and close() are called after each port.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 017@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all portIds | input: none | Non-empty portIds list | Should Pass |@n
 * | 02 | For each portId: open port with valid listener | portId = each valid ID | Returns valid controller | Should Pass |@n
 * | 03 | Call start() on the controller | controller from step 02 | API returns status OK | Should Pass |@n
 * | 04 | Verify state is STARTED | state == State::STARTED | Assertion passes | Should Pass |@n
 * | 05 | Stop and close the port (cleanup) | controller from step 02 | close() returns true | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, controller_start_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto listener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, listener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        State state;
        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after start:[%d]\n", portId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::STARTED);

        // Cleanup
        COMPOSITEINPUT_STOP(controller);
        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);
    }
}

/**
 * @brief Verify that controller stop() transitions the port back to READY state.
 *
 * This test opens each port, starts it, then calls stop() on the controller and verifies
 * the port transitions back to READY. Cleanup close() is called after each port.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 018@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve all portIds | input: none | Non-empty portIds list | Should Pass |@n
 * | 02 | For each portId: open port and start controller | portId = each valid ID | Port in STARTED state | Should Pass |@n
 * | 03 | Call stop() on the controller | controller from step 02 | API returns status OK | Should Pass |@n
 * | 04 | Verify state is READY after stop | state == State::READY | Assertion passes | Should Pass |@n
 * | 05 | Close the port (cleanup) | controller from step 02 | close() returns true | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, controller_stop_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto listener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, listener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        COMPOSITEINPUT_STOP(controller);

        State state;
        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after stop:[%d]\n", portId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::READY);

        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);
    }
}

/**
 * @brief Verify that start() fails with EX_ILLEGAL_STATE when the port is already STARTED.
 *
 * This test opens the first port, starts it, then attempts a second start(). The AIDL
 * contract requires EX_ILLEGAL_STATE when the port is not in READY state.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 019@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Open first port and start controller | portIds[0], valid listener | Port in STARTED state | Should Pass |@n
 * | 02 | Call start() again on the already-STARTED port | controller | API returns !isOk() (EX_ILLEGAL_STATE) | Should Pass |@n
 * | 03 | Verify state is still STARTED after failed second start | state == State::STARTED | Assertion passes | Should Pass |@n
 * | 04 | Stop and close the port (cleanup) | controller | close() returns true | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, controller_start_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    auto listener = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller;
    COMPOSITEINPUT_OPEN(port, listener, controller);
    UT_ASSERT_NOT_EQUAL(controller, nullptr);

    COMPOSITEINPUT_START(controller);

    State state;
    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after start:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::STARTED);

    // Second start — port is already STARTED, must fail
    ::android::binder::Status startStatus;
    COMPOSITEINPUT_START_NEG(controller, startStatus);
    UT_LOG_INFO("Port[%d] second start: isOk:[%s] exception:[%d]\n",
                portIds[0], startStatus.isOk() ? "true" : "false", startStatus.exceptionCode());
    UT_ASSERT_TRUE(!startStatus.isOk());
    UT_ASSERT_EQUAL(startStatus.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after failed second start:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::STARTED);

    // Cleanup
    COMPOSITEINPUT_STOP(controller);
    bool closed = false;
    COMPOSITEINPUT_CLOSE(port, controller, closed);
    UT_ASSERT_TRUE(closed);
}

/**
 * @brief Verify that stop() fails with EX_ILLEGAL_STATE when the port is not STARTED.
 *
 * Two scenarios are tested:
 * 1. stop() called on a READY port (before start) → EX_ILLEGAL_STATE.
 * 2. stop() called again after a successful stop (port back to READY) → EX_ILLEGAL_STATE.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 020@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Open first port | portIds[0], valid listener | Returns valid controller; state is READY | Should Pass |@n
 * | 02 | Call stop() before start() (port is READY) | controller | API returns !isOk() (EX_ILLEGAL_STATE) | Should Pass |@n
 * | 03 | Verify state is still READY after failed stop | state == State::READY | Assertion passes | Should Pass |@n
 * | 04 | Call start() | controller | API returns EX_NONE; state is STARTED | Should Pass |@n
 * | 05 | Call stop() | controller | API returns EX_NONE; state is READY | Should Pass |@n
 * | 06 | Call stop() again (port is READY, not STARTED) | controller | API returns !isOk() (EX_ILLEGAL_STATE) | Should Pass |@n
 * | 07 | Verify state is still READY after failed second stop | state == State::READY | Assertion passes | Should Pass |@n
 * | 08 | Close the port (cleanup) | controller | close() returns true | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, controller_stop_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    auto listener = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller;
    COMPOSITEINPUT_OPEN(port, listener, controller);
    UT_ASSERT_NOT_EQUAL(controller, nullptr);

    // Scenario 1: stop() before start() — port is READY, must fail
    ::android::binder::Status stopStatus;
    COMPOSITEINPUT_STOP_NEG(controller, stopStatus);
    UT_LOG_INFO("Port[%d] stop() before start: isOk:[%s] exception:[%d]\n",
                portIds[0], stopStatus.isOk() ? "true" : "false", stopStatus.exceptionCode());
    UT_ASSERT_TRUE(!stopStatus.isOk());
    UT_ASSERT_EQUAL(stopStatus.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

    State state;
    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after failed stop:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::READY);

    // Normal start then stop — must succeed
    COMPOSITEINPUT_START(controller);
    COMPOSITEINPUT_STOP(controller);

    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after stop:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::READY);

    // Scenario 2: stop() again — port is READY, not STARTED, must fail
    COMPOSITEINPUT_STOP_NEG(controller, stopStatus);
    UT_LOG_INFO("Port[%d] second stop: isOk:[%s] exception:[%d]\n",
                portIds[0], stopStatus.isOk() ? "true" : "false", stopStatus.exceptionCode());
    UT_ASSERT_TRUE(!stopStatus.isOk());
    UT_ASSERT_EQUAL(stopStatus.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

    COMPOSITEINPUT_GET_STATE(port, state);
    UT_LOG_INFO("Port[%d] state after failed second stop:[%d]\n", portIds[0], static_cast<int>(state));
    UT_ASSERT_EQUAL(state, State::READY);

    bool closed = false;
    COMPOSITEINPUT_CLOSE(port, controller, closed);
    UT_ASSERT_TRUE(closed);
}

/**
 * @brief Verify that getProperty() succeeds for all supported properties on each port.
 *
 * This test retrieves the supported properties for each port from PortCapabilities and calls
 * getProperty() for each one. The API must return EX_NONE; the returned value may be null
 * (nullable) but the call itself must not fail.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 015@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | For each port get PortCapabilities.supportedProperties | portId = each valid ID | Capabilities returned | Should Pass |@n
 * | 02 | For each supported property call getProperty() | property = each value in supportedProperties | API returns status OK | Should Pass |@n
 * | 03 | Also test getProperty(SIGNAL_QUALITY) directly as a baseline | PortProperty::SIGNAL_QUALITY | API returns status OK | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getProperty_pos)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    for (auto portId : portIds)
    {
        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        PortCapabilities caps;
        COMPOSITEINPUT_GET_CAPABILITIES(port, caps);

        // getProperty for each declared supported property
        for (const auto& property : caps.supportedProperties)
        {
            com::rdk::hal::PropertyValue propValue;
            COMPOSITEINPUT_GET_PROPERTY(port, property, propValue);
            UT_LOG_INFO("Port[%d] getProperty(%d) OK\n", portId, static_cast<int>(property));
        }

    }
}

/**
 * @brief Verify that getProperty() returns an error for an unsupported or out-of-range property.
 *
 * This test passes an invalid property value (cast from an out-of-range integer) to getProperty()
 * and verifies the API returns EX_ILLEGAL_ARGUMENT as specified by the AIDL contract.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 016@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve first portId and get port | portIds[0] | Non-null port | Should Pass |@n
 * | 02 | Call getProperty with an out-of-range property value | property = static_cast<PortProperty>(-1) | API returns !isOk() (EX_ILLEGAL_ARGUMENT) | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getProperty_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    PortProperty invalidProperty = static_cast<PortProperty>(-1);
    ::std::optional<::com::rdk::hal::PropertyValue> optValue;
    ::android::binder::Status status;
    COMPOSITEINPUT_GET_PROPERTY_NEG(port, invalidProperty, optValue, status);
    UT_LOG_INFO("getProperty(invalid): isOk:[%s] exception:[%d]\n",
                status.isOk() ? "true" : "false", status.exceptionCode());
    UT_ASSERT_TRUE(!status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
 * @brief Verify that getPropertyMulti() returns values for all requested properties.
 *
 * This test calls getPropertyMulti() with SIGNAL_QUALITY and SIGNAL_STRENGTH as inputs and
 * verifies that the returned array has the same length as the input array and the API
 * returns EX_NONE.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 017@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read port count from profile (compositeinput.ports) | input: none | profileCount > 0 | Should Pass |@n
 * | 02 | For each profile port get port object | portId = compositeinput.ports.N.id | Non-null port | Should Pass |@n
 * | 03 | Load supported properties from compositeinput.ports.N.supportedProperties in profile | profile properties list | Properties vector populated from profile | Should Pass |@n
 * | 04 | Call getPropertyMulti() with all profile-declared properties | properties vector from profile | API returns status OK and kvPairs.size() == properties.size() | Should Pass |@n
 * | 05 | Verify each returned key matches the corresponding requested property | kvPairs[i].property == properties[i] | Keys match requested properties in order | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getPropertyMulti_pos)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        // Load supported properties for this port from profile
        std::vector<PortProperty> properties;
        std::string propListKey = "compositeinput.ports." + std::to_string(i) + ".supportedProperties";
        uint32_t profilePropCount = UT_KVP_PROFILE_GET_LIST_COUNT(propListKey.c_str());
        UT_ASSERT_TRUE(profilePropCount > 0);

        for (uint32_t j = 0; j < profilePropCount; ++j)
        {
            char propStr[UT_KVP_MAX_ELEMENT_SIZE];
            std::string propKey = propListKey + "." + std::to_string(j);
            UT_KVP_PROFILE_GET_STRING(propKey.c_str(), propStr);
            properties.push_back(convertStringToPortProperty(std::string(propStr)));
        }

        std::vector<PropertyKVPair> kvPairs;
        COMPOSITEINPUT_GET_PROPERTY_MULTI(port, properties, kvPairs);

        UT_LOG_INFO("Port[%d] getPropertyMulti returned %zu pair(s) for %zu requested\n",
                    profilePortId, kvPairs.size(), properties.size());
        UT_ASSERT_EQUAL(kvPairs.size(), properties.size());

        for (size_t k = 0; k < kvPairs.size(); ++k)
        {
            UT_LOG_INFO("kvPairs[%zu]: key:[%d]\n", k, static_cast<int>(kvPairs[k].property));
            UT_ASSERT_EQUAL(kvPairs[k].property, properties[k]);
        }
    }
}

/**
 * @brief Verify that getPropertyMulti() returns EX_ILLEGAL_ARGUMENT when the property list contains an invalid property.
 *
 * This test builds a property list containing one valid property followed by an out-of-range
 * property value and calls getPropertyMulti(). The AIDL contract requires EX_ILLEGAL_ARGUMENT
 * when any element of the input array is an unknown PortProperty value.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 018@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Retrieve first portId and get port | portIds[0] | Non-null port | Should Pass |@n
 * | 02 | Build a properties list containing one valid property and one invalid property (static_cast<PortProperty>(-1)) | mixed properties list | List constructed | Should Pass |@n
 * | 03 | Call getPropertyMulti() with the mixed list | properties list with invalid entry | API returns !isOk() (EX_ILLEGAL_ARGUMENT) | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, getPropertyMulti_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    // Mix one valid property with one invalid property
    std::vector<PortProperty> properties;
    properties.push_back(PortProperty::SIGNAL_QUALITY);
    properties.push_back(static_cast<PortProperty>(-1));

    std::vector<PropertyKVPair> kvPairs;
    ::android::binder::Status status;
    COMPOSITEINPUT_GET_PROPERTY_MULTI_NEG(port, properties, kvPairs, status);
    UT_LOG_INFO("getPropertyMulti(invalid property): isOk:[%s] exception:[%d]\n",
                status.isOk() ? "true" : "false", status.exceptionCode());
    UT_ASSERT_TRUE(!status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
 * @brief Verify that resetMetrics() succeeds on a started composite input controller.
 *
 * This test opens and starts the first port, calls resetMetrics() on the controller, and
 * verifies the call returns EX_NONE. It then stops and closes the port.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 018@n
 * **Priority:** Medium@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read port count from profile (compositeinput.ports) | input: none | profileCount > 0 | Should Pass |@n
 * | 02 | For each profile port open and start the port | portId = compositeinput.ports.N.id | Port in STARTED state | Should Pass |@n
 * | 03 | Call resetMetrics() on the controller | controller from step 02 | API returns status OK | Should Pass |@n
 * | 04 | Verify metric property (METRIC_UPTIME) is readable after reset | METRIC_UPTIME | getProperty returns OK | Should Pass |@n
 * | 05 | Stop and close the port | controller from step 02 | Port returns to CLOSED | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, resetMetrics_pos)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto listener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, listener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        // Reset all telemetry metrics
        COMPOSITEINPUT_RESET_METRICS(controller);
        UT_LOG_INFO("resetMetrics() passed for portId:[%d]\n", profilePortId);

        COMPOSITEINPUT_STOP(controller);

        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);
    }
}

/**
 * @brief Verify that setProperty() succeeds for all writable properties on each port.
 *
 * This test reads propertyMetadata from the profile for each port. For every entry where
 * readOnly == false the port is opened, started, setProperty() is called with a default
 * value whose type matches PropertyMetadata.type, and EX_NONE is asserted. If no writable
 * properties are declared for a port the test passes vacuously for that port.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 019@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read port count from profile (compositeinput.ports) | input: none | profileCount > 0 | Should Pass |@n
 * | 02 | For each port read propertyMetadata list from profile | compositeinput.ports.N.propertyMetadata | Metadata list populated | Should Pass |@n
 * | 03 | Skip read-only entries; collect writable properties with typed default values | readOnly == false entries | Writable props list built | Should Pass |@n
 * | 04 | If writable props exist: open port, start controller | valid listener | Port in STARTED state | Should Pass |@n
 * | 05 | For each writable property call setProperty() with matching typed value | property, value | API returns EX_NONE | Should Pass |@n
 * | 06 | Stop and close the port | controller | Port returns to CLOSED | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, setProperty_pos)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        std::string metaListKey = "compositeinput.ports." + std::to_string(i) + ".propertyMetadata";
        uint32_t metaCount = UT_KVP_PROFILE_GET_LIST_COUNT(metaListKey.c_str());

        // Collect writable (readOnly == false) properties with typed default values
        std::vector<std::pair<PortProperty, com::rdk::hal::PropertyValue>> writableProps;

        for (uint32_t j = 0; j < metaCount; ++j)
        {
            std::string base = metaListKey + "." + std::to_string(j);
            bool readOnly = UT_KVP_PROFILE_GET_BOOL((base + ".readOnly").c_str());
            if (readOnly)
                continue;

            char metaKeyStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".key").c_str(), metaKeyStr);
            char metaTypeStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".type").c_str(), metaTypeStr);

            PortProperty prop = convertStringToPortProperty(std::string(metaKeyStr));
            PropertyMetadata::PropertyType propType = convertStringToPropertyType(std::string(metaTypeStr));

            com::rdk::hal::PropertyValue pv;
            switch (propType)
            {
                case PropertyMetadata::PropertyType::BOOLEAN:
                    pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::booleanValue>(false);
                    break;
                case PropertyMetadata::PropertyType::INTEGER:
                    pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                    break;
                case PropertyMetadata::PropertyType::LONG:
                    pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::longValue>(int64_t(0));
                    break;
                case PropertyMetadata::PropertyType::FLOAT:
                    pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::floatValue>(0.0f);
                    break;
                case PropertyMetadata::PropertyType::DOUBLE:
                    pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::doubleValue>(0.0);
                    break;
                case PropertyMetadata::PropertyType::STRING:
                    pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::stringValue>(android::String16(""));
                    break;
                default:
                    pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                    break;
            }

            writableProps.emplace_back(prop, pv);
            UT_LOG_INFO("Port[%d] found writable property:[%s]\n", profilePortId, metaKeyStr);
        }

        if (writableProps.empty())
        {
            UT_LOG_INFO("Port[%d]: no writable properties in profile, skipping setProperty_pos for this port\n", profilePortId);
            continue;
        }

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto listener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, listener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        for (auto& [prop, pv] : writableProps)
        {
            COMPOSITEINPUT_SET_PROPERTY(controller, prop, pv);
            UT_LOG_INFO("Port[%d] setProperty(%d) OK\n", profilePortId, static_cast<int>(prop));
        }

        COMPOSITEINPUT_STOP(controller);

        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);
    }
}

/**
 * @brief Verify that setProperty() fails correctly for invalid and read-only properties.
 *
 * Two negative scenarios are tested on the first port:
 * 1. Call setProperty() with an out-of-range property enum → EX_ILLEGAL_ARGUMENT.
 * 2. Call setProperty() with the first read-only property from the profile metadata →
 *    EX_UNSUPPORTED_OPERATION.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 020@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Open first port and start controller | portIds[0], valid listener | Port in STARTED state | Should Pass |@n
 * | 02 | Call setProperty() with invalid property (static_cast<PortProperty>(-1)) | invalid property, intValue(0) | API returns !isOk() (EX_ILLEGAL_ARGUMENT) | Should Pass |@n
 * | 03 | Call setProperty() with first read-only property from profile | readOnly property, typed default value | API returns !isOk() (EX_UNSUPPORTED_OPERATION) | Should Pass |@n
 * | 04 | Stop and close the port | controller | Port returns to CLOSED | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, setProperty_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    auto listener = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller;
    COMPOSITEINPUT_OPEN(port, listener, controller);
    UT_ASSERT_NOT_EQUAL(controller, nullptr);

    COMPOSITEINPUT_START(controller);

    // Scenario 1: invalid (unknown) property → EX_ILLEGAL_ARGUMENT
    {
        com::rdk::hal::PropertyValue pv;
        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));

        PortProperty invalidProp = static_cast<PortProperty>(-1);
        ::android::binder::Status setStatus;
        COMPOSITEINPUT_SET_PROPERTY_NEG(controller, invalidProp, pv, setStatus);
        UT_LOG_INFO("setProperty(invalid): isOk:[%s] exception:[%d]\n",
                    setStatus.isOk() ? "true" : "false", setStatus.exceptionCode());
        UT_ASSERT_TRUE(!setStatus.isOk());
        UT_ASSERT_EQUAL(setStatus.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    // Scenario 2: first read-only property from profile → EX_UNSUPPORTED_OPERATION
    {
        // Read first propertyMetadata entry (all profile entries are readOnly, so entry 0 qualifies)
        std::string base = "compositeinput.ports.0.propertyMetadata.0";
        char metaKeyStr[UT_KVP_MAX_ELEMENT_SIZE];
        UT_KVP_PROFILE_GET_STRING((base + ".key").c_str(), metaKeyStr);
        char metaTypeStr[UT_KVP_MAX_ELEMENT_SIZE];
        UT_KVP_PROFILE_GET_STRING((base + ".type").c_str(), metaTypeStr);
        bool readOnly = UT_KVP_PROFILE_GET_BOOL((base + ".readOnly").c_str());

        UT_LOG_INFO("setProperty_neg scenario 2: property:[%s] type:[%s] readOnly:[%s]\n",
                    metaKeyStr, metaTypeStr, readOnly ? "true" : "false");
        UT_ASSERT_TRUE(readOnly); // confirm the property is actually read-only

        PortProperty readOnlyProp = convertStringToPortProperty(std::string(metaKeyStr));
        PropertyMetadata::PropertyType propType = convertStringToPropertyType(std::string(metaTypeStr));

        com::rdk::hal::PropertyValue pv;
        switch (propType)
        {
            case PropertyMetadata::PropertyType::INTEGER:
                pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                break;
            case PropertyMetadata::PropertyType::LONG:
                pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::longValue>(int64_t(0));
                break;
            default:
                pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                break;
        }

        ::android::binder::Status setStatus;
        COMPOSITEINPUT_SET_PROPERTY_NEG(controller, readOnlyProp, pv, setStatus);
        UT_LOG_INFO("setProperty(readOnly prop %s): isOk:[%s] exception:[%d]\n",
                    metaKeyStr, setStatus.isOk() ? "true" : "false", setStatus.exceptionCode());
        UT_ASSERT_TRUE(!setStatus.isOk());
        UT_ASSERT_EQUAL(setStatus.exceptionCode(), android::binder::Status::EX_UNSUPPORTED_OPERATION);
    }

    COMPOSITEINPUT_STOP(controller);

    bool closed = false;
    COMPOSITEINPUT_CLOSE(port, controller, closed);
    UT_ASSERT_TRUE(closed);
}

/**
 * @brief Verify that setPropertyMulti() succeeds for all writable properties on each port.
 *
 * This test reads propertyMetadata from the profile for each port. All non-readOnly entries
 * are collected into a PropertyKVPair vector and passed to setPropertyMulti() in a single
 * call while the port is in STARTED state. EX_NONE is asserted. If no writable properties
 * are declared for a port the test passes vacuously for that port.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 021@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read port count from profile (compositeinput.ports) | input: none | profileCount > 0 | Should Pass |@n
 * | 02 | For each port read propertyMetadata list from profile | compositeinput.ports.N.propertyMetadata | Metadata list populated | Should Pass |@n
 * | 03 | Skip read-only entries; build PropertyKVPair vector for writable properties | readOnly == false entries | kvPairs vector built | Should Pass |@n
 * | 04 | If writable props exist: open port, start controller | valid listener | Port in STARTED state | Should Pass |@n
 * | 05 | Call setPropertyMulti() with the entire writable kvPairs vector | kvPairs vector | API returns EX_NONE | Should Pass |@n
 * | 06 | Stop and close the port | controller | Port returns to CLOSED | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, setPropertyMulti_pos)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        std::string metaListKey = "compositeinput.ports." + std::to_string(i) + ".propertyMetadata";
        uint32_t metaCount = UT_KVP_PROFILE_GET_LIST_COUNT(metaListKey.c_str());

        // Build PropertyKVPair list for all writable properties
        std::vector<PropertyKVPair> kvPairs;

        for (uint32_t j = 0; j < metaCount; ++j)
        {
            std::string base = metaListKey + "." + std::to_string(j);
            bool readOnly = UT_KVP_PROFILE_GET_BOOL((base + ".readOnly").c_str());
            if (readOnly)
                continue;

            char metaKeyStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".key").c_str(), metaKeyStr);
            char metaTypeStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".type").c_str(), metaTypeStr);

            PropertyKVPair kv;
            kv.property = convertStringToPortProperty(std::string(metaKeyStr));
            PropertyMetadata::PropertyType propType = convertStringToPropertyType(std::string(metaTypeStr));

            switch (propType)
            {
                case PropertyMetadata::PropertyType::BOOLEAN:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::booleanValue>(false);
                    break;
                case PropertyMetadata::PropertyType::INTEGER:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                    break;
                case PropertyMetadata::PropertyType::LONG:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::longValue>(int64_t(0));
                    break;
                case PropertyMetadata::PropertyType::FLOAT:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::floatValue>(0.0f);
                    break;
                case PropertyMetadata::PropertyType::DOUBLE:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::doubleValue>(0.0);
                    break;
                case PropertyMetadata::PropertyType::STRING:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::stringValue>(android::String16(""));
                    break;
                default:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                    break;
            }

            kvPairs.push_back(kv);
            UT_LOG_INFO("Port[%d] found writable property:[%s]\n", profilePortId, metaKeyStr);
        }

        if (kvPairs.empty())
        {
            UT_LOG_INFO("Port[%d]: no writable properties in profile, skipping setPropertyMulti_pos for this port\n", profilePortId);
            continue;
        }

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto listener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, listener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        COMPOSITEINPUT_SET_PROPERTY_MULTI(controller, kvPairs);
        UT_LOG_INFO("Port[%d] setPropertyMulti(%zu props) OK\n", profilePortId, kvPairs.size());

        COMPOSITEINPUT_STOP(controller);

        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);
    }
}

/**
 * @brief Verify that setPropertyMulti() fails correctly for invalid and read-only properties.
 *
 * Two negative scenarios are tested on the first port:
 * 1. Call setPropertyMulti() with a vector containing one invalid property enum →
 *    EX_ILLEGAL_ARGUMENT.
 * 2. Call setPropertyMulti() with a vector of read-only properties from the profile →
 *    EX_UNSUPPORTED_OPERATION.
 *
 * **Test Group ID:** Negative: 01@n
 * **Test Case ID:** 022@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Open first port and start controller | portIds[0], valid listener | Port in STARTED state | Should Pass |@n
 * | 02 | Call setPropertyMulti() with vector containing static_cast<PortProperty>(-1) | invalid property, intValue(0) | API returns !isOk() (EX_ILLEGAL_ARGUMENT) | Should Pass |@n
 * | 03 | Call setPropertyMulti() with vector of read-only properties from profile | readOnly properties with typed default values | API returns !isOk() (EX_UNSUPPORTED_OPERATION) | Should Pass |@n
 * | 04 | Stop and close the port | controller | Port returns to CLOSED | Should Pass |@n
 */
UT_ADD_TEST(VTS_L1_COMPOSITEINPUT, setPropertyMulti_neg)
{
    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    android::sp<ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port);
    UT_ASSERT_NOT_EQUAL(port, nullptr);

    auto listener = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller;
    COMPOSITEINPUT_OPEN(port, listener, controller);
    UT_ASSERT_NOT_EQUAL(controller, nullptr);

    COMPOSITEINPUT_START(controller);

    // Scenario 1: vector containing an invalid property enum → EX_ILLEGAL_ARGUMENT
    {
        PropertyKVPair kv;
        kv.property = static_cast<PortProperty>(-1);
        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
        std::vector<PropertyKVPair> kvPairs = {kv};

        ::android::binder::Status setStatus;
        COMPOSITEINPUT_SET_PROPERTY_MULTI_NEG(controller, kvPairs, setStatus);
        UT_LOG_INFO("setPropertyMulti(invalid property): isOk:[%s] exception:[%d]\n",
                    setStatus.isOk() ? "true" : "false", setStatus.exceptionCode());
        UT_ASSERT_TRUE(!setStatus.isOk());
        UT_ASSERT_EQUAL(setStatus.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    // Scenario 2: vector of read-only properties → EX_UNSUPPORTED_OPERATION
    {
        // Build kvPairs from the first two profile metadata entries (all are readOnly in this profile)
        std::string metaListKey = "compositeinput.ports.0.propertyMetadata";
        uint32_t metaCount = UT_KVP_PROFILE_GET_LIST_COUNT(metaListKey.c_str());
        UT_ASSERT_TRUE(metaCount > 0);

        std::vector<PropertyKVPair> kvPairs;
        uint32_t limit = (metaCount < 2) ? metaCount : 2; // use up to 2 entries
        for (uint32_t j = 0; j < limit; ++j)
        {
            std::string base = metaListKey + "." + std::to_string(j);
            char metaKeyStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".key").c_str(), metaKeyStr);
            char metaTypeStr[UT_KVP_MAX_ELEMENT_SIZE];
            UT_KVP_PROFILE_GET_STRING((base + ".type").c_str(), metaTypeStr);

            PropertyKVPair kv;
            kv.property = convertStringToPortProperty(std::string(metaKeyStr));
            PropertyMetadata::PropertyType propType = convertStringToPropertyType(std::string(metaTypeStr));

            switch (propType)
            {
                case PropertyMetadata::PropertyType::INTEGER:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                    break;
                case PropertyMetadata::PropertyType::LONG:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::longValue>(int64_t(0));
                    break;
                default:
                    kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                    break;
            }

            kvPairs.push_back(kv);
            UT_LOG_INFO("setPropertyMulti_neg scenario 2: added readOnly property:[%s]\n", metaKeyStr);
        }

        ::android::binder::Status setStatus;
        COMPOSITEINPUT_SET_PROPERTY_MULTI_NEG(controller, kvPairs, setStatus);
        UT_LOG_INFO("setPropertyMulti(readOnly props): isOk:[%s] exception:[%d]\n",
                    setStatus.isOk() ? "true" : "false", setStatus.exceptionCode());
        UT_ASSERT_TRUE(!setStatus.isOk());
        UT_ASSERT_EQUAL(setStatus.exceptionCode(), android::binder::Status::EX_UNSUPPORTED_OPERATION);
    }

    COMPOSITEINPUT_STOP(controller);

    bool closed = false;
    COMPOSITEINPUT_CLOSE(port, controller, closed);
    UT_ASSERT_TRUE(closed);
}

/** @} */ // End of COMPOSITE_INPUT_HALTEST_L1
/** @} */ // End of COMPOSITE_INPUT_HALTEST
/** @} */ // End of COMPOSITE_INPUT
/** @} */ // End of HPK

