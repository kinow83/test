#!/bin/bash

# ref: https://www.lesstif.com/pages/viewpage.action?pageId=6979614

DIRNAME=rootca
FILENAME=$DIRNAME/rootca

mkdir $DIRNAME > /dev/null

openssl genrsa -aes256 -out $FILENAME.key 2048

openssl req -new -key $FILENAME.key -out $FILENAME.csr -config openssl.conf

openssl x509 -req -days 3650 -extensions v3_ca -set_serial 1 -in $FILENAME.csr -signkey $FILENAME.key -out $FILENAME.crt -extfile openssl.conf

rm -fr $FILENAME.csr
