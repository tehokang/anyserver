# How to make CA(Certification Authority) certification
## 1.Create rootca private key
<pre>
openssl genrsa -aes256 -out rootca_private.key 2048
</pre>

## 2. Make conf to create CSR of rootca certification (rootca_opensll.conf)
I learned how to make self-sign certification at [here](https://www.lesstif.com/pages/viewpage.action?pageId=6979614)
<pre>
openssl req -new -key rootca_private.key -out rootca.csr -config rootca_openssl.conf
</pre>

## 3. Create rootca certification
<pre>
openssl x509 -req -days 3650 -extensions v3_ca -set_serial 1 -in rootca.csr -signkey rootca_private.key -out rootca_cert.crt -extfile rootca_openssl.conf
</pre>

# How to make Server certification
## 1. Create webserver private key
<pre>
openssl genrsa -aes256 -out webserver.key 2048
cp webserver.key webserver.key.enc
openssl rsa -in webserver.key.enc -out webserver.key
</pre>

## 2. Make conf to create CSR of webserver certification(host_openssl.conf)
<pre>
openssl req -new -key webserver.key -out webserver.csr -config host_openssl.conf
</pre>

## 3. Create webserver certification
<pre>
openssl x509 -req -days 1825 -extensions v3_user -in webserver.csr -CA rootca_cert.crt -CAcreateserial -CAkey rootca_private.key -out webserver.crt -extfile host_openssl.conf
</pre>


## 4. double check if certification has problem or not. 
<pre>
openssl x509 -text -in /etc/pki/tls/certs/lesstif.com.crt
</pre>

