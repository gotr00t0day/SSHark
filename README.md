# SSHark 🦈

```
  ██████   ██████  ██░ ██  █████  ██████  ██   ██ 
▒██    ▒ ▒██    ▒ ▓██░ ██▒▒██   ▀ ▒██   ██▒██  ██ 
░ ▓██▄   ░ ▓██▄   ▒██▀▀██░░███   ░██  ████████   
  ▒   ██▒  ▒   ██▒░▓█ ░██ ░▓█▒  ░██  ░██ ▒██ ██  
▒██████▒▒▒██████▒▒░▓█▒░██▓░██████▒██  ░██ ▒██ ██  
```

**Version:** 1.0  
**Author:** c0d3Ninja

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

## ⚡ Performance

SSHark uses aggressive multithreading for maximum speed:

- **Subnet Scan:** 50 parallel threads
- **Port Scanning:** 4 parallel threads per host
- **SSH Testing:** 10 parallel authentication attempts

**Typical Performance:**
- Full /24 subnet scan: ~10 seconds
- 100 SSH authentication attempts: ~50 seconds
- Total operation: ~1-2 minutes for complete network mapping

## 🏗️ Architecture

```
SSHark
├── sshkeyspread.cpp       # Main application
└── modules/
    ├── parsers.cpp        # Config file parsing
    ├── readfile.cpp       # File operations
    ├── executils.cpp      # Command execution
    └── portscanner.cpp    # TCP port scanning
```

## 🛡️ Defense & Detection

### For Defenders

Protect against SSHark-style attacks:

1. **SSH Key Rotation** - Rotate keys regularly, especially privileged keys
2. **Unique Keys** - Use different SSH keys for each system
3. **SSH Certificates** - Implement SSH certificate authorities
4. **Monitor Authentication** - Alert on multiple rapid SSH attempts
5. **Network Segmentation** - Limit SSH access between systems
6. **Audit Logging** - Log all SSH authentications

### Detection Signatures

SSHark creates distinctive patterns:
- Multiple SSH connections from single source
- Rapid authentication attempts across multiple hosts
- Sequential port scanning on SSH ports
- High volume of authentication failures

## ⚠️ Legal Disclaimer

**FOR AUTHORIZED TESTING ONLY**

SSHark is designed for:
- Authorized penetration testing
- Security research
- Educational purposes
- Red team operations with explicit permission

**WARNING:** Unauthorized access to computer systems is illegal. Always obtain explicit written permission before testing any network or system you do not own.

The author assumes NO LIABILITY for misuse of this tool. Users are solely responsible for ensuring their activities comply with applicable laws and regulations.

## 📝 License

This tool is provided for educational and authorized testing purposes only.

## 🤝 Contributing

Contributions, issues, and feature requests are welcome!

## 👨‍💻 Author

**c0d3Ninja**

## 🔗 Use Cases

- **Penetration Testing** - Post-exploitation lateral movement
- **Red Team Ops** - Network foothold expansion
- **Security Audits** - Identify SSH key reuse vulnerabilities
- **Cloud Security** - Test AWS/Azure instance key reuse
- **Incident Response** - Assess breach scope

## 📚 Technical Details

### Scanning Process

1. **Key Discovery Phase**
   - Searches default SSH key locations
   - Supports custom key paths
   - Validates key existence

2. **Host Discovery Phase**
   - Subnet ping sweep (--ip mode)
   - Or /etc/hosts parsing (--hostnames mode)
   - Parallel host-up checks

3. **Port Scanning Phase**
   - Checks SSH ports on live hosts
   - Multithreaded port detection
   - Configurable port list

4. **Authentication Phase**
   - Tests all key/host/port/username combinations
   - Parallel SSH connection attempts
   - Real-time success reporting

### Threading Model

- **Subnet Scan:** 50 worker threads
- **Port Check:** 4 threads per host
- **SSH Auth:** 10 concurrent connections
- **Thread-Safe:** Mutex-protected output and data structures

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

## 📈 Roadmap

- [ ] IPv6 support
- [ ] SOCKS proxy support
- [ ] Output to JSON/CSV
- [ ] Custom username lists
- [ ] SSH config file parsing
- [ ] Distributed scanning mode
- [ ] Integration with Metasploit

## 🎓 Learning Resources

- [SSH Key-Based Authentication](https://www.ssh.com/academy/ssh/key)
- [Lateral Movement Techniques](https://attack.mitre.org/tactics/TA0008/)
- [Post-Exploitation Guide](https://github.com/mubix/post-exploitation)

## 🙏 Acknowledgments

Built with modern C++ and inspired by the need for efficient post-exploitation tools.

---

**Remember: With great power comes great responsibility. Use SSHark ethically and legally.**

🦈 Happy (Ethical) Hacking! 🦈

