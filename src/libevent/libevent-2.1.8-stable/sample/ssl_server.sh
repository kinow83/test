#!/bin/bash
openssl s_server -accept 443 -www -key server.key -cert server.crt
