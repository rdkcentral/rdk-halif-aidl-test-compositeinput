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
import threading

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from composite_controller.compositeController import CompositeController
from raft.framework.core.logModule import logModule
from vts_compositeinput_helper import compositeInputHelperClass, COMPOSITEINPUT_L3_TEST_SUITE_NAME

class VTS_COMPOSITEINPUT_L3_ConnectionTestAfterStart(compositeInputHelperClass):
    """
    Runs L3 ConnectionTestAfterStart test for composite input HAL.

    Validates that onConnectionChanged fires during OPENING and
    onSignalStatusChanged fires during STARTING. Then verifies that
    subsequent signal status events (driven by the vcomponent) are
    received correctly while the port is in STARTED state.

    connection_sequence items:
        False -> vcomponent sends NO_SIGNAL
        True  -> vcomponent sends UNSTABLE followed by STABLE
    """

    def __init__(self, log: logModule = None):
        """
        Initializes the L3 ConnectionTestAfterStart test.

        Args:
            log (logModule, optional): Log module instance. Defaults to None.
        """
        self.testName = "CompositeInput_L3_ConnectionTestAfterStart"
        qcId = "2"
        super().__init__(self.testName, qcId, log)

        self.compositeController = CompositeController(
            self.log, self.dut.rawConfig.get("dut").get("compositeController"))

    def signalThread(self):
        """
        Background thread that drives the signal status sequence for each port.

        Reads connection_sequence from the test config. For each item:
          - False -> sends NO_SIGNAL
          - True  -> sends UNSTABLE then STABLE (matching vcomponent two-event pattern)

        An initial delay allows the C++ test to initialize and start the port.
        """
        time.sleep(2)
        ports = self.test_config_data[self.moduleName][self.testLevel][self.testcase]['ports']
        for port in ports:
            port_id = port.get('id')
            connection_sequence = port.get('connection_sequence', [])
            for item in connection_sequence:
                self.compositeController.connectDevice(port_id, item)
                if item is False:
                    self.compositeController.setSignalStatus(port_id, "NO_SIGNAL")
                else:
                    self.compositeController.setSignalStatus(port_id, "UNSTABLE")
                    self.compositeController.setSignalStatus(port_id, "STABLE")
                time.sleep(1)

    def testFunction(self):
        """
        The main test function to run the L3 ConnectionTestAfterStart.

        Sets the initial connection and signal state for each configured port so
        that the C++ test observes them during OPENING and STARTING. Then starts
        a background thread to drive the signal sequence events while the port is
        in STARTED state.

        Returns:
            bool: True if the test passed, False otherwise.
        """
        self.testcase = "ConnectionTestAfterStart"
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        final_results = True

        # Update test configuration on target device from HFP profile
        self.updateConnectionTestConfiguration(self.testcase)

        # Prepare background thread for signal sequence events
        self.signalThreadObj = threading.Thread(target=self.signalThread)

        self.utMenu.start()

        # Set initial connection and signal state before the test selects the port
        ports = self.test_config_data[self.moduleName][self.testLevel][self.testcase]['ports']
        for port in ports:
            port_id = port.get('id')
            conn_state = port.get('connection_state', False)
            self.compositeController.connectDevice(port_id, conn_state)
            if conn_state:
                # Active source connected: signal must be STABLE so STARTING reports STABLE
                self.compositeController.setSignalStatus(port_id, "STABLE")
                time.sleep(1)

        self.signalThreadObj.start()

        self.log.stepStart(f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')
        output = self.utMenu.select(COMPOSITEINPUT_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)

        results = self.utMenu.collect_results(output, True)
        if results is None:
            final_results = False

        self.signalThreadObj.join()

        self.log.stepResult(results, f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')

        self.utMenu.stop()

        return final_results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_COMPOSITEINPUT_L3_ConnectionTestAfterStart(summaryLog)
    test.run(False)
