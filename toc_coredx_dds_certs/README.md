These commands assume the CWD is the repository root (dds-security)

Generation of TwinOaks Identity private key:

    openssl genrsa -out toc_coredx_dds_certs/private/TESTONLY_toc_coredx_dds_identity_private_key.pem 2048

Generation of certificate request to be signed by the Identity CA:

    openssl req -config toc_coredx_dds_certs/toc_coredx_dds_openssl.cnf -new -key toc_coredx_dds_certs/private/TESTONLY_toc_coredx_dds_identity_private_key.pem -out toc_coredx_dds_certs/identity_twinoaks_coredx.csr

See identity_ca_files/README.md for the generation of TwinOaks Identity
Certificate by the Identity CA (signing of certificate request)

See permissions_ca_files/README.md for the generation of TwinOaks Permission
Signed documents by the Permissions CA (signing of Permissions XML documents)
