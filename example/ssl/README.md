## RootCA
#1. Create rootca private key
openssl genrsa -aes256 -out rootca_private.key 2048

#2. Make conf to create CSR of rootca certification (rootca_opensll.conf)
#(https://www.lesstif.com/pages/viewpage.action?pageId=6979614)
openssl req -new -key rootca_private.key -out rootca.csr -config rootca_openssl.conf

#3. Create rootca certification
openssl x509 -req -days 3650 -extensions v3_ca -set_serial 1 -in rootca.csr -signkey rootca_private.key -out rootca_cert.crt -extfile rootca_openssl.conf

## Server Certification
#1. Create webserver private key
openssl genrsa -aes256 -out webserver.key 2048
cp webserver.key webserver.key.enc
openssl rsa -in webserver.key.enc -out webserver.key

#2. Make conf to create CSR of webserver certification(host_openssl.conf)
openssl req -new -key webserver.key -out webserver.csr -config host_openssl.conf

#3. Create webserver certification
openssl x509 -req -days 1825 -extensions v3_user -in webserver.csr -CA rootca_cert.crt -CAcreateserial -CAkey rootca_private.key -out webserver.crt -extfile host_openssl.conf



# double check 
openssl x509 -text -in /etc/pki/tls/certs/lesstif.com.crt

