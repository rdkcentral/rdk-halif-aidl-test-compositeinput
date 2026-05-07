#!/usr/bin/env python3
#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2026 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

import os
import sys
import time

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from composite_controller.compositeController import CompositeController
from raft.framework.core.logModule import logModule
from vts_compositeinput_helper import compositeInputHelperClass, COMPOSITEINPUT_L3_TEST_SUITE_NAME

class VTS_COMPOSITEINPUT_L3_PropertyApplicationVerification(compositeInputHelperClass):
    """
    Runs L3 PropertyApplicationVerification test for composite input HAL.

    Validates that writable composite input port properties discovered from the
    HFP profile can be set via the controller and that onPropertyChanged()
    callbacks are delivered. The C++ test is fully automated — it reads the
    property metadata from the profile and drives all set/verify cycles.

    The host side only needs to ensure the port has an active source (connected
    + STABLE signal) before the test starts so the port can enter STARTED state.
    """

    def __init__(self, log: logModule = None):
        """
        Initializes the L3 PropertyApplicationVerification test.

        Args:
            log (logModule, optional): Log module instance. Defaults to None.
        """
        self.testName = "CompositeInput_L3_PropertyApplicationVerification"
        qcId = "4"
        super().__init__(self.testName, qcId, log)

        self.compositeController = CompositeController(
            self.log, self.dut.rawConfig.get("dut").get("compositeController"))

    def testFunction(self):
        """
        The main test function to run the L3 PropertyApplicationVerification.

        Sets the connected + STABLE signal state for each port so the port can
        enter STARTED state. The C++ test then iterates over all writable
        properties from the HFP profile, sets each one via the controller, and
        verifies the onPropertyChanged() callback and getProperty() read-back.

        Returns:
            bool: True if the test passed, False otherwise.
        """
        self.testcase = "PropertyApplicationVerification"
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        final_results = True

        # Load test config to get per-port parameters
        self.test_config_data = self.compositeInputReadYaml(self.l3TestConfigFile)

        self.utMenu.start()

        # Establish active source for each port before the test starts
        ports = self.test_config_data[self.moduleName][self.testLevel][self.testcase]['ports']
        for port in ports:
            port_id = port.get('id')
            self.compositeController.connectDevice(port_id, True)
            self.compositeController.setSignalStatus(port_id, "STABLE")
            time.sleep(1)

        self.log.stepStart(f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')
        output = self.utMenu.select(COMPOSITEINPUT_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)

        results = self.utMenu.collect_results(output, True)
        if results is None:
            final_results = False

        self.log.stepResult(results, f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')

        self.utMenu.stop()

        return final_results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_COMPOSITEINPUT_L3_PropertyApplicationVerification(summaryLog)
    test.run(False)
