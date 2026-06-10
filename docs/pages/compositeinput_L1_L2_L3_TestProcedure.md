# Composite Input L1, L2, L3 Test Procedure

## Table of Contents
- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Build the VTS](#build-the-vts)
- [Run L1 L2 Tests Without Python](#run-l1-l2-tests-without-python)
- [Run Tests in Python Environment](#run-tests-in-python-environment)
  - [Update Configuration Files](#update-configuration-files)
  - [Install the Python Packages](#install-the-python-packages)
  - [Run the Tests](#run-the-tests)
- [L3 Test Cases](#l3-test-cases)
- [Notes](#notes)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `VTS`    - Vendor Test Suite
- `L1`     - Level 1 Testing
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `DUT`    - Device Under Test
- `RAFT`   - Rapid Automation Framework for Testing
- `YAML`   - YAML Ain't Markup Language
- `AIDL`   - Android Interface Definition Language

## Build the VTS

### Build Linux binder

The test suite depends on Linux binder with `AIDL` support.

For detailed instructions, refer to: [Linux Binder](https://github.com/rdkcentral/linux_binder_idl/tree/1.0.0)

```bash
git clone -b develop 'https://github.com/rdkcentral/linux_binder_idl.git'
cd linux_binder_idl
./build-linux-binder-aidl.sh 64
```

### Build VTS binaries

Clone and build the Composite Input test suite:

```bash
git clone git@github.com:rdkcentral/rdk-halif-aidl-test-compositeinput.git
cd rdk-halif-aidl-test-compositeinput
```

Set environment variables:

- `AIDL_BIN`: Path to the AIDL compiler.
- `LINUX_BINDER_OUT_DIR`: Path to Linux binder output directory.
- `UT_CORE_VERSION`: UT-Core version (optional; defaults to latest tag).
- `RDK_HALIF_AIDL_VERSION`: AIDL HAL spec version (optional; defaults to latest HEAD).

Example:

```bash
export UT_CORE_VERSION=5.0.0
export RDK_HALIF_AIDL_VERSION=0.15.0
export AIDL_BIN=/path/to/linux_binder_idl/out/aidl
export LINUX_BINDER_OUT_DIR=/path/to/linux_binder_idl/local/
```

Enable UT logs (optional):

```bash
export ENABLE_UT_INFO_LOGGING=1
```

Build:

```bash
./build.sh Target=<Target-name>

# Supported Targets: linux, arm
```

Build output:
- binaries under `build/bin`
- tarball `vts-CompositeInput.tar.gz` under `build/bin`

## Run L1 L2 Tests Without Python

### Transfer Tar File to Target Device

```bash
scp -P <port> build/bin/vts-CompositeInput.tar.gz root@target-device:/tmp/
```

### Extract on Target Device

```bash
ssh root@target-device
mkdir -p /tmp/compositeinput
tar -zxf /tmp/vts-CompositeInput.tar.gz -C /tmp/compositeinput
cd /tmp/compositeinput
```

### Run the VTS Test on Target Device

```bash
./run.sh -p profiles/hfp-compositeinput.yaml
```

## Run Tests in Python Environment

### Update Configuration Files

#### Rack Configuration File

Example: [vts_rack_configuration.yml](../../host/configs/vts_rack_configuration.yml)

Update console sessions for DUT operations and test execution.

| Console Session | Description |
|----------------|-------------|
| `default`      | Device status and basic operations |
| `ssh_hal_test` | Executes Composite Input HAL VTS binary |

```yaml
rackConfig:
  rack1:
    name: "rack1"
    slot1:
      name: "slot1"
      devices:
        - dut:
            ip: ""
            platform: "vDevice"
            consoles:
              - default:
                  type: "ssh"
                  port: 5522
                  username: "root"
                  ip: ""
                  password: ""
                  prompt: "~#"
              - ssh_hal_test:
                  type: "ssh"
                  port: 5522
                  username: "root"
                  ip: ""
                  password: ""
                  prompt: "~#"
            outbound:
              download_url: "tftp://tftp-server.com/rack1/slot1/"
              upload_url: "sftp://server-address/home/workspace/tftp/rack1/slot1/"
              upload_url_base_dir: "sftp://server-address/home/workspace/tftp/rack1/slot1"
              workspaceDirectory: "./logs/workspace"
            compositeController:
                type: virtual-composite-controller
                vendor: "vComponent"
                address: "127.0.0.1" # IP address
                username: "" # username
                password: '' # password
                port: 5522
                control_port: 8080
```

***compositeController:***
Configure this field according to your test platform. For physical hardware testing, use `manual-composite-controller` as `RAFT` doesn't support a composite input controller device. For virtual device testing, use `virtual-composite-controller`. For virtual device, provide the connection details of the device running vComponent.
- `control_port` - The port number where vComponent accepts command requests.

#### Device Configuration File

Example: [vts_device_configuration.yml](../../host/configs/vts_device_configuration.yml)

```yaml
deviceConfig:
  cpe1:
    platform: "vDevice"
    model: "VTV"
    soc_vendor: "intel"
    target_directory: "/tmp/"
    prompt: "~#"
    wait_time: 240
    test:
      profile: "../../profiles/hfp-compositeinput.yaml"
```

### Install the Python Packages

From `host/`:

```bash
./install.sh
source activate_venv.sh
./install.sh
```

### Run the Tests

#### L1 and L2

```bash
python host/tests/vts_compositeinput_L1_L2_tests.py \
  --config </PATH>/host/configs/vts_rack_configuration.yml \
  --deviceConfig </PATH>/host/configs/vts_device_configuration.yml
```

#### L3

```bash
python host/tests/<testcase>.py \
  --config </PATH>/host/configs/vts_rack_configuration.yml \
  --deviceConfig </PATH>/host/configs/vts_device_configuration.yml
```
##### Device Setup

###### vDevice

For vDevice, no external hardware needs to be connected to the setup.

###### Real Device

For real hardware, use a composite signal generator to drive connection and signal events. As `RAFT` doesn't support automation of composite input source devices, control the device manually.

## L3 Test Cases

Host-side implementation is in `host/tests` and covers:

1. **ConnectionTestBeforeStart**
   - Triggers DUT-side connection validation after `open` (OPENING phase).
   - Verifies that `onConnectionChanged` fires at least once before `start`.
   - Validates the initial connection state and subsequent HPD event sequence.

2. **ConnectionTestAfterStart**
   - Triggers DUT-side validation during and after `start` (STARTING phase).
   - Verifies that `onSignalStatusChanged` fires at least once during STARTING.
   - Validates connection and signal event sequences.

3. **VideoModeDetection**
   - Triggers DUT-side video mode detection after signal becomes STABLE.
   - Verifies that `onVideoModeChanged` fires with the expected `VideoResolution`.
   - Validates mode updates when the source switches resolution.

4. **VideoPlaybackVerification**
   - Verifies end-to-end video playback readiness (signal STABLE, valid `VideoResolution`).
   - Requires an active composite source connected to the DUT.

## Note

### Test Setup on vDevice

To validate automation on vDevice, create a password for the root user using the following command:

```bash
passwd root
```

Then, update the same password in the `vts_rack_configuration.yaml` file.

Currently, the Python `paramiko` library does not support empty passwords on vDevice, so setting a password is necessary.

### Run on Windows Environment

To run the Python environment tests in a Windows environment, follow these additional steps after checking out the repository and compiling the code:

1. Install the python environment and update the rack configuration files as in the Linux environment.

2. Move the entire project folder (including `host`) to the Windows machine.

3. Open the file located at `framework/plugins/ut_raft/interactive_shell.py`.

   - Comment out or remove the following import lines to avoid errors on Windows:
     ```python
     import pty
     import fcntl
     ```

4. After these changes, run the test cases using Python on the Windows system.
