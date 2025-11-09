#!/usr/bin/env python3
import http.server
import socketserver
import requests
import json

class ProxyHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        
        try:
            # Forward to real Moonbeam RPC
            response = requests.post(
                'https://rpc.api.moonbase.moonbeam.network',
                data=post_data,
                headers={'Content-Type': 'application/json'}
            )
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(response.content)
            
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b'{"error":"proxy_error"}')

PORT = 8080
with socketserver.TCPServer(("", PORT), ProxyHandler) as httpd:
    print(f"HTTP proxy running on port {PORT}")
    httpd.serve_forever()
