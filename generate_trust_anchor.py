#!/usr/bin/env python3
import ssl
import socket
from cryptography import x509
from cryptography.hazmat.primitives import serialization

def get_cert_chain(hostname, port=443):
    context = ssl.create_default_context()
    with socket.create_connection((hostname, port)) as sock:
        with context.wrap_socket(sock, server_hostname=hostname) as ssock:
            cert_der = ssock.getpeercert(True)
            cert_chain = ssock.getpeercert_chain()
    return cert_chain

def cert_to_trust_anchor(cert, index=0):
    # Get the subject
    subject = cert.subject
    subject_der = subject.public_bytes()
    
    # Get the public key
    public_key = cert.public_key()
    
    if hasattr(public_key, 'public_numbers'):  # RSA key
        numbers = public_key.public_numbers()
        n_bytes = numbers.n.to_bytes((numbers.n.bit_length() + 7) // 8, 'big')
        e_bytes = numbers.e.to_bytes((numbers.e.bit_length() + 7) // 8, 'big')
        
        # Generate C arrays
        dn_array = ', '.join(f'0x{b:02x}' for b in subject_der)
        n_array = ', '.join(f'0x{b:02x}' for b in n_bytes)
        e_array = ', '.join(f'0x{b:02x}' for b in e_bytes)
        
        return f"""
static const unsigned char TA_DN{index}[] = {{
  {dn_array}
}};

static const unsigned char TA_RSA_N{index}[] = {{
  {n_array}
}};

static const unsigned char TA_RSA_E{index}[] = {{
  {e_array}
}};
""", f"""  {{
    {{ (unsigned char*)TA_DN{index}, sizeof TA_DN{index} }},
    BR_X509_TA_CA,
    {{
      BR_KEYTYPE_RSA,
      {{ .rsa = {{
        (unsigned char*)TA_RSA_N{index}, sizeof TA_RSA_N{index},
        (unsigned char*)TA_RSA_E{index}, sizeof TA_RSA_E{index},
      }} }}
    }}
  }}"""
    else:
        print(f"Unsupported key type: {type(public_key)}")
        return "", ""

def main():
    hostname = "rpc.api.moonbase.moonbeam.network"
    
    try:
        cert_chain = get_cert_chain(hostname)
        
        # Use the root certificate (last in chain)
        root_cert = cert_chain[-1]
        
        cert_arrays, trust_anchor = cert_to_trust_anchor(root_cert, 0)
        
        header_content = f"""#ifndef TRUST_ANCHORS_H
#define TRUST_ANCHORS_H

#include <SSLClient.h>

#define TAs_NUM 1
{cert_arrays}
static const br_x509_trust_anchor TAs[] = {{
{trust_anchor}
}};

#endif"""
        
        with open('trust_anchors.h', 'w') as f:
            f.write(header_content)
        
        print("Trust anchor generated successfully!")
        print(f"Root certificate subject: {root_cert.subject}")
        
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
