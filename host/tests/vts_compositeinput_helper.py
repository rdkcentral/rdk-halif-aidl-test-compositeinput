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
import tarfile
import yaml
import time

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils

COMPOSITEINPUT_TEST_MENU_CONFIGURATION_FILE = os.path.join(dir_path, "vts_compositeinput_testMenuConfiguration.yaml")
COMPOSITEINPUT_TESTCASES_CONFIGURATION_FILE = os.path.join(os.path.join(dir_path, "../configs"), "vts_compositeinput_testcase_configuration.yaml")
COMPOSITEINPUT_L3_TEST_CONFIG_FILE = os.path.join(os.path.join(dir_path, "../../test_configuration"), "vts_l3_CompositeInput.yaml")

COMPOSITEINPUT_L3_TEST_SUITE_NAME = "VTS_L3_COMPOSITEINPUT"

class compositeInputHelperClass(utHelperClass):

    def __init__(self, testName: str, qcId: str, log: logModule = None, hfpfile:str=None, testMenuConfigFile:str=None, moduleName:str=None, artifactDirPath:str=None):
        """
        Initializes the composite input test helper class.

        Args:
            testName (str): Name of the test case.
            qcId (str): QC identifier for the test case.
            log (logModule, optional): Log module instance. Defaults to None.
        """
        if moduleName is None:
            moduleName = "compositeinput"
        self.moduleName = moduleName
        self.testLevel = "L3"
        super().__init__(testName, qcId, log)

        self.log = log
        if log is None:
            self.log = logModule(self.__class__.__name__)
            self.log.setLevel(self.log.INFO)

        # Parse device configuration
        self.deviceTestSetup = self.cpe.get("test")
        self.targetWorkspace = os.path.join(self.cpe.get("target_directory"), self.moduleName)
        profileFileFromConfig = self.deviceTestSetup.get("profile")
        # The moduleHFPFile should be the file available in compositeinput/profiles path
        profileFileTmp = hfpfile or profileFileFromConfig
        self.moduleHFPFile = os.path.join(dir_path, profileFileTmp)

        if (self.moduleName == "VTS"):
            # profileOnTarget for unifiedVTS is the one available in profiles path of unifiedVTS repo
            profileOnTarget = os.path.join(self.targetWorkspace, "bin", "profiles")
            profileOnTarget = os.path.join(profileOnTarget, os.path.basename(profileFileFromConfig))
        else:
            # profileOnTarget for specificmodule is the one available in profiles path of test repo
            profileOnTarget = os.path.join(self.targetWorkspace, "profiles")
            profileOnTarget = os.path.join(profileOnTarget, os.path.basename(self.moduleHFPFile))
        profileOnTarget = profileOnTarget.replace("\\", "/")

        # Load test menu and test case configuration files
        self.l3TestConfigFile = COMPOSITEINPUT_L3_TEST_CONFIG_FILE
        if testMenuConfigFile is not None:
            # this if code part is for unifiedVTS
            compositeinputTestMenuConfigFile = testMenuConfigFile
        else:
            compositeinputTestMenuConfigFile = COMPOSITEINPUT_TEST_MENU_CONFIGURATION_FILE

        self.testMenuConfiguration = ConfigRead(compositeinputTestMenuConfigFile, self.moduleName)
        if (self.moduleName != "VTS"):
            self.testCaseConfiguration = ConfigRead(COMPOSITEINPUT_TESTCASES_CONFIGURATION_FILE, self.moduleName) # Only needed while running individual tests
        self.testMenuConfiguration.test.execute = self.testMenuConfiguration.test.execute + f" -p {profileOnTarget}"

        # Establish HAL session
        self.hal_session = self.dut.getConsoleSession("ssh_hal_test")

        # Initialize UT menu navigator and utilities
        self.utMenu = UTSuiteNavigatorClass(self.testMenuConfiguration, None, self.hal_session)
        self.utilities = utBaseUtils()
        self.waitTime = 500

        # Copy required artifacts to target device
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        for artifact in self.testMenuConfiguration.test.artifacts:
            if artifactDirPath is None:
                filesPath = os.path.join(dir_path, artifact)
            else:
                # this if code part is for unifiedVTS
                filesPath = os.path.join(artifactDirPath, artifact)
            self.hal_session.write("ls " + self.targetWorkspace + "/" + os.path.basename(artifact) + "\n")
            res = self.hal_session.read_until(self.hal_session.prompt, timeout=5)
            if "No such file or directory" not in res:
                continue  # File already exists on target, skip copying and continue with next artifact
            self.utilities.sftpCopy(self.hal_session, filesPath, self.targetWorkspace)

            if tarfile.is_tarfile(filesPath):
                target_tarfile = os.path.join(self.targetWorkspace, os.path.basename(artifact))
                target_tarfile = target_tarfile.replace('\\', '/')
                self.utilities.untar(self.hal_session, target_tarfile, self.targetWorkspace)

        if (self.moduleName == "VTS"):
            #unified VTS repo target workspace needs a /bin/ after module name
            self.targetWorkspace = os.path.join(self.targetWorkspace, "bin")
            self.targetWorkspace = self.targetWorkspace.replace("\\", "/")
        self.moduleName = "compositeinput" #change the modulename to interface name as its being used inside test files


    def compositeInputReadYaml(self, yaml_path: str) -> dict:
        """
        Reads a YAML file and returns its content as a dictionary.

        Args:
            yaml_path (str): Path to the YAML file.

        Returns:
            dict: Content of the YAML file as a dictionary.
        """
        with open(yaml_path, "r") as f:
            return yaml.safe_load(f)

    def compositeInputWriteYaml(self, yaml_path: str, data: dict):
        """
        Writes a dictionary to a YAML file.

        Args:
            yaml_path (str): Path to the YAML file.
            data (dict): Content to write to the YAML file.
        """
        with open(yaml_path, "w") as f:
            yaml.dump(data, f, sort_keys=False, default_flow_style=None)

    def updateConnectionTestConfiguration(self, testName: str):
        """
        Updates the connection test configuration for the specified test.

        Reads the L3 test config YAML, populates port entries from the HFP profile,
        and copies the updated config to the target device.

        Args:
            testName (str): Name of the test case to update.
        """
        profile_path = self.moduleHFPFile

        # Read profile and L3 test config files
        self.test_config_data = self.compositeInputReadYaml(self.l3TestConfigFile)
        profile_data = self.compositeInputReadYaml(profile_path)

        port_list = profile_data[self.moduleName]['ports']
        port_ids = [port['id'] for port in port_list]

        # Get the template from the first port entry
        ports_section = self.test_config_data[self.moduleName][self.testLevel][testName]['ports']
        template = ports_section[0].copy()
        template.pop('id', None)

        # Build new ports section with all port IDs from HFP profile
        new_ports = []
        for pid in port_ids:
            entry = {'id': pid}
            entry.update(template)
            new_ports.append(entry)

        self.test_config_data[self.moduleName][self.testLevel][testName]['ports'] = new_ports

        # Write back to YAML and copy to target device
        self.compositeInputWriteYaml(self.l3TestConfigFile, self.test_config_data)

        target_config_path = self.targetWorkspace + "/test_configuration/"
        self.utilities.sftpCopy(self.hal_session, self.l3TestConfigFile, target_config_path)


    def testEndFunction(self, powerOff=True):
        """
        Ends the test case.

        Args:
            powerOff (bool): Whether to power off the device. Defaults to True.
        """
        super().testEndFunction(powerOff)
