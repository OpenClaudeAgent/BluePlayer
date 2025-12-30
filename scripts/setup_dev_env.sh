#!/bin/bash
# BluePlayer Development Environment Setup
# This script guides you through the complete setup process

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Get script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

echo ""
echo -e "${BLUE}${BOLD}=======================================${NC}"
echo -e "${BLUE}${BOLD}  BluePlayer Development Setup${NC}"
echo -e "${BLUE}${BOLD}=======================================${NC}"
echo ""

# ==============================================================================
# Helper Functions
# ==============================================================================

print_step() {
    echo -e "${CYAN}[STEP]${NC} $1"
}

print_check() {
    echo -e "${GREEN}[CHECK]${NC} $1"
}

print_ok() {
    echo -e "${GREEN}[OK]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

# ==============================================================================
# Step 1: Check Prerequisites
# ==============================================================================

print_step "Checking prerequisites..."
echo ""

# Check for Homebrew
if command -v brew &> /dev/null; then
    print_ok "Homebrew installed"
else
    print_error "Homebrew not found. Please install from https://brew.sh"
    exit 1
fi

# Check for mkcert
if command -v mkcert &> /dev/null; then
    print_ok "mkcert installed"
else
    print_warn "mkcert not found. Installing..."
    brew install mkcert
    mkcert -install
    print_ok "mkcert installed and configured"
fi

# Check for Qt
if command -v qmake &> /dev/null || [ -d "/opt/homebrew/opt/qt" ]; then
    print_ok "Qt found"
else
    print_warn "Qt not detected. You may need to install it: brew install qt"
fi

echo ""

# ==============================================================================
# Step 2: Generate TLS Certificates
# ==============================================================================

print_step "Setting up TLS certificates for OAuth..."
echo ""

CERTS_DIR="$PROJECT_ROOT/certs"

if [ -f "$CERTS_DIR/twitch-cert.pem" ] && [ -f "$CERTS_DIR/twitch-key.pem" ]; then
    echo -e "  Existing certificates found in ${YELLOW}certs/${NC}"
    read -p "  Regenerate certificates? (y/N): " REGEN_CERTS
    if [[ "$REGEN_CERTS" =~ ^[Yy]$ ]]; then
        rm -rf "$CERTS_DIR"
    else
        print_ok "Keeping existing certificates"
    fi
fi

if [ ! -f "$CERTS_DIR/twitch-cert.pem" ]; then
    mkdir -p "$CERTS_DIR"
    print_info "Generating TLS certificates with mkcert..."
    (cd "$CERTS_DIR" && mkcert -cert-file twitch-cert.pem -key-file twitch-key.pem localhost 127.0.0.1)
    print_ok "TLS certificates generated in certs/"
fi

echo ""

# ==============================================================================
# Step 3: Twitch Application Setup (Interactive)
# ==============================================================================

print_step "Twitch Application Configuration"
echo ""

# Check if .env already exists
if [ -f "$PROJECT_ROOT/.env" ]; then
    print_warn ".env file already exists"
    read -p "  Overwrite with new configuration? (y/N): " OVERWRITE_ENV
    if [[ ! "$OVERWRITE_ENV" =~ ^[Yy]$ ]]; then
        print_ok "Keeping existing .env file"
        echo ""
        print_step "Setup complete! Your environment is ready."
        exit 0
    fi
fi

echo ""
echo -e "${BOLD}To use BluePlayer, you need a Twitch Developer Application.${NC}"
echo ""
echo -e "  ${YELLOW}1.${NC} Open ${CYAN}https://dev.twitch.tv/console/apps${NC}"
echo -e "  ${YELLOW}2.${NC} Log in with your Twitch account"
echo -e "  ${YELLOW}3.${NC} Click ${BOLD}'Register Your Application'${NC} (or use existing)"
echo -e "  ${YELLOW}4.${NC} Fill in the form:"
echo -e "       - Name: ${CYAN}BluePlayer (or any name)${NC}"
echo -e "       - OAuth Redirect URLs: ${CYAN}https://127.0.0.1:8443/callback${NC}"
echo -e "       - Category: ${CYAN}Application Integration${NC}"
echo -e "  ${YELLOW}5.${NC} Click ${BOLD}'Create'${NC}"
echo -e "  ${YELLOW}6.${NC} On your app page, click ${BOLD}'New Secret'${NC} to generate a Client Secret"
echo ""

read -p "Press Enter when you have your Client ID and Secret ready..."

echo ""

# Get Client ID
while true; do
    read -p "Enter your Twitch Client ID: " TWITCH_CLIENT_ID
    if [ -z "$TWITCH_CLIENT_ID" ]; then
        print_error "Client ID cannot be empty"
    elif [ ${#TWITCH_CLIENT_ID} -lt 20 ]; then
        print_warn "Client ID seems too short. Twitch Client IDs are usually 30 characters."
        read -p "Continue anyway? (y/N): " CONTINUE
        if [[ "$CONTINUE" =~ ^[Yy]$ ]]; then
            break
        fi
    else
        break
    fi
done

# Get Client Secret
while true; do
    read -sp "Enter your Twitch Client Secret: " TWITCH_CLIENT_SECRET
    echo ""
    if [ -z "$TWITCH_CLIENT_SECRET" ]; then
        print_error "Client Secret cannot be empty"
    elif [ ${#TWITCH_CLIENT_SECRET} -lt 20 ]; then
        print_warn "Client Secret seems too short. Twitch secrets are usually 30 characters."
        read -p "Continue anyway? (y/N): " CONTINUE
        if [[ "$CONTINUE" =~ ^[Yy]$ ]]; then
            break
        fi
    else
        break
    fi
done

echo ""

# ==============================================================================
# Step 4: Create .env file
# ==============================================================================

print_step "Creating .env file..."

cat > "$PROJECT_ROOT/.env" << EOF
# BluePlayer Environment Configuration
# Generated by setup_dev_env.sh on $(date +%Y-%m-%d)
# WARNING: Never commit this file to git!

# Twitch Application Credentials
TWITCH_CLIENT_ID=$TWITCH_CLIENT_ID
TWITCH_CLIENT_SECRET=$TWITCH_CLIENT_SECRET

# TLS Certificates (default paths)
TWITCH_TLS_CERT_PATH=certs/twitch-cert.pem
TWITCH_TLS_KEY_PATH=certs/twitch-key.pem

# OAuth Configuration
TWITCH_REDIRECT_URI=https://127.0.0.1:8443/callback
TWITCH_REDIRECT_PORT=8443
EOF

print_ok ".env file created"

echo ""

# ==============================================================================
# Step 5: Verify Setup
# ==============================================================================

print_step "Verifying setup..."
echo ""

ERRORS=0

# Check .env exists and has content
if [ -f "$PROJECT_ROOT/.env" ]; then
    print_ok ".env file exists"
else
    print_error ".env file missing"
    ((ERRORS++))
fi

# Check certificates
if [ -f "$CERTS_DIR/twitch-cert.pem" ] && [ -f "$CERTS_DIR/twitch-key.pem" ]; then
    print_ok "TLS certificates present"
else
    print_error "TLS certificates missing"
    ((ERRORS++))
fi

# Check .gitignore
if grep -q "^\.env$" "$PROJECT_ROOT/.gitignore" 2>/dev/null; then
    print_ok ".env is in .gitignore"
else
    print_warn ".env may not be in .gitignore - check manually!"
fi

if grep -q "^certs/" "$PROJECT_ROOT/.gitignore" 2>/dev/null; then
    print_ok "certs/ is in .gitignore"
else
    print_warn "certs/ may not be in .gitignore - check manually!"
fi

echo ""

# ==============================================================================
# Summary
# ==============================================================================

if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}${BOLD}=======================================${NC}"
    echo -e "${GREEN}${BOLD}  Setup Complete!${NC}"
    echo -e "${GREEN}${BOLD}=======================================${NC}"
    echo ""
    echo -e "Your BluePlayer development environment is ready."
    echo ""
    echo -e "${BOLD}Next steps:${NC}"
    echo -e "  1. Build the project:"
    echo -e "     ${CYAN}make build${NC}"
    echo ""
    echo -e "  2. Run BluePlayer:"
    echo -e "     ${CYAN}make run${NC}"
    echo ""
    echo -e "  3. Log in with your Twitch account when prompted"
    echo ""
else
    echo -e "${RED}${BOLD}Setup completed with $ERRORS error(s)${NC}"
    echo "Please fix the errors above and run this script again."
    exit 1
fi
