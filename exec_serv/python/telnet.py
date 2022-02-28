import telnetlib
import base64

import cv2
# raw = """POST /test/ HTTP/1.0
# From: frog@jmarshall.com
# User-Agent: HTTPTool/1.0
# Content-Type: application/x-www-form-urlencoded
# Content-Length: 32

# veme=Cosby&favorite+flavor=flies
# """
# raw_byte = str.encode(raw)


host = "127.0.0.1"
port = 8082
timeout = 100

#classic post upload
# raw = """POST / HTTP/1.1
<<<<<<< HEAD
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# filename= "test.txt"
# Content-Lenght: 22 \r\n\r
# hello bebe chat mignon
# """
#chunked encoding upload
raw = """POST / HTTP/1.1
Host: 127.0.0.1:8082
Content-Type: text/plain
filename= "hello.txt"
Transfer-Encoding: chunked\r\n\r
26\r\nVoici les données du premier morceau\r\n\r
1C\ret voici un second morceau\r\n\r\n
20\ret voici deux derniers morceaux \r\n
12\r\sans saut de ligne\r\n
0\r\r\n
=======
# Content-Type: application/x-www-form-urlencoded 
# Content-Length: 34

# cat=Ivitch&enfant=bebe&cat=Lolilol"""

raw = """DELETE /ugly_cat/mdr.jpg HTTP/1.1
Host: 127.0.0.1:8082
Content-Type: text/plain
filename= "test.txt
Content-Lenght: 22 \r\n\r
hello bebe chat mignon
>>>>>>> 70691cf5e86d1e2ea84fada0ecc9f316c5336dbe
"""

raw_byte = str.encode(raw)
with telnetlib.Telnet(host, port, timeout) as session:
    session.write(raw_byte)
    output = session.read_until(b"done", timeout)
    session.close()
    print(output)
    print("Done")