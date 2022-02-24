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
port = 8080
timeout = 100

# with telnetlib.Telnet(host, port, timeout) as session:
#     session.write(raw_byte)
#     output = session.read_until(b"OK", timeout)
#     session.close()
#     print(output)
#     print("Done")


  
# # read the image file
# img = cv2.imread('chat.jpg', 2)
  
# ret, bw_img = cv2.threshold(img, 127, 255, cv2.THRESH_BINARY)
  
# # converting to its binary form
# bw = cv2.threshold(img, 127, 255, cv2.THRESH_BINARY)
  
# # cv2.imshow("Binary", bw_img)
# # cv2.waitKey(0)
# # cv2.destroyAllWindows()


# f = open('chat.jpg', 'rb') # opening a binary file
# content = f.read() # reading all lines 
# f.close()

# # data = open("chat.jpg", "rb").read()
# # encoded = base64.b64encode(data)

# raw = """POST /hello HTTP/1.1
# Content-Disposition: form-data; name="files[]"; filename="norminet.jpg"
# Content-Type: image/jpg
# Content-Length: %s


# """ % (
#     len(content)
# )

# raw = """POST / HTTP/1.1
# Content-Type: application/x-www-form-urlencoded 
# Content-Length: 34

# cat=Ivitch&enfant=bebe&cat=Lolilol"""

raw = """POST / HTTP/1.1
Content-Type: multipart/form-data
Content-Length: 151;boundary="hibourigolo"

--hibourigolo
Content-Disposition: form-data; name="cat"

hello
--hibourigolo
Content-Disposition: form-data; name="test"; filename="example.txt"

nope"""

raw_byte = str.encode(raw)
with telnetlib.Telnet(host, port, timeout) as session:
    session.write(raw_byte)
    output = session.read_until(b"done", timeout)
    session.close()
    print(output)
    print("Done")