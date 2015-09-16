# These commands assume the CWD is the repository root (dds-security)

# Generation of TwinOaks Identity private key:
openssl genrsa -out twinoaks_coredx_certs/private/TESTONLY_twinoaks_coredx_identity_private_key.pem 2048

# Generation of certificate request to be signed by the Identity CA:
openssl req -config twinoaks_coredx_certs/twinoaks_coredx_openssl.cnf -new -key twinoaks_coredx_certs/private/TESTONLY_twinoaks_coredx_identity_private_key.pem -out twinoaks_coredx_certs/identity_twinoaks_coredx.csr

# See identity_ca_files/README.txt for the generation of TwinOaks Identity Certificate by the Identity CA (signing of certificate request)

# See permissions_ca_files/README.txt for the generation of TwinOaks Permission Signed documents by the Permissions CA (signing of Permissions XML documents)




