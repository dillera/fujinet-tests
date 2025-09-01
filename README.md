# FujiNet-Tests

These are integration tests which exercise firmware functionality.

## Unitrix Test Suite

The `unitrix/` directory contains a Python-based test suite for exercising FujiNet functionality.

### Setup and Execution

To run the test suite, you first need to set up the Python virtual environment.

1.  **Navigate to the `unitrix` directory:**
    ```bash
    cd unitrix
    ```

2.  **Activate the virtual environment:**
    The virtual environment is included in the repository. To activate it, run:
    ```bash
    source venv/bin/activate
    ```

3.  **Install dependencies:**
    Install the required Python packages using the `requirements.txt` file.
    ```bash
    pip install -r requirements.txt
    ```

4.  **Run the script:**
    The `unitrix.py` script requires a serial port and a path to a test definition JSON file.
    ```bash
    python3 unitrix.py <serial_port> <tests.json>
    ```
    For example:
    ```bash
    python3 unitrix.py /dev/ttyUSB0 stanard-tests.json
    ```

## Remote Command Tests

The `remotecmd/` directory contains tests for remote command functionality. These tests are built into disk images for various target platforms.

### Building Disk Images

#### Apple II

To build the Apple II disk image:

1.  **Navigate to the `remotecmd` directory:**
    ```bash
    cd remotecmd
    ```

2.  **Build the project:**
    ```bash
    make
    ```

3.  **Create the disk image:**
    This will create a `.po` disk image file.
    ```bash
    make disk
    ```

4.  **Load the image:**
    Move the generated `.po` file to a location where your Apple II emulator or hardware can access and load it.

#### Atari

Building for Atari is currently a work in progress.

#### C64

Building for C64 is currently a work in progress.
