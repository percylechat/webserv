import requests
with open('norminet.jpg', 'rb') as f:
    data = f.read()
res = requests.post(url='http://localhost:8080/',
                    data=data,
                    headers={'Content-Type': 'application/octet-stream'})
