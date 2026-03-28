# ERTP - Ermis Reliable Transfer Protocol

## Overview

**ERTP** (Ermis Reliable Transfer Protocol) is a sophisticated steganographic application that embeds content within network packets across multiple protocol layers. It enables secure, covert data transfer between two peers by leveraging ICMP, DNS, UDP, and HTTP/TLS protocols as carrier mediums for hidden data payload transmission.

The protocol uses advanced steganography techniques to conceal data within legitimate network traffic, making it ideal for scenarios requiring covert communication channels. ERTP supports both **file transfers** and **plain text data** with **optional end-to-end encryption**.

---

## Features

 **Multi-Protocol Support**
- **ICMP** - Echo/Reply packet steganography
- **DNS** - Data embedded in DNS responses
- **UDP** - Raw UDP packet encoding
- **HTTP/TLS** - Encrypted HTTPS channel steganography

 **Flexible Data Transfer**
- **File Transfer**: Send complete files between peers
- **Plain Text/Binary Data**: Direct message transmission
- Both modes support identical protocol handlers

 **Advanced Encryption (Optional)**
- **AES-256-CBC** encryption with PBKDF2 key derivation
- Random salt generation (16 bytes) for each encryption session
- **100,000+ PBKDF2 iterations** for key hardening
- HMAC-SHA1 based key/IV generation
- Encryption is **completely optional** - send unencrypted or encrypted data
- Format: `[salt (16 bytes)][IV (16 bytes)][encrypted data]`

 **Transfer Features**
- File and raw data transfer support
- Sliding window protocol for reliable delivery
- Session-based communication with ACK/NACK control
- Automatic packet sequencing and reassembly
- Progress tracking (send/receive)
- Timeout and retry mechanisms

 **Advanced Capabilities**
- IP filtering (whitelist/blacklist mode)
- Domain name resolution
- Configurable port binding
- Transfer cancellation and reset functions
- **Built-in Network Testing Dialog** for easy protocol testing and debugging

---

## Architecture

### Core Components

#### 1. **StegEngine** (`stegengine.h`)
Cryptography engine providing AES-256-CBC encryption/decryption:

**Encryption Methods:**

```cpp
// Encrypt data with passphrase-based AES-256
QByteArray encryptData(const QByteArray &data, const QString &passphrase);

// Decrypt data with passphrase
QByteArray decryptData(const QByteArray &encryptedData, const QString &passphrase);
```

**Encryption Details:**
- Algorithm: AES-256-CBC (256-bit key, Cipher Block Chaining mode)
- Key Derivation: PBKDF2 with HMAC-SHA1
  - Key iterations: 100,000
  - IV iterations: 5,000
- Salt: 16 random bytes (generated per encryption)
- IV: 16 bytes (derived from passphrase + salt offset)
- Format: `[16-byte salt][16-byte IV][encrypted payload]`

When encryption is enabled, the complete packet (header + data) is encrypted.

#### 2. **ICMPStegEngine** (`icmpstegengine.h`)
ICMP protocol handler implementing packet steganography over ICMP Echo/Reply:

**Features:**
- Raw socket-based ICMP packet construction
- Checksum calculation for ICMP packets
- Session management with windowing (window size: 32)
- Max payload per packet: 1400 bytes per packet
- Timeout: 1000ms, Max retries: 3
- Optional encryption layer

**Public API:**
```cpp
bool sendData(const QString& targetIp, const QByteArray& data);
bool sendFile(const QString& targetIp, const QString& filePath);
bool startListening();
void stopListening();
void setEncryptionKey(const QString& passphrase);  // Enable encryption
void clearEncryption();                             // Disable encryption
```

#### 3. **DNSStegEngine** (`dnsstegengine.h`)
DNS-over-UDP steganography layer:

**Features:**
- DNS query/response packet embedding
- Custom DNS packet parsing and creation
- Larger window size (64) for better throughput
- Max payload per packet: 1400 bytes
- Timeout: 2000ms, Max retries: 5
- Optional encryption support

**Public API:**
```cpp
bool sendData(const QString& targetIp, const QByteArray& data, quint16 dnsPort);
bool sendFile(const QString& targetIpOrDomain, const QString& filePath, quint16 dnsPort);
bool startListening(quint16 udpport);
void setEncryptionKey(const QString& passphrase);
void clearEncryption();
```

#### 4. **UDPStegEngine** (`udpstegengine.h`)
Direct UDP packet steganography:

