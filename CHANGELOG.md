# Changelog

## [v1.0.0] - 2026-03-12

### 🎉 Initial Release

Ermis is a cross-platform steganography tool that allows you to hide and extract secret data within digital media files.

#### 🖼️ Image Steganography
- Hide data in PNG, JPG, and BMP images using LSB (Least Significant Bit) techniques
- Live preview of original and modified images with side-by-side comparison
- Real-time capacity indicator showing available hiding space

#### 🎵 Audio Steganography
- Support for WAV, MP3, FLAC, and OGG audio files
- Automatic conversion of non-WAV files using FFmpeg
- Built-in audio player with play/pause/stop and volume controls
- Metadata extraction for accurate capacity calculation

#### 📝 Data Handling
- Text input for hiding plain messages with UTF-8 encoding
- File input for hiding any file type with filename preservation
- Smart marker system (0x00 for text, length byte for files)
- Automatic detection of data type during extraction

#### 🔒 Security Features
- Optional AES encryption with passphrase protection
- ENCR marker for automatic encrypted data detection
- Passphrase memory during current session
- Secure extraction with decryption prompts

#### 🎨 User Interface
- Dual-tab interface for hiding and extracting data
- Drag & drop support for both images and audio files
- Clipboard integration for one-click text copying
- Image preview highlighting with file path display
- Smart directory fallbacks (Pictures → Images, Music → AppDir)

#### 🔧 Technical Highlights
- Modular engine design with separate image and audio steganography
- FFmpeg integration for professional-grade audio conversion
- Multi-threaded file operations with progress feedback
- Automatic timestamped filenames for stego images and extracted data
- Cross-platform support (Linux, Windows, macOS)

---

