#!/bin/bash

# ref: https://www.lesstif.com/pages/viewpage.action?pageId=6979614

CA=rootca
FILENAME=ca

mkdir $DIRNAME > /dev/null

echo ">>>>>>>>>>>>>>>>>>>>> [1] gen key"
openssl genrsa -aes256 -out $FILENAME.key.enc 2048

# remove key password
echo ">>>>>>>>>>>>>>>>>>>>> [2] passphase"
openssl rsa -in $FILENAME.key.enc -out $FILENAME.key

echo ">>>>>>>>>>>>>>>>>>>>> [3] csr"
openssl req -new  -key $FILENAME.key -out $FILENAME.csr -config ca_openssl.conf

echo ">>>>>>>>>>>>>>>>>>>>> [4] sign cert"
openssl x509 -req -days 1825 -extensions v3_ca -in $FILENAME.csr -CA $CA.crt -CAcreateserial -CAkey  $CA.key -out $FILENAME.crt  -extfile ca_openssl.conf

openssl x509 -in $FILENAME.crt -text -noout

rm -fr $FILENAME.csr
