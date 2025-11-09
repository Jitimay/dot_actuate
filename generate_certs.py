#!/usr/bin/env python3
import ssl
import socket
import subprocess
import sys

def get_certificate_chain(hostname, port=443):
    """Get the certificate chain for a hostname"""
    try:
        # Get the certificate
        context = ssl.create_default_context()
        with socket.create_connection((hostname, port)) as sock:
            with context.wrap_socket(sock, server_hostname=hostname) as ssock:
                cert_der = ssock.getpeercert(True)
                cert_pem = ssl.DER_cert_to_PEM_cert(cert_der)
                return cert_pem
    except Exception as e:
        print(f"Error getting certificate: {e}")
        return None

def generate_trust_anchors_h(hostname, output_file="trust_anchors.h"):
    """Generate trust_anchors.h file for SSLClient"""
    cert_pem = get_certificate_chain(hostname)
    if not cert_pem:
        print("Failed to get certificate")
        return False
    
    # Write the trust_anchors.h file
    with open(output_file, 'w') as f:
        f.write('#ifndef _TRUST_ANCHORS_H_\n')
        f.write('#define _TRUST_ANCHORS_H_\n\n')
        f.write('#ifdef __cplusplus\n')
        f.write('extern "C" {\n')
        f.write('#endif\n\n')
        
        # Convert PEM to C array format
        cert_lines = cert_pem.strip().split('\n')
        f.write('const unsigned char TAs[] PROGMEM = {\n')
        
        cert_data = ''.join(cert_lines[1:-1])  # Remove BEGIN/END lines
        import base64
        cert_bytes = base64.b64decode(cert_data)
        
        for i, byte in enumerate(cert_bytes):
            if i % 16 == 0:
                f.write('  ')
            f.write(f'0x{byte:02x}')
            if i < len(cert_bytes) - 1:
                f.write(', ')
            if (i + 1) % 16 == 0:
                f.write('\n')
        
        if len(cert_bytes) % 16 != 0:
            f.write('\n')
        
        f.write('};\n\n')
        f.write(f'const size_t TAs_NUM = {len(cert_bytes)};\n\n')
        f.write('#ifdef __cplusplus\n')
        f.write('}\n')
        f.write('#endif\n\n')
        f.write('#endif\n')
    
    print(f"Generated {output_file} successfully")
    return True

if __name__ == "__main__":
    hostname = "rpc.api.moonbase.moonbeam.network"
    generate_trust_anchors_h(hostname)