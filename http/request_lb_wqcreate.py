# The configuration of haproxy to support http or https
# as follow directory -- haproxy_configuration.

import requests
import json

headers={'content-type' : 'application/json'}
headers['X-auth-token'] = "d61b3de5c28243aeb77953cba9d5178a"

ssl_certificate="""-----BEGIN CERTIFICATE-----
MIICJzCCAZACCQDF4wEPWueW6zANBgkqhkiG9w0BAQUFADBYMQswCQYDVQQGEwJV
UzEUMBIGA1UECAwLQ29ubmVjdGljdXQxEjAQBgNVBAcMCU5ldyBIYXZlbjEMMAoG
A1UECgwDU0ZIMREwDwYDVQQDDAgqLnhpcC5pbzAeFw0xNjAzMDgwOTIxMThaFw0x
NzAzMDgwOTIxMThaMFgxCzAJBgNVBAYTAlVTMRQwEgYDVQQIDAtDb25uZWN0aWN1
dDESMBAGA1UEBwwJTmV3IEhhdmVuMQwwCgYDVQQKDANTRkgxETAPBgNVBAMMCCou
eGlwLmlvMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDQqZtWFvV/8I1oQgvJ
RXhxRY+R61qgRhivjwpOAcf0ZdCZL9WwpDa6pX3R+TlP7cgpmQckY9I01E3+tMY7
F4E4D9S5NU1gGpwuiuYKfzMsK8qlFn6HQ7Fo/A+sDFCzZB2cLn5aWv9SKTr0aUrX
VHBiy3Hk/aHN/4pN/XhzR3UDKQIDAQABMA0GCSqGSIb3DQEBBQUAA4GBALsh8IoK
DVEK2CwWribzCYi68PU3Z/LpABUkm3qyLAodeFiMW+M9pGDIp5URey90R+y9R8i/
ZiUGPGvUyg5mcObvK9N9ttKqqEyOf3+opCn5f74dW/alTXhuXSylAbS4jlsl6eYW
mnbhxQ4bAw+JAiL3weBEkshDD2OHbYGJK3/v
-----END CERTIFICATE-----"""

private_key="""-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQDQqZtWFvV/8I1oQgvJRXhxRY+R61qgRhivjwpOAcf0ZdCZL9Ww
pDa6pX3R+TlP7cgpmQckY9I01E3+tMY7F4E4D9S5NU1gGpwuiuYKfzMsK8qlFn6H
Q7Fo/A+sDFCzZB2cLn5aWv9SKTr0aUrXVHBiy3Hk/aHN/4pN/XhzR3UDKQIDAQAB
AoGAC3V0+VpTaw47xcDywr/U6GuIgdvp4ylWS7XQPA5u+6XnubHdy1CJTOCvsys7
kX8eYeJsePweXBZYnicxR6d3yhHmMi6Rkj1WopSF3nP/qMbt1ncx6XmEFqdx6xW6
A/HM1OcE3BGZDjVkaBfQr0fqu479MR14alMHRzt4yUBWAH0CQQD532wpiPzLWfbY
MemFsl5l52s5yjyzDReaPEDQJz45rGupIoDK5tdfW00Pec6lANq/Mof7wfvFNHYD
FBeVJ2zTAkEA1cd8pjW2igWMeY24R4aU05J249yTKMAwaRyTKiigo86tOVY5aiyN
ZJxUU0nOq7xj3K3AQkf4rZh5iLk8rLyikwJBAMtQ3qPVUUztwQsdYdRl1KW6becy
Bb3zczcOu/YjWGB5SYjwCDgRKUHv+n4TMO/WCCf3J+rUZtaJ31SV7rJwx/0CQG76
foUygS6prsEzhyTqgrxK3plv2p1fGEKY6PUL3kjrdBW941RxCH3ggg4y1K1mwVjA
gnxmLHzCGOWXdB4gVy0CQAfihumkHIlkLrKhLI4mpc2pfWMbkxioCNkmbWrPTMsb
HbGkEx808nqpgT8VsnFi9i2vh8LMvY3bxi/q254hUKg=
-----END RSA PRIVATE KEY-----"""

data = {"pool": {"subnet_id": "c0d5c1c6-0aaa-4998-ad3e-1421c8a8b4e5", 
                 "lb_method": "ROUND_ROBIN",
                 "protocol": "HTTPS",
                 "name": "zwtestv4",
                 "admin_state_up": True,
                 "ssl_certificate": ssl_certificate,
                 "private_key": private_key}
       }
data = json.dumps(data)
url = "http://192.168.0.1:9696/v2.0/lb/pools.json"
response=requests.post(url, data=data, headers=headers)
print response.text