**Features:**
- UDP socket-based transmission
- Sliding window protocol (window size: 32)
- Max payload per packet: 1400 bytes
- Default port: 5353
- Timeout: 1000ms, Max retries: 3
- Optional encryption layer

**Public API:**
```cpp
bool sendData(const QString& targetIp, const QByteArray& data, quint16 port = 5353);
bool sendFile(const QString& targetIp, const QString& filePath, quint16 port = 5353);
bool startListening(quint16 port = 5353);
void setEncryptionKey(const QString& passphrase);
void clearEncryption();
```

#### 5. **HTTPStegEngine** (`httpstegengine.h`)
TLS/HTTPS steganography with encrypted channel:

**Features:**
- TLS/SSL socket encryption
- Self-signed certificate support
- HTTP/1.1 request-response embedding
- Large chunk size (64KB per chunk)
- Automatic domain-to-IP resolution
- Session-based communication
- Built-in TLS layer security

**Public API:**
```cpp
bool startListening(quint16 port);
bool sendFile(const QString& targetIpOrDomain, const QString& filePath, quint16 port);
bool sendData(const QString& targetIpOrDomain, const QByteArray& data, quint16 port);
void setEncryptionKey(const QString&);  // No-op, uses TLS only
void clearEncryption();                  // No-op, uses TLS only
```

#### 6. **Data Structures** (`ertp_structs.h`)

```cpp
struct PacketHeader {
    quint32 magic;      // "ERMI" magic number
    quint32 sessionId;  // Unique session identifier
    quint16 sequence;   // Packet sequence number
    quint16 total;      // Total packets in transfer
    quint32 checksum;   // CRC32 checksum
    quint16 dataSize;   // Payload data size
    char padding[2];    // Alignment padding
};

struct SendSession {
    QByteArray data;
    int totalPackets;
    QString targetIp;
    quint16 targetPort;
    QMap<int, bool> acked;              // ACK tracking
    QMap<int, QElapsedTimer> sentTime;  // Send timestamps
    QMap<int, int> retryCount;          // Retry counters
    QTimer* timeoutTimer;
};

struct ReceiveSession {
    QByteArray filename;
    int totalPackets;
    QMap<int, QByteArray> chunks;       // Reassembly buffer
    QTimer* completionTimer;
};
```

#### 7. **IP Filtering** (`ipfilter.h`)
Advanced packet source filtering:

**Features:**
- Whitelist/Blacklist modes
- CIDR notation support for subnet filtering
- Domain name resolution
- IP address validation

**Modes:**
- **BlockMode**: Reject packets from listed IPs/domains
- **AllowMode**: Accept only from listed IPs/domains

#### 8. **Configuration** (`constants.h`)
Global application constants and settings:

```cpp
namespace Constants {
    enum Protocol { ICMP, UDP, DNS, HTTP };
    extern Protocol currentProtocol;
    extern quint16 bindPort;
    extern bool ipFilterEnabled;
    extern int ipFilterMode;        // 0 = Block, 1 = Allow
    extern QStringList ipFilterEntries;
};
```

---

## Network Testing Dialog

ERTP includes a **built-in Network Testing Dialog** that is packaged with the application. This dialog provides a user-friendly interface for:

- **Protocol Selection**: Choose between ICMP, DNS, UDP, and HTTP/TLS transports
- **Send/Receive Testing**: Quickly test data and file transfers
- **Encryption Configuration**: Set optional encryption passphrases
- **IP Filtering**: Configure whitelist/blacklist rules
- **Progress Monitoring**: View real-time transfer progress
- **Error Diagnostics**: Display detailed error messages and logs

The Network Testing Dialog is ideal for:
- Protocol validation and debugging
- Quick functionality verification
- Testing different encryption configurations
- Validating IP filter rules
- Development and troubleshooting

Simply launch the application to access the Network Testing Dialog from the main interface.

---

## ⚠️ CRITICAL: Sender Listening Requirement

**IMPORTANT**: The sender MUST start listening on the same protocol and port that it is sending to, or the transfer will **NOT work**.

### Why This is Required

ERTP uses a bidirectional communication model:
1. **Sender** → Sends packets to receiver
2. **Receiver** → Sends ACK/NACK responses back to sender
3. **Sender** → Must listen to receive acknowledgments

Without the sender actively listening, it cannot:
- Receive ACK packets from the receiver
- Know if packets were successfully received
- Retransmit failed packets
- Complete the transfer

### Critical Implementation Pattern

**ALWAYS follow this pattern - Sender MUST listen BEFORE sending:**

