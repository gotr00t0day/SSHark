# SSHark 🦈

A powerful multithreaded SSH lateral movement tool designed for penetration testing and red team operations.

## 📋 Description

SSHark automates the process of discovering SSH private keys on a compromised system and testing them against other hosts in the network. It exploits a common security weakness: SSH key reuse across multiple systems.

When you compromise a Linux system, there are often SSH private keys in the `~/.ssh/` directory that might grant access to other systems on the network. SSHark automates this reconnaissance and lateral movement process with blazing speed using multithreading.

## ✨ Features

- 🔑 **Automatic SSH Key Discovery** - Finds RSA, DSA, ECDSA, and ED25519 keys
- 🌐 **Dual Scanning Modes** - Subnet scanning (--ip) or targeted hosts (--hostnames)
- ⚡ **Multithreaded Performance** - 50 parallel threads for subnet scanning
- 🔌 **Smart Port Detection** - Checks common SSH ports (22, 2222, 2200, 22000)
- 👤 **Username Enumeration** - Tests common usernames (root, admin, ubuntu, ec2-user, user)
- 🎨 **Color-Coded Output** - Real-time feedback with intuitive color scheme
- ⚙️ **Customizable** - Specify custom keys and ports via command-line options
- 🚀 **Fast** - Complete subnet scans in seconds instead of minutes

## 🔧 Installation

### Prerequisites

- C++20 compatible compiler (g++ 10+)
- POSIX-compliant system (Linux, macOS)
- Basic networking tools (ping, ifconfig, ssh)

### Compilation

```bash
# Clone or download SSHark
cd sshark

# Build using Makefile
make

# Binary will be in ./bin/sshark
```

### System-Wide Installation (Optional)

```bash
sudo make install
```

This installs SSHark to `/usr/local/bin/sshark` for system-wide access.

## 🚀 Usage

### Basic Commands

```bash
# Show help
./bin/sshark --help

# Scan local subnet for live hosts
./bin/sshark --ip

# Use hosts from /etc/hosts file
./bin/sshark --hostnames
```

### Advanced Usage

#### Custom SSH Keys
```bash
# Specify custom SSH key paths
./bin/sshark --ip --keys ~/.ssh/my_key,/tmp/stolen_key

# Use a single custom key
./bin/sshark --hostnames --keys /path/to/private_key
```

#### Custom Ports
```bash
# Scan specific SSH ports
./bin/sshark --ip --ports 22,8022,2222

# Combine custom keys and ports
./bin/sshark --ip --keys ~/.ssh/id_rsa --ports 22,2222
```

## 📖 Usage Examples

### Example 1: Quick Subnet Scan
```bash
./bin/sshark --ip
```
Automatically detects your local IP, scans the entire /24 subnet for live hosts, and attempts SSH authentication using all discovered keys.

### Example 2: Targeted Attack
```bash
./bin/sshark --hostnames --keys ~/.ssh/id_rsa
```
Tests a specific SSH key against hosts defined in `/etc/hosts`.

### Example 3: Custom Configuration
```bash
./bin/sshark --ip --keys /tmp/key1,/tmp/key2 --ports 22,2222,10022
```
Scans subnet using two specific keys against three custom SSH ports.

## 📊 Output Examples

### Successful Authentication
```
✓ AUTHENTICATED: root@10.0.0.120:22 with key /home/user/.ssh/id_rsa
```

### Progress Indicators
```
[*] Scanning subnet 10.0.0.0/24 for live hosts...
  Progress: 50/254 IPs scanned...
  [+] Found live host: 10.0.0.120
```


## 🐛 Troubleshooting

### No SSH Keys Found
```bash
# Check if keys exist
ls -la ~/.ssh/

# Specify custom key path
./bin/sshark --ip --keys /path/to/your/key
```

### No Hosts Discovered
```bash
# Verify network connectivity
ping -c 1 <gateway_ip>

# Check your IP address
ifconfig
```

### Permission Denied
```bash
# Ensure key has correct permissions
chmod 600 ~/.ssh/id_rsa

# Run with appropriate privileges if needed
sudo ./bin/sshark --ip
```

## 🎓 Learning Resources

- [SSH Key-Based Authentication](https://www.ssh.com/academy/ssh/key)
- [Lateral Movement Techniques](https://attack.mitre.org/tactics/TA0008/)
- [Post-Exploitation Guide](https://github.com/mubix/post-exploitation)


**Remember: With great power comes great responsibility. Use SSHark ethically and legally.**

