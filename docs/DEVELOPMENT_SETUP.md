# BluePlayer - Development Environment Setup

This guide walks you through setting up a development environment for BluePlayer.

## Prerequisites

### Required Software

| Software | Version | Installation |
|----------|---------|--------------|
| macOS | 13+ (Ventura) | - |
| Xcode Command Line Tools | Latest | `xcode-select --install` |
| Homebrew | Latest | [brew.sh](https://brew.sh) |
| Qt | 6.5+ | `brew install qt` |
| CMake | 3.24+ | `brew install cmake` |
| Ninja | Latest | `brew install ninja` |
| mkcert | Latest | `brew install mkcert` |

### Twitch Developer Account

You need a Twitch account with developer access:
1. Go to [dev.twitch.tv](https://dev.twitch.tv)
2. Log in with your Twitch account
3. Accept the Developer Agreement if prompted

## Quick Setup (Recommended)

Run the automated setup script:

```bash
./scripts/setup_dev_env.sh
```

This script will:
1. Check and install prerequisites (mkcert)
2. Generate TLS certificates for OAuth
3. Guide you through creating a Twitch application
4. Create your `.env` file with credentials

## Manual Setup

If you prefer to set things up manually, follow these steps:

### Step 1: Install Dependencies

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required packages
brew install qt cmake ninja mkcert

# Install mkcert root CA
mkcert -install
```

### Step 2: Create Twitch Application

1. Go to [Twitch Developer Console](https://dev.twitch.tv/console/apps)
2. Click **"Register Your Application"**
3. Fill in the form:
   - **Name**: BluePlayer (or any name you prefer)
   - **OAuth Redirect URLs**: `https://127.0.0.1:8443/callback`
   - **Category**: Application Integration
4. Click **"Create"**
5. On your application page:
   - Copy the **Client ID**
   - Click **"New Secret"** to generate a **Client Secret**
   - Copy the secret immediately (it won't be shown again)

### Step 3: Generate TLS Certificates

BluePlayer uses HTTPS for OAuth redirects. Generate local certificates:

```bash
# Create certificates directory
mkdir -p certs

# Generate certificates for localhost
cd certs
mkcert -cert-file twitch-cert.pem -key-file twitch-key.pem localhost 127.0.0.1
cd ..
```

### Step 4: Configure Environment Variables

Create a `.env` file in the project root:

```bash
cp .env.example .env
```

Edit `.env` and fill in your credentials:

```bash
# Required
TWITCH_CLIENT_ID=your_client_id_here
TWITCH_CLIENT_SECRET=your_client_secret_here

# Optional (defaults shown)
TWITCH_TLS_CERT_PATH=certs/twitch-cert.pem
TWITCH_TLS_KEY_PATH=certs/twitch-key.pem
TWITCH_REDIRECT_URI=https://127.0.0.1:8443/callback
TWITCH_REDIRECT_PORT=8443
```

### Step 5: Build and Run

```bash
# Build
make build

# Run
make run
```

## Environment Variables Reference

### Required Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `TWITCH_CLIENT_ID` | Your Twitch application Client ID | `abc123def456...` |
| `TWITCH_CLIENT_SECRET` | Your Twitch application Client Secret | `xyz789ghi012...` |

### Optional Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `TWITCH_TLS_CERT_PATH` | Path to TLS certificate | `certs/twitch-cert.pem` |
| `TWITCH_TLS_KEY_PATH` | Path to TLS private key | `certs/twitch-key.pem` |
| `TWITCH_REDIRECT_URI` | OAuth redirect URL | `https://127.0.0.1:8443/callback` |
| `TWITCH_REDIRECT_PORT` | OAuth server port | `8443` |

### Development/Testing Variables

| Variable | Description | Used By |
|----------|-------------|---------|
| `BLUEPLAYER_API_URL` | Custom API base URL | E2E tests |
| `BLUEPLAYER_IRC_URL` | Custom IRC WebSocket URL | E2E tests |
| `BLUEPLAYER_HLS_PROXY_URL` | Custom HLS proxy URL | E2E tests |
| `E2E_FIXTURES_PATH` | Path to test fixtures | E2E tests |

## Troubleshooting

### "Certificate not trusted" error

If you see SSL/TLS errors:

```bash
# Reinstall mkcert root CA
mkcert -install

# Regenerate certificates
rm -rf certs/
mkdir -p certs
cd certs && mkcert -cert-file twitch-cert.pem -key-file twitch-key.pem localhost 127.0.0.1
```

### "Invalid Client ID" error

1. Verify your Client ID in `.env` matches the one on dev.twitch.tv
2. Make sure there are no extra spaces or quotes
3. Check that your Twitch application is not suspended

### "Invalid Client Secret" error

1. Client secrets can only be viewed once when generated
2. If you lost it, generate a new one on dev.twitch.tv (click "New Secret")
3. Update your `.env` with the new secret

### OAuth redirect fails

1. Verify `https://127.0.0.1:8443/callback` is in your Twitch app's OAuth Redirect URLs
2. Check that port 8443 is not in use by another application
3. Verify certificates are valid: `openssl x509 -in certs/twitch-cert.pem -text -noout`

### Build errors

```bash
# Clean build directory
make clean

# Rebuild
make build
```

## Security Notes

- **Never commit `.env`** - It contains secrets
- **Never commit `certs/`** - Contains private keys
- Both are already in `.gitignore`
- If you accidentally commit secrets, revoke them immediately on dev.twitch.tv

## Running Tests

```bash
# Unit and functional tests
make test

# E2E tests (fast)
make e2e

# E2E tests (all, including slow)
make e2e-extended

# Run specific E2E scenario
make e2e SCENARIO=open-stream

# Interactive launcher with mock servers
make e2e-launcher
```

## IDE Setup

### CLion

1. Open the project folder
2. CLion should detect CMakeLists.txt automatically
3. Set CMake options: `-G Ninja`
4. Add environment variables in Run Configuration

### VS Code

1. Install extensions: CMake Tools, C/C++
2. Open the project folder
3. Select kit when prompted
4. Configure with: `CMake: Configure`

### Qt Creator

1. Open CMakeLists.txt as project
2. Configure with your Qt kit
3. Add environment variables in Projects > Build Environment