```cpp
// SENDER MUST LISTEN on the same protocol/port
UDPStegEngine sender;
sender.setEncryptionKey("SharedSecret");      // Optional encryption
sender.startListening(5353);                  // ⚠️ CRITICAL: Start listening FIRST
sender.sendData("192.168.1.100", QByteArray("Message"), 5353);

// Similarly for other protocols
ICMPStegEngine icmpSender;
icmpSender.startListening();                  // ⚠️ CRITICAL: Start listening
icmpSender.sendFile("192.168.1.100", "/path/to/file");

DNSStegEngine dnsSender;
dnsSender.startListening(53);                 // ⚠️ CRITICAL: Start listening
dnsSender.sendData("8.8.8.8", QByteArray("data"), 53);

HTTPStegEngine httpsSender;
httpsSender.startListening(8443);             // ⚠️ CRITICAL: Start listening
httpsSender.sendFile("example.com", "/path/to/file", 443);
```

### Verification Checklist

Before sending, verify:
- [ ] `startListening()` was called on the sender
- [ ] Listening port matches sending port (if applicable)
- [ ] Listening is active before `sendData()` / `sendFile()` is called
- [ ] Both sender and receiver use the same encryption key (if encryption enabled)
- [ ] No firewall rules block bidirectional traffic on the protocol/port

---

## Encryption Specification

### Encryption Algorithm

ERTP uses **AES-256-CBC** for symmetric encryption with PBKDF2 key derivation:

| Parameter | Value | Notes |
|-----------|-------|-------|
| Algorithm | AES-256-CBC | 256-bit key, Cipher Block Chaining |
| Key Size | 256 bits (32 bytes) | Derived from passphrase |
| IV Size | 128 bits (16 bytes) | Derived from passphrase + salt |
| Salt Size | 128 bits (16 bytes) | Randomly generated per encryption |
| KDF | PBKDF2-HMAC-SHA1 | Standard key derivation function |
| Key Iterations | 100,000 | OWASP recommended minimum |
| IV Iterations | 5,000 | Fewer iterations for IV generation |

### Encryption Workflow

**Encryption (Sender):**
1. Generate 16-byte random salt
2. Derive 256-bit key from passphrase + salt (PBKDF2, 100k iterations)
3. Derive 128-bit IV from passphrase + salt offset (PBKDF2, 5k iterations)
4. Encrypt plaintext with AES-256-CBC
5. Return: `[salt][IV][ciphertext]`

**Decryption (Receiver):**
1. Extract salt (first 16 bytes)
2. Extract IV (bytes 16-32)
3. Extract ciphertext (remaining bytes)
4. Derive key from passphrase + salt (100k iterations)
5. Decrypt ciphertext with AES-256-CBC
6. Return: plaintext (or empty if wrong passphrase)

### Optional Encryption

Encryption is **completely optional**. Choose your security model:

- **No Encryption** (fast): Raw data embedded in packets
- **With Encryption** (secure): Passphrase-protected data
- **TLS Transport** (HTTP): Always encrypted at transport layer

---

## Protocol Specification

### Packet Structure

All ERTP packets follow this header format:

| Field | Size | Description |
|-------|------|-------------|
| Magic | 4 bytes | "ERMI" identifier |
| Session ID | 4 bytes | Unique session identifier |
| Sequence | 2 bytes | Packet sequence number |
| Total | 2 bytes | Total packets in session |
| Checksum | 4 bytes | CRC32 data validation |
| Data Size | 2 bytes | Payload length |
| Padding | 2 bytes | Alignment bytes |
| **Payload** | Variable | Embedded data (optionally encrypted) |

### Data Transfer Modes

**File Transfer:**
- Send complete file with metadata
- Receiver reassembles chunks and saves to disk
- Progress tracking throughout transfer

**Plain Text/Binary Data:**
- Send arbitrary byte arrays
- Useful for messages, commands, or custom formats
- Same reliability guarantees as file transfers

### Communication Flow

1. **Initialization**: Sender initiates session with target IP/port
2. **Windowed Transmission**: Packets sent using sliding window protocol
3. **Acknowledgment**: Receiver sends ACK/NACK for each packet window
4. **Retransmission**: Failed packets retransmitted up to MAX_RETRIES
5. **Completion**: DONE packet signals transfer completion
6. **Assembly**: Receiver reassembles chunks into original data

### Session Management

- **Session ID**: Unique 32-bit identifier per transfer
- **Window Size**: Protocol-dependent (32-64 packets)
- **Timeout**: Protocol-dependent (1000-2000ms)
- **Retries**: 3-5 attempts before failure

---

## Supported Transports

