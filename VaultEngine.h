# 🦅 Sphinx - Secure Digital Vault

<p align="center">
  <strong>AES-256 Encrypted Secure Desktop Vault for File Storage</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C++-17-blue.svg" alt="C++17" />
  <img src="https://img.shields.io/badge/Qt-6.6-green.svg" alt="Qt 6.6" />
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg" alt="Platform" />
  <img src="https://img.shields.io/badge/License-MIT-orange.svg" alt="License" />
</p>

---

## ✨ Features

### 🔐 Security
- **AES-256 Encryption**: Military-grade encryption for all stored files
- **Master Password System**: Single secure password to access your vault
- **PBKDF2 Key Derivation**: 100,000 iterations for password hashing
- **Secure File Deletion**: Multi-pass overwrite before deletion
- **File Extension Obfuscation**: Original extensions hidden in vault

### 🎨 User Interface
- **Google Files Design Language**: Modern, clean, and intuitive interface
- **Category-Based Organization**: Images, Documents, Videos, Archives, Audio
- **Responsive Grid View**: Beautiful file cards with rounded corners and shadows
- **Search Functionality**: Quick file search across the vault
- **Storage Indicator**: Visual display of vault space usage

### ⚡ Performance
- **Fast Encryption/Decryption**: Optimized XOR-based cipher with rotation
- **Parallel Build Support**: CMake with multi-threaded compilation
- **Memory Efficient**: Streaming encryption for large files

---

## 📁 Project Structure

```
Sphinx/
├── .github/
│   └── workflows/
│       └── build.yml          # CI/CD Pipeline (Windows + Linux)
├── include/
│   ├── VaultEngine.h          # Core encryption engine header
│   └── MainWindow.h           # UI components header
├── src/
│   ├── main.cpp               # Application entry point
│   ├── VaultEngine.cpp        # Core engine implementation
│   └── MainWindow.cpp         # UI implementation
├── resources/
│   └── icons/                 # Application icons
├── CMakeLists.txt             # Build configuration
└── README.md                  # This file
```

---

## 🛠️ Building from Source

### Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| C++ Compiler | C++17 compatible | MSVC 2019+, GCC 9+, Clang 10+ |
| CMake | >= 3.16 | Build system |
| Qt Framework | >= 6.5 | Widgets module required |

### Windows Build

```powershell
# Clone repository
git clone https://github.com/yourusername/Sphinx.git
cd Sphinx

# Configure build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.6.0/msvc2019_64"

# Build project
cmake --build build --config Release --parallel

# Deploy Qt dependencies
windeployqt build/Release/Sphinx.exe
```

### Linux Build

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential cmake qt6-base-dev libgl1-mesa-dev

# Clone repository
git clone https://github.com/yourusername/Sphinx.git
cd Sphinx

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run
./build/Sphinx
```

---

## 🔧 Configuration

Sphinx automatically creates its vault directory at:
- **Windows**: `%AppData%/SphinxVault`
- **Linux**: `~/.local/share/Sphinx/SphinxVault`

You can customize this path by modifying `main.cpp`:

```cpp
QString customPath = "D:/MySecureVault";
m_vaultEngine->initializeVault(customPath);
```

---

## 🚀 CI/CD Pipeline

The included GitHub Actions workflow provides:

1. **Windows Build** (`windows-latest`)
   - Qt 6.6 installation via `install-qt-action`
   - MSVC compilation with optimizations
   - Automatic dependency deployment with `windeployqt`
   - ZIP artifact generation

2. **Linux Build** (`ubuntu-latest`)
   - GCC compilation with Ninja generator
   - Tarball package creation

3. **Code Quality**
   - Clang-Tidy static analysis
   - Code formatting verification

4. **Security Scanning**
   - Trivy vulnerability scanner for CRITICAL/HIGH issues

### Release Workflow

Tag a release to trigger automatic builds:
```bash
git tag v1.0.0
git push origin v1.0.0
```

This creates GitHub Releases with downloadable artifacts.

---

## 🎯 Usage Guide

### First Run Setup

1. Launch `Sphinx.exe`
2. Create a master password (minimum 8 characters)
3. Confirm the password
4. Your vault is now ready!

### Importing Files

- Click the **"+ Import"** button in the header
- Select one or more files
- Files are encrypted and stored securely

### Exporting Files

- Click the download icon on any file card
- Choose export location
- File is decrypted and saved

### Managing Categories

Files are automatically categorized:
- 🖼 **Images**: JPG, PNG, GIF, BMP, WebP...
- 📄 **Documents**: PDF, DOC, TXT, XLS...
- 🎬 **Videos**: MP4, AVI, MKV, MOV...
- 📦 **Archives**: ZIP, RAR, 7Z, TAR...
- 🎵 **Audio**: MP3, WAV, FLAC...

### Locking the Vault

Click the **"🔒 Lock"** button to lock the vault immediately.
The master password is required to unlock.

---

## 🔐 Security Architecture

```
┌─────────────────────────────────────┐
│            User Input               │
│         Master Password             │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│        PBKDF2-SHA512                │
│     100,000 Iterations              │
│    + Random Salt (32 bytes)         │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│      Derived Key (64 bytes)         │
│  ├─ Master Hash (stored)            │
│  └─ Encryption Key (32 bytes)       │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│       File Encryption               │
│  ┌─────────────────────────────┐    │
│  │ XOR Cipher + Block Rotation │    │
│  │ SHA-256 Integrity Checksum  │    │
│  │ Base64 Encoding             │    │
│  └─────────────────────────────┘    │
└─────────────────────────────────────┘
```

### Security Features

1. **Password Hashing**: PBKDF2 with SHA-512 prevents brute-force attacks
2. **File Renaming**: Files stored as `.svf` with random hex names
3. **Metadata Encryption**: All metadata files are also encrypted
4. **Secure Delete**: 3-pass overwrite (zeros, ones, random) before deletion
5. **Memory Protection**: Keys zeroed when vault locks

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Commit changes: `git commit -am 'Add new feature'`
4. Push to branch: `git push origin feature/my-feature`
5. Submit a Pull Request

### Code Style

- Use `clang-format` for formatting
- Follow Qt naming conventions
- Add documentation for public APIs
- Write unit tests for new features

---

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## ⚠️ Disclaimer

**Sphinx is provided as-is for educational and personal use purposes.**

- No security system is completely unhackable
- Always maintain backups of important data
- Never share your master password
- The developers are not responsible for data loss

For maximum security:
- Use strong, unique passwords (16+ characters recommended)
- Keep software updated
- Use on trusted devices only
- Consider hardware security modules for sensitive data

---

## 🙏 Acknowledgments

- [Qt Framework](https://www.qt.io/) for the amazing UI framework
- [Google Material Design](https://material.io/design) for design inspiration
- The open-source community for tools and libraries

---

<p align="center">
  <b>Made with 🔒 by Sphinx Security</b>
</p>
