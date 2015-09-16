# These commands assume the CWD is the repository root (dds-security)

# Generation of RTI Identity private key:
openssl genrsa -out twinoaks_coredx_files/private/TESTONLY_twinoaks_coredx_identity_private_key.pem 2048

# Generation of certificate request to be signed by the Identity CA:
openssl req -config twinoaks_coredx_files/twinoaks_coredx_openssl.cnf -new -key twinoaks_coredx_files/private/TESTONLY_twinoaks_coredx_identity_private_key.pem -out twinoaks_coredx_files/identity_twinoaks_coredx.csr

# Generation of RTI Identity Certificate by the Identity CA (signing of certificate request)
openssl ca -config identity_ca_files/identity_ca_openssl.cnf -days 3650 -in twinoaks_coredx_files/identity_twinoaks_coredx.csr -out  twinoaks_coredx_files/TESTONLY_twinoaks_coredx_identity_cert.pem




