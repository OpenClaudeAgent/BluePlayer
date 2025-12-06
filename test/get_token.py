#!/usr/bin/env python3
"""Extract OAuth token from BluePlayer SecureStorage"""
import sys
import base64
from pathlib import Path
from configparser import ConfigParser

def derive_key():
    """Derive encryption key (simplified - matches C++ implementation)"""
    import hashlib
    # This is a simplified version - the real key derivation uses system UUID
    # For testing, we'll try to read from the same location
    salt = b"BluePlayerSecureStorage2024"
    # Try to get system UUID - simplified approach
    # In real implementation, this uses IOKit on macOS
    system_id = "test-uuid"  # This won't work, but let's try reading the file
    key_data = (system_id + salt.decode()).encode()
    return hashlib.sha256(key_data).digest()

def decrypt(ciphertext, key):
    """Decrypt using XOR (matches C++ implementation)"""
    encrypted = base64.b64decode(ciphertext)
    decrypted = bytearray()
    for i in range(len(encrypted)):
        decrypted.append(encrypted[i] ^ key[i % len(key)])
    return decrypted.decode('utf-8')

def main():
    # Find QSettings file location
    config_path = Path.home() / "Library/Preferences/BluePlayer/SecureStorage.ini"
    if not config_path.exists():
        # Try alternative location
        config_path = Path.home() / ".config/BluePlayer/SecureStorage.ini"
    
    if not config_path.exists():
        print(f"ERROR: SecureStorage file not found at {config_path}", file=sys.stderr)
        print("Please run the app first to generate tokens", file=sys.stderr)
        return 1
    
    config = ConfigParser()
    config.read(config_path)
    
    encrypted_token = config.get('General', 'access_token', fallback=None)
    if not encrypted_token:
        print("ERROR: No access_token found in SecureStorage", file=sys.stderr)
        return 1
    
    # Note: This won't work without the actual system UUID, but let's try
    # The user will need to provide the token manually or we need the C++ extractor
    print("ERROR: Token extraction requires system UUID matching", file=sys.stderr)
    print("Please use the C++ extractor or provide token manually", file=sys.stderr)
    print(f"Encrypted token location: {config_path}", file=sys.stderr)
    return 1

if __name__ == '__main__':
    sys.exit(main())