### ICMP (Internet Control Message Protocol)
- **Uses**: Echo Request/Reply packets
- **Advantages**: Often not filtered, universal firewall bypass
- **Limitations**: May trigger IDS/monitoring alerts
- **Typical Use**: Network diagnostics cover
- **Encryption**: Optional (via passphrase)

### DNS (Domain Name System)
- **Uses**: DNS response packets
- **Advantages**: Appears as legitimate DNS traffic
- **Limitations**: DNS responses filtered in some networks
- **Typical Use**: DNS traffic masquerading
- **Encryption**: Optional (via passphrase)

### UDP (User Datagram Protocol)
- **Uses**: Raw UDP datagrams
- **Advantages**: Lower overhead, faster transmission
- **Limitations**: No guaranteed delivery at protocol level (handled by ERTP)
- **Typical Use**: Speed-optimized transfers
- **Encryption**: Optional (via passphrase)

### HTTP/TLS (HyperText Transfer Protocol Secure)
- **Uses**: HTTPS encrypted tunnel
- **Advantages**: Indistinguishable from normal web traffic, encrypted by transport
- **Limitations**: Requires certificate, slight overhead
- **Typical Use**: Most secure, production-grade transfers
- **Encryption**: Always TLS (optional additional encryption)

---

## Building & Dependencies

### Requirements
- **Qt Framework** (Qt5+)
- **OpenSSL** (libssl-dev) - For AES-256 and PBKDF2
- **CMake** (build system)
- **C++11** or later

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

---

## Usage Examples

### Example 1: Send Plain Text with Encryption (UDP)

```cpp
UDPStegEngine udp;
udp.setEncryptionKey("MySecretPassphrase");  // Enable encryption
udp.startListening(5353);                     // ⚠️ CRITICAL: Sender must listen
udp.sendData("192.168.1.100", QByteArray("Hello, this is a secret message!"), 5353);

// Signals
connect(&udp, &UDPStegEngine::dataReceived, this, [](const QByteArray& data, const QString& source) {
    qDebug() << "Received:" << QString(data) << "from" << source;
});
```

### Example 2: Send File without Encryption (ICMP)

```cpp
ICMPStegEngine icmp;
icmp.startListening();                        // ⚠️ CRITICAL: Sender must listen
// No encryption key set - data sent in plaintext
icmp.sendFile("192.168.1.100", "/path/to/document.pdf");

connect(&icmp, &ICMPStegEngine::fileReceived, this, [](const QString& path, const QString& source) {
    qDebug() << "File saved to:" << path << "from" << source;
});
```

### Example 3: Encrypted DNS Transfer

```cpp
DNSStegEngine dns;
dns.setEncryptionKey("MySecurePassword123");
dns.startListening(53);                       // ⚠️ CRITICAL: Sender must listen
dns.sendData("8.8.8.8", QByteArray("Encrypted data over DNS"), 53);

connect(&dns, &DNSStegEngine::dataReceived, this, [](const QByteArray& data, const QString& source) {
    qDebug() << "Received decrypted data:" << QString(data);
});
```

### Example 4: HTTPS Secure Transfer (Built-in TLS)

```cpp
HTTPStegEngine https;
https.startListening(8443);                   // ⚠️ CRITICAL: Sender must listen
https.sendFile("example.com", "/path/to/confidential.docx", 443);
// TLS encryption is automatic and always enabled

connect(&https, &HTTPStegEngine::fileReceived, this, [](const QString& path, const QString& source) {
    qDebug() << "Securely received file:" << path;
});
```

### Example 5: Receiver with IP Filtering and Encryption

```cpp
UDPStegEngine udp;
udp.setEncryptionKey("SharedSecret");

// Only accept from trusted IPs
IpFilter ipFilter;
ipFilter.setEnabled(true);
ipFilter.setMode(IpFilter::AllowMode);
ipFilter.setEntries({"192.168.1.100", "10.0.0.0/8"});

udp.startListening(5353);

connect(&udp, &UDPStegEngine::dataReceived, this, [&ipFilter](const QByteArray& data, const QString& source) {
    if (ipFilter.isAllowed(source)) {
        qDebug() << "Trusted sender:" << source << "Data:" << QString(data);
    }
});
```

### Example 6: CLI Sender-Receiver (Bidirectional with Listening)

