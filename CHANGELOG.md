# Changelog

## [v1.1.1] - 2026-04-20

### PDF Steganography

**Object Stream Embedding:**
- Hide data in PDF files using `/ObjStm` object streams
- FlateDecode compression for efficient storage
- Incremental update structure for broad viewer compatibility
- Conservative capacity limit (~50% of carrier PDF size)
- Support for any file type with original filename preservation

**Packet Format:**
- FNAM flag preserves original filename for binary files
- ENCR flag enables automatic encryption detection on extraction
- Metadata encrypted together with payload when encryption enabled

**PDF Steganography UI (PDFStegDialog):**
- Dual-tab interface (Hide Data / Extract Data)
- Text or file input selection with capacity display
- Real-time capacity calculation and validation
- Auto-save for binary files with timestamp_ermis_filename format
- Theme-aware info button with feature documentation

### Distributed Steganography

**Wikipedia Pointer Maps:**
- Hide messages across Wikipedia articles using word position pointers
- No single carrier file - message reconstructed from JSON map
- 10 language support (English, German, French, Spanish, Italian, Portuguese, Dutch, Russian, Japanese, Chinese)
- Rate-limited API calls with automatic throttling

**Map Format:**
- JSON wrapper with language and pointers array
- Optional AES-256-CBC encryption of entire map
- Base64 encoding for encrypted maps
- Backward compatible with legacy array format

**Distributed Steganography UI (DistributedStegDialog):**
- Dual-tab interface (Build Pointer Map / Reconstruct Message)
- Real-time progress tracking for word mapping
- Copy-to-clipboard and save-to-file for generated maps
- Automatic encrypted map detection and decryption
- Input validation for message format

### UI Enhancements

**Theme-Aware Info Buttons:**
- Info button (ⓘ) in tab bar corner across all steganography dialogs
- Dark theme: blue tint with white text
- Light theme: gray tint with dark text
- Comprehensive tooltips with feature-specific documentation
- Warnings for coordination requirements in network steganography

### Bug Fixes

**Dark Theme Persistence:**
- Fixed theme toggle not saving state correctly on application restart
- Removed redundant setTheme() call in constructor
- Single action trigger now correctly updates both UI and settings

**File Dialog Improvements:**
- Fixed Linux file dialogs hiding binary/system files
- Removed (*.*) filter pattern that restricted MIME types
- Consistent use of Constants::appDirPath for starting directories

**PDF Extraction:**
- Fixed magic bytes detection in stream extraction
- Resolved double extraction from duplicate signal connection
- Corrected output path defaulting behavior

---

## [v1.1.0] - 2026-03-27

### Network Steganography (ERTP Protocol)

**Multi-Protocol Packet Embedding:**
- ICMP Echo/Reply steganography with raw socket support
- DNS query/response embedding over UDP
- Direct UDP packet hiding on configurable ports
- HTTP/TLS encrypted HTTPS tunnel steganography

**Reliable Data Transfer:**
- Session-based communication with 32-bit unique identifiers
- Sliding window protocol (32-64 packets per protocol)
- ACK/NACK acknowledgment system with automatic retransmission
- CRC32 checksum validation for data integrity
- Configurable timeouts (1000-2000ms) and retry limits (3-5 attempts)

**Security & Control:**
- Optional AES-256-CBC encryption with PBKDF2 key derivation (100,000 iterations)
- Random salt generation (16 bytes per session)
- IP filtering with whitelist/blacklist and CIDR notation support
- Configurable port binding and domain resolution
- Transfer progress tracking and cancellation support

### Text Steganography (Zero-Width Characters)

**Invisible Text Encoding:**
- Zero-width Unicode character embedding (U+200B, U+200C, U+200D)
- Binary-to-zero-width mapping: `0 = ZWSP, 1 = ZWNJ`
- 4-byte header for data size validation
- Capacity: ~1 bit per visible character in cover text

**Flexible Data Handling:**
- Hide plain text messages or any file type
- Smart text/binary detection on extraction
- Binary file warnings during file selection
- Automatic preservation of file data integrity

**Text Steganography UI (TextStegDialog):**
- Dual-tab interface (Hide Data / Extract Data)
- Cover text input with file loading capability
- Text or file input selection for secret data
- Real-time capacity calculation with percentage indicator
- Copy-to-clipboard and save-to-file functionality
- Extracted data display with automatic format detection
- Timestamp-based default filenames

### Multi-Layer Security

**Combined Steganography Modes:**
- Single layer: Text steg or Network steg independently
- Dual layer: Text steg embedded in ERTP packets
- Triple layer: Text steg + AES-256 + ERTP + optional TLS
- Maximum flexibility for different threat models

### Technical Improvements

**New Components:**
- `StegEngine`: AES-256-CBC encryption/decryption with PBKDF2
- `ICMPStegEngine`: ICMP Echo/Reply packet steganography
- `DNSStegEngine`: DNS-over-UDP embedding
- `UDPStegEngine`: Direct UDP packet hiding
- `HTTPStegEngine`: TLS/HTTPS secure tunnel
- `TextStegEngine`: Zero-width character encoding
- `TextStegDialog`: User interface for text steganography

**Performance:**
- Network payload: Up to 1400 bytes per packet
- Text capacity: ~12-625 bytes (100-5000 char cover text)
- Encryption: <100ms for typical message sizes
- No external dependencies for text steganography

### New Features
- Dark Theme
- Ermis user data folder shortcut creation
- Multiple network protocol support
-  Optional passphrase encryption across all methods
-  IP filtering and source validation
-  File transfer with progress tracking
-  Zero-width character steganography
-  Auto text/binary format detection
-  Multi-layer security composability

###  Enhanced Architecture
- Modular engine design (separate image, audio, text, network)
- Unified encryption interface across all engines
- Session-based communication with reliability guarantees
- Cross-platform network steganography support

---


## [v1.0.0] - 2026-03-12

###  Initial Release

Ermis is a cross-platform steganography tool that allows you to hide and extract secret data within digital media files.

####  Image Steganography
- Hide data in PNG, JPG, and BMP images using LSB (Least Significant Bit) techniques
- Live preview of original and modified images with side-by-side comparison
- Real-time capacity indicator showing available hiding space

####  Audio Steganography
- Support for WAV, MP3, FLAC, and OGG audio files
- Automatic conversion of non-WAV files using FFmpeg
- Built-in audio player with play/pause/stop and volume controls
- Metadata extraction for accurate capacity calculation

####  Data Handling
- Text input for hiding plain messages with UTF-8 encoding
- File input for hiding any file type with filename preservation
- Smart marker system (0x00 for text, length byte for files)
- Automatic detection of data type during extraction

####  Security Features
- Optional AES encryption with passphrase protection
- ENCR marker for automatic encrypted data detection
- Passphrase memory during current session
- Secure extraction with decryption prompts

####  User Interface
- Dual-tab interface for hiding and extracting data
- Drag & drop support for both images and audio files
- Clipboard integration for one-click text copying
- Image preview highlighting with file path display
- Smart directory fallbacks (Pictures → Images, Music → AppDir)

####  Technical Highlights
- Modular engine design with separate image and audio steganography
- FFmpeg integration for professional-grade audio conversion
- Multi-threaded file operations with progress feedback
- Automatic timestamped filenames for stego images and extracted data
- Cross-platform support (Linux, Windows, macOS)

---

