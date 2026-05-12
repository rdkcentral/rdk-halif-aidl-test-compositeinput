#!/bin/bash
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2026 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

RUN_DIR=$(realpath "$(dirname "$0")")
VCOMPONENT_SERVICE_NAME=Vendor-CompositeInput
VCOMPONENT_CONFIG_PATH=${RUN_DIR}/vcomponent_configurations/compositeinput_vcomponent_configuration.yaml

# Function to check if a process is running
check_process()
{
    local process_name=$1
    pgrep -f "$process_name" > /dev/null
    return $?
}

prompt_string()
{
    local prompt=$1
    local response
    read -p "$prompt : " response
    echo "$response"
}

start_vcomponent_service()
{
    echo "Starting ${VCOMPONENT_SERVICE_NAME} with config: ${VCOMPONENT_CONFIG_PATH}"
    echo "Vcomponent Service Config Path (Default: ${VCOMPONENT_CONFIG_PATH})"
    local user_config
    if [ -z "$user_config" ]; then
        user_config="$VCOMPONENT_CONFIG_PATH"
    fi
    ${RUN_DIR}/$VCOMPONENT_SERVICE_NAME -c "$user_config" &
    vcomponentservice_pid=$!
    # Wait for service to be ready
    sleep 10
}

if check_process $VCOMPONENT_SERVICE_NAME; then
    echo "${VCOMPONENT_SERVICE_NAME} is already running"
else
    start_vcomponent_service
fi

echo "Executing [vts_compositeinput_hal_test]"
"${RUN_DIR}/vts_compositeinput_hal_test" "$@"
