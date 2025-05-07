
import http.client
import urllib.parse

def make_request(method, host, port, path, body=None, headers=None):
    conn = http.client.HTTPConnection(host, port, timeout=5)
    try:
        conn.request(method, path, body=body, headers=headers or {})
        response = conn.getresponse()
        print(f"{method} {path} => {response.status} {response.reason}")
        data = response.read()
        print(f"Response: {data[:150].decode(errors='ignore')}...\n")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        conn.close()

def test_all():
    # Test du serveur 127.0.0.1:8000 (server_name caca)
    print("=== Test serveur caca ===")
    make_request("GET", "127.0.0.1", 8000, "/")
    make_request("GET", "127.0.0.1", 8000, "/images/")
    make_request("POST", "127.0.0.1", 8000, "/form", body="name=test", headers={"Content-Type": "application/x-www-form-urlencoded"})
    make_request("DELETE", "127.0.0.1", 8000, "/upload/form")
    
    # Test d’un CGI Python
    make_request("POST", "127.0.0.1", 8000, "/scripts/test.py", body="x=1", headers={"Content-Type": "application/x-www-form-urlencoded"})

    # Test du serveur 127.0.0.1:1234 (server_name prout)
    print("=== Test serveur prout ===")
    make_request("POST", "127.0.0.1", 1234, "/admin", body="admin=1", headers={"Content-Type": "application/x-www-form-urlencoded"})
    make_request("GET", "127.0.0.1", 1234, "/")  # devrait échouer (GET non autorisé)

    # Test du serveur 127.0.0.1:8080 (server_name pipi)
    print("=== Test serveur pipi ===")
    make_request("GET", "127.0.0.1", 8080, "/")
    make_request("GET", "127.0.0.1", 8080, "/images/")
    make_request("DELETE", "127.0.0.1", 8080, "/images/")
    make_request("GET", "127.0.0.1", 8080, "/upload/")
    make_request("POST", "127.0.0.1", 8080, "/admin/ok", body="action=login", headers={"Content-Type": "application/x-www-form-urlencoded"})

if __name__ == "__main__":
    test_all()