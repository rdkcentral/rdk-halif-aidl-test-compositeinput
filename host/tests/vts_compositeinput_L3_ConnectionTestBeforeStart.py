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
from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from vts_compositeinput_helper import compositeInputHelperClass, COMPOSITEINPUT_L3_TEST_SUITE_NAME

class VTS_COMPOSITEINPUT_L3_ConnectionTestBeforeStart(compositeInputHelperClass):
    """
    Runs L3 ConnectionTestBeforeStart test for composite input HAL.

    This test validates that composite input port connection callbacks are
    correctly delivered when connection state changes occur before the port
    is started.
    """

    def __init__(self, log: logModule = None, hfpfile:str=None, testMenuConfigFile=None, moduleName=None, artifactDirPath=None):
        """
        Initializes the L3 ConnectionTestBeforeStart test.

        Args:
            log (logModule, optional): Log module instance. Defaults to None.
        """
        self.testName = "CompositeInput_L3_ConnectionTestBeforeStart"
        qcId = "1"
        super().__init__(self.testName, qcId, log, hfpfile, testMenuConfigFile, moduleName, artifactDirPath)

        self.compositeController = CompositeController(
            self.log, self.dut.rawConfig.get("dut").get("compositeController"))

    def connectionThread(self):
        """
        Thread function to drive composite input connection sequence events.

        Reads the connection_sequence list from the test config for each port and
        sends connectDevice() events with a 1s delay between each event.
        An initial delay allows the C++ test to initialize before events arrive.
        """
        time.sleep(2)
        ports = self.test_config_data[self.moduleName][self.testLevel][self.testcase]['ports']
        for port in ports:
            port_id = port.get('id')
            connection_sequence = port.get('connection_sequence', [])
            for conn_state in connection_sequence:
                self.compositeController.connectDevice(port_id, conn_state)
                time.sleep(1)

    def testFunction(self):
        """
        The main test function to run the L3 ConnectionTestBeforeStart.

        Sets the initial connection state for each configured port, then starts
        a background thread to drive connection_sequence events while the C++
        test is running.

        Returns:
            bool: True if the test passed, False otherwise.
        """
        self.testcase = "ConnectionTestBeforeStart"
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        final_results = True
        # Update test configuration on target device from HFP profile
        self.updateConnectionTestConfiguration(self.testcase)

        # Start background thread to drive the connection_sequence events
        self.connectionThreadObj = threading.Thread(target=self.connectionThread)

        self.utMenu.start()

        # Set initial connection state for each port before the test starts
        ports = self.test_config_data[self.moduleName][self.testLevel][self.testcase]['ports']
        for port in ports:
            port_id = port.get('id')
            conn_state = port.get('connection_state', False)
            self.compositeController.connectDevice(port_id, conn_state)
            time.sleep(1)

        self.connectionThreadObj.start()

        self.log.stepStart(f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')
        output = self.utMenu.select(COMPOSITEINPUT_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)

        results = self.utMenu.collect_results(output, True)
        if results is None:
            final_results = False

        self.connectionThreadObj.join()

        self.log.stepResult(results, f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')

        self.utMenu.stop()

        return final_results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_COMPOSITEINPUT_L3_ConnectionTestBeforeStart(summaryLog)
    test.run(False)
