Generation of Authentication/Identity CA private key:

    openssl genrsa -out identity_ca_files/private/TESTONLY_identity_ca_private_key.pem  2048

Generation of self-signed certificate request for the Identity/Authentication CA

    openssl req -config identity_ca_files/identity_ca_openssl.cnf -new -key identity_ca_files/private/TESTONLY_identity_ca_private_key.pem -out identity_ca_files/identity_ca.csr

Generation of Certificate for the Identity/Authentication CA (signing of certificate request)

    openssl x509 -req -days 3650 -in  identity_ca_files/identity_ca.csr  -signkey identity_ca_files/private/TESTONLY_identity_ca_private_key.pem  -out TESTONLY_identity_ca_cert.pem

Initialization of Index and Serial number files

    touch identity_ca_files/index.txt echo "01" > identity_ca_files/serial


Generation of RTI Identity Certificate by the Identity CA (signing of certificate request)

    openssl ca -config identity_ca_files/identity_ca_openssl.cnf -days 3650 -in rti_connext_files/identity_rti_connext_dds.csr -out  rti_connext_files/TESTONLY_rti_connext_dds_identity_cert.pem		

Generation of TwinOaks Identity Certificate by the Identity CA (signing of certificate request)

    openssl ca -config identity_ca_files/identity_ca_openssl.cnf -days 3650 -in toc_coredx_dds_files/identity_toc_coredx_dds.csr -out  toc_coredx_dds_files/TESTONLY_toc_coredx_dds_identity_cert.pem
