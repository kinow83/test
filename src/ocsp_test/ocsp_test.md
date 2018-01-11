# OCSP Test
## Examples
Good certificate
```
sh ./ocsp_test.sh google.ca
======== Downloading certificates ========
Saving certificate in "ssl_certificate_nAy.pem"...
10.8kB 0:00:00 [ 124kB/s] [   <=>                                                                                                                                                                  ]
Saving certificate chain in "ssl_certificate_chain_kL9.pem"...
2.63kB 0:00:00 [  40kB/s] [   <=>                                                                                                                                                                  ]

============ Locating OCSP URI ===========
The full OCSP address is "http://clients1.google.com/ocsp", so the host is "clients1.google.com"

======= Performing OCSP Validation =======
WARNING: no nonce in response
Response verify OK
/tmp/ssl_certificate_nAy.pem: good
	This Update: Aug 23 07:01:21 2015 GMT
	Next Update: Aug 30 07:01:21 2015 GMT

=========== Removing temp files ==========
removed ‘/tmp/ssl_certificate_nAy.pem’
removed ‘/tmp/ssl_certificate_chain_kL9.pem’
```

Bad Certificate
```
sh ./ocsp_test.sh test-sspev.verisign.com 2443
======== Downloading certificates ========
Saving certificate in "ssl_certificate_21q.pem"...
   2kB 0:00:00 [5.03kB/s] [   <=>                                                                                                                                                                  ]
Saving certificate chain in "ssl_certificate_chain_bmS.pem"...
 3.5kB 0:00:00 [13.1kB/s] [   <=>                                                                                                                                                                  ]

============ Locating OCSP URI ===========
The full OCSP address is "http://sr.symcd.com", so the host is "sr.symcd.com"

======= Performing OCSP Validation =======
WARNING: no nonce in response
Response Verify Failure
140545422997152:error:27069065:OCSP routines:OCSP_basic_verify:certificate verify error:ocsp_vfy.c:126:Verify error:unable to get local issuer certificate
/tmp/ssl_certificate_21q.pem: revoked
	This Update: Jul  7 22:38:29 2015 GMT
	Next Update: Oct  3 10:24:12 2015 GMT
	Reason: unspecified
	Revocation Time: Oct 29 21:29:37 2014 GMT

=========== Removing temp files ==========
removed ‘/tmp/ssl_certificate_21q.pem’
removed ‘/tmp/ssl_certificate_chain_bmS.pem’
```

## Common Errors
### OCSP_basic_verify:certificate verify error
For the purpose of this script, I allways check the OCSP against the same cert that signed the
website's certificate. This error means that the CA didn't use the same cert to sign the OCSP
response, the [RFC](https://tools.ietf.org/html/rfc6960#section-4.2.2.2) says they should but
don't have to. 

### WARNING: no nonce in response
Most CAs will cache OCSP responses for performance reasons so they will not be able to respond
with nonse. If you don't like this in the output you can add -no_nonce