# ROS2 Lift Controller - Docker Development Environment

This guide provides instructions for setting up a Docker-based development environment for the ROS2 Lift Controller on a Raspberry Pi 5, with development done remotely via SSH.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Initial Setup](#initial-setup)
- [Development Container Setup](#development-container-setup)
- [Remote Development via SSH](#remote-development-via-ssh)
- [Building and Running](#building-and-running)
- [Development Workflow](#development-workflow)
- [Production Deployment](#production-deployment)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites
### On Raspberry Pi 5

1. **Install Docker:**
   ```bash
   curl -fsSL https://get.docker.com -o get-docker.sh
   sudo sh get-docker.sh
   sudo usermod -aG docker $USER
   ```
   **Important:** Log out and back in for group changes to take effect.

2. **Install Docker Compose:**
   ```bash
   sudo apt-get update
   sudo apt-get install -y docker-compose-plugin
   ```

3. **Enable GPIO Access:**
   ```bash
   sudo usermod -aG gpio $USER
   ```

4. **Verify Docker Installation:**
   ```bash
   docker --version
   docker compose version
   ```

---

## Initial Setup

### 1. Clone or Copy Your Project to Raspberry Pi

```bash
# SSH into your Raspberry Pi
ssh pi@<raspberry-pi-ip>

# Create workspace
mkdir -p ~/lift_ws
cd ~/lift_ws
```

### 2. Project Structure

Ensure your project has this structure:
```
lift_ws/
├── .devcontainer/
│   ├── devcontainer.json
│   ├── Dockerfile
│   └── docker-compose.yml
├── src/
│   └── lift_control/
│       ├── CMakeLists.txt
│       ├── package.xml
│       ├── action/
│       │   └── Command.action
│       └── src/
│           ├── lift_rpi.cpp
│           ├── lift_state.cpp
│           └── lift_command.cpp
├── Dockerfile                    # Production Dockerfile
├── docker-compose.yml            # Production docker-compose
├── docker-entrypoint.sh
└── README.md                     # This file
```

### 3. Create Dev Container Configuration

Create the `.devcontainer` directory and files:

```bash
cd ~/lift_ws
mkdir -p .devcontainer
```

Copy the three dev container files into `.devcontainer/`:
- `devcontainer.json`
- `Dockerfile` (development version)
- `docker-compose.yml` (development version)

---

## Development Container Setup

The dev container includes:
- **ROS2 Humble** - Full ROS2 environment
- **WiringPi 3.x** - Built from source for Raspberry Pi 5 support
- **Development Tools** - GDB, Valgrind, clang-format, editors
- **VS Code Extensions** - C/C++, Python, ROS, CMake tools
- **ROS_DOMAIN_ID=30** - Pre-configured for network isolation

### Dev Container Features

| Feature | Description |
|---------|-------------|
| **Live Code Sync** | Changes reflect immediately without rebuild |
| **Full GPIO Access** | Direct access to Raspberry Pi GPIO pins |
| **Debugging Tools** | GDB, Valgrind for memory debugging |
| **Code Formatting** | clang-format, clang-tidy, Python linters |
| **Bash History** | Persisted across container restarts |
| **Host Networking** | ROS2 nodes visible on network |

---

## Remote Development via SSH

### Step 1: Connect to Raspberry Pi via SSH in VS Code

1. **Open VS Code on your local machine**

2. **Press `F1` or `Ctrl+Shift+P`** to open command palette

3. **Type and select:** `Remote-SSH: Connect to Host...`

4. **Enter your Raspberry Pi connection:**
   ```
   pi@<raspberry-pi-ip>
   ```
   Or if you have SSH config:
   ```
   your-pi-hostname
   ```

5. **Enter your password** when prompted

6. **Wait for VS Code to connect** - A new VS Code window will open

### Step 2: Open the Workspace

1. **In the remote VS Code window:**
   - Click "Open Folder"
   - Navigate to `/home/pi/lift_ws`
   - Click "OK"

### Step 3: Reopen in Dev Container

1. **VS Code should detect the `.devcontainer` folder** and show a notification:
   > "Folder contains a Dev Container configuration file. Reopen folder to develop in a container"

2. **Click "Reopen in Container"**
   
   Or manually:
   - Press `F1` or `Ctrl+Shift+P`
   - Type: `Dev Containers: Reopen in Container`
   - Press Enter

3. **Wait for the container to build** (first time takes 10-15 minutes)
   - You can click "Show Log" to see build progress
   - The container is built on the Raspberry Pi

4. **Once complete**, you're now developing inside the container!

### Step 4: Verify Setup

Open a terminal in VS Code (`Ctrl+` ` or Terminal → New Terminal):

```bash
# Check ROS2
ros2 --version
# Output: ros2 doctor 0.10.3

# Check ROS_DOMAIN_ID
echo $ROS_DOMAIN_ID
# Output: 30

# Check WiringPi
gpio -v
# Output: gpio version: 3.16_arm64

# Check GPIO access
gpio readall
# Should show GPIO pin layout

# Check workspace
pwd
# Output: /root/lift_ws
```

---

## Building and Running

### Build the Workspace

```bash
# Navigate to workspace
cd /root/lift_ws

# Build (using alias)
cb

# Or full command
colcon build --symlink-install

# Source the workspace
source install/setup.bash
# Or use alias
source_ws
```

### Run the Lift Controller Node

**Terminal 1 - Run the node:**
```bash
ros2 run lift_control lift_rpi
```

**Terminal 2 - Monitor status:**
```bash
ros2 topic echo /lift_state
```

**Terminal 3 - Send commands:**
```bash
# Move up
ros2 action send_goal /lift_action lift_control/action/Command "{command: 1}"

# Move down
ros2 action send_goal /lift_action lift_control/action/Command "{command: 2}"

# Stop
ros2 action send_goal /lift_action lift_control/action/Command "{command: 0}"
```

### Using Multiple Terminals in VS Code

You can split terminals for easier workflow:
1. Open terminal (`Ctrl+` `)
2. Click the split icon (⊞) in the terminal panel
3. Or use `Ctrl+Shift+5` to split terminal

---