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
 * @file vts_compositeinput_helper_functions.cpp
 */

#include "vts_compositeinput_helper_functions.h"

// Map table for boolean values
const ut_control_keyStringMapping_t bool_mapTable_compositeinput[] = {
    {"true",  1},
    {"false", 0},
    {nullptr, -1}
};

// Map table for binder exception codes
const ut_control_keyStringMapping_t statusCompositeInputException_mapTable[] = {
    {"EX_NONE",              0},
    {"EX_SECURITY",          1},
    {"EX_BAD_PARCELABLE",    2},
    {"EX_ILLEGAL_ARGUMENT",  3},
    {"EX_NULL_POINTER",      4},
    {"EX_ILLEGAL_STATE",     5},
    {"EX_NETWORK_MAIN_THREAD", 6},
    {"EX_UNSUPPORTED_OPERATION", 7},
    {"EX_SERVICE_SPECIFIC",  8},
    {"EX_PARCELABLE",        9},
    {"EX_TRANSACTION_FAILED", -129},
    {nullptr, -1}
};

// Map table for SignalStatus enum
const ut_control_keyStringMapping_t signalStatus_mapTable_compositeinput[] = {
    {"NO_SIGNAL",  0},
    {"UNSTABLE",   1},
    {"STABLE",     2},
    {nullptr, -1}
};

// Map table for PortProperty enum
const ut_control_keyStringMapping_t portProperty_mapTable_compositeinput[] = {
    {"SIGNAL_STRENGTH",             0},
    {"SIGNAL_QUALITY",              1},
    {"METRIC_SIGNAL_LOCK_TIME",     1000},
    {"METRIC_SIGNAL_DROPS",         1001},
    {"METRIC_UPTIME",               1002},
    {"METRIC_SIGNAL_LOCK_COUNT",    1003},
    {"METRIC_LAST_SIGNAL_LOCK_TIME",1004},
    {"METRIC_LAST_RESET_TIMESTAMP", 1005},
    {nullptr, -1}
};

std::string vecIntToString(std::vector<int32_t>& vals)
{
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vals.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << vals[i];
    }
    oss << "]";
    return oss.str();
}

compositeinput::SignalStatus convertStringToSignalStatus(const std::string& val)
{
    if (val == "NO_SIGNAL")  return compositeinput::SignalStatus::NO_SIGNAL;
    if (val == "UNSTABLE")   return compositeinput::SignalStatus::UNSTABLE;
    if (val == "STABLE")     return compositeinput::SignalStatus::STABLE;
    UT_LOG_WARNING("Unknown SignalStatus string: %s, defaulting to NO_SIGNAL\n", val.c_str());
    return compositeinput::SignalStatus::NO_SIGNAL;
}

compositeinput::PortProperty convertStringToPortProperty(const std::string& propertyStr)
{
    if (propertyStr == "SIGNAL_STRENGTH")             return compositeinput::PortProperty::SIGNAL_STRENGTH;
    if (propertyStr == "SIGNAL_QUALITY")              return compositeinput::PortProperty::SIGNAL_QUALITY;
    if (propertyStr == "METRIC_SIGNAL_LOCK_TIME")     return compositeinput::PortProperty::METRIC_SIGNAL_LOCK_TIME;
    if (propertyStr == "METRIC_SIGNAL_DROPS")         return compositeinput::PortProperty::METRIC_SIGNAL_DROPS;
    if (propertyStr == "METRIC_UPTIME")               return compositeinput::PortProperty::METRIC_UPTIME;
    if (propertyStr == "METRIC_SIGNAL_LOCK_COUNT")    return compositeinput::PortProperty::METRIC_SIGNAL_LOCK_COUNT;
    if (propertyStr == "METRIC_LAST_SIGNAL_LOCK_TIME") return compositeinput::PortProperty::METRIC_LAST_SIGNAL_LOCK_TIME;
    if (propertyStr == "METRIC_LAST_RESET_TIMESTAMP") return compositeinput::PortProperty::METRIC_LAST_RESET_TIMESTAMP;
    UT_LOG_WARNING("Unknown PortProperty string: %s\n", propertyStr.c_str());
    return compositeinput::PortProperty::SIGNAL_STRENGTH;
}

compositeinput::PropertyMetadata::PropertyType convertStringToPropertyType(const std::string& typeStr)
{
    if (typeStr == "BOOLEAN") return compositeinput::PropertyMetadata::PropertyType::BOOLEAN;
    if (typeStr == "INTEGER") return compositeinput::PropertyMetadata::PropertyType::INTEGER;
    if (typeStr == "LONG")    return compositeinput::PropertyMetadata::PropertyType::LONG;
    if (typeStr == "FLOAT")   return compositeinput::PropertyMetadata::PropertyType::FLOAT;
    if (typeStr == "DOUBLE")  return compositeinput::PropertyMetadata::PropertyType::DOUBLE;
    if (typeStr == "STRING")  return compositeinput::PropertyMetadata::PropertyType::STRING;
    UT_LOG_WARNING("Unknown PropertyType string: %s, defaulting to BOOLEAN\n", typeStr.c_str());
    return compositeinput::PropertyMetadata::PropertyType::BOOLEAN;
}