```cpp
#include <QCoreApplication>
#include "udpstegengine.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Parse command line arguments
    if (argc < 4) {
        std::cerr << "Usage: ertp <role> <target-ip> <message> [port] [passphrase]" << std::endl;
        std::cerr << "Example: ertp send 192.168.1.100 \"Hello World\" 5353 mysecret" << std::endl;
        return 1;
    }

    QString role = argv[1];
    QString targetIp = argv[2];
    QString message = argv[3];
    quint16 port = (argc > 4) ? QString(argv[4]).toUShort() : 5353;
    QString passphrase = (argc > 5) ? argv[5] : "";

    UDPStegEngine engine;

    if (!passphrase.isEmpty()) {
        engine.setEncryptionKey(passphrase);
        std::cout << "Encryption enabled with passphrase: " << passphrase.toStdString() << std::endl;
    }

    // ⚠️ CRITICAL: Sender MUST listen on the same port it sends to
    engine.startListening(port);
    std::cout << "Listening on port " << port << std::endl;

    // Connect signals for feedback
    connect(&engine, &UDPStegEngine::dataReceived, this, [](const QByteArray& data, const QString& source) {
        std::cout << "Received from " << source.toStdString() << ": " << QString(data).toStdString() << std::endl;
    });

    connect(&engine, &UDPStegEngine::error, this, [](const QString& errorMsg) {
        std::cerr << "Error: " << errorMsg.toStdString() << std::endl;
    });

    connect(&engine, &UDPStegEngine::sendProgress, this, [](int packet, int total) {
        std::cout << "Sending... [" << packet << "/" << total << "]" << std::endl;
    });

    if (role == "send") {
        std::cout << "Sending to " << targetIp.toStdString() << ":" << port << std::endl;
        std::cout << "Message: " << message.toStdString() << std::endl;
        engine.sendData(targetIp, message.toUtf8(), port);
    } else if (role == "listen") {
        std::cout << "Listening on port " << port << std::endl;
        std::cout << "Waiting for incoming data..." << std::endl;
    } else {
        std::cerr << "Invalid role. Use 'send' or 'listen'" << std::endl;
        return 1;
    }

    return app.exec();
}
```

---

## Security Considerations

⚠️ **Warning**: ERTP is designed for research and educational purposes.

**Security Notes:**
- Encryption is optional; use passphrases for sensitive data
- TLS provides transport-layer security; ICMP/DNS/UDP do not (use optional encryption)
- IP filtering can be bypassed with spoofed packets
- Session IDs should be treated as public; rely on encryption for confidentiality
- Monitor IDS/network security appliances for anomalies
- Never hardcode passphrases; use secure configuration management
- PBKDF2 with 100k iterations may be slow on resource-constrained devices
- For production use, consider additional authentication (digital signatures)

**Best Practices:**
1. Always use encryption for sensitive data
2. Combine with TLS transport for maximum security (HTTP/TLS)
3. Use strong passphrases (16+ characters, mixed case/numbers/symbols)
4. Implement additional authentication and integrity checking
5. Log all transfers for audit purposes
6. Regularly rotate encryption keys and passphrases

---

## Signals & Slots

All engines emit standard signals:

```cpp
void dataReceived(const QByteArray& data, const QString& sourceIp);
void fileReceived(const QString& filePath, const QString& sourceIp);
void sendProgress(int packet, int total);
void receiveProgress(int packet, int total);
void error(const QString& message);
```

---

## Contributing

Contributions welcome. Please ensure code follows Qt/C++ conventions and includes appropriate error handling.

---

## Support & Documentation

- **Issues**: Report via GitHub Issues
- **Architecture**: See class headers for detailed API documentation
- **Protocol Details**: Review `ertp_structs.h` for packet format specification
- **Encryption**: See `stegengine.cpp` for AES-256-CBC implementation details
- **Network Testing**: Use the built-in Network Testing Dialog for quick validation

---

## Authors

**@alamahant** - ERTP Protocol Design & Implementation

---

## License

ERTP is licensed under the **GNU General Public License v3.0** (GPL-3).

**Copyright © 2026 Alamahant**

This means:
- ✅ Free to use, modify, and distribute
- ✅ You must provide source code to recipients
- ✅ Any derivative work must also use GPL-3
- ✅ No warranty provided (see LICENSE file)

See the [LICENSE](LICENSE) file for full details.

### Dependency Licenses

- **Qt Framework**: LGPL-3.0 (compatible with GPL-3)
- **OpenSSL**: Apache 2.0 / SSL License (compatible with GPL-3)
- **Kernel Network Stack**: System library exception applies

For commercial use or proprietary licensing, contact [alamahant@gmail.com].

---

**ERTP v1.0** - Secure Steganographic Packet Transfer across ICMP, DNS, UDP, and HTTPS