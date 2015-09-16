# Generation of Permissions CA private key:
openssl genrsa -out permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem  2048

# Generation of self-signed certificate request for the Permissions CA
openssl req -config permissions_ca_files/permissions_ca_openssl.cnf -new -key permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem -out permissions_ca_files/permissions_ca.csr

# Generation of RTI Identity Certificate by the Identity CA (signing of certificate request)
openssl x509 -req -days 3650 -in  permissions_ca_files/permissions_ca.csr  -signkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem  -out TESTONLY_permissions_ca_cert.pem

#Initialization of Index and Serial number files
touch permissions_ca_files/index.txt
echo "01" > permissions_ca_files/serial

# Signing a governance document d:
openssl smime -sign -in TESTONLY_governance.xml -text -out  TESTONLY_governance_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	


