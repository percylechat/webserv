import requests
with open('chat.jpg', 'rb') as f:
    data = f.read()
res = requests.post(url='http://localhost:8080/',
                    data=data,
                    headers={'Content-Type': 'application/octet-stream'})
