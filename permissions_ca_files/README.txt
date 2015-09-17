# Generation of Permissions CA private key:
openssl genrsa -out permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem  2048

# Generation of self-signed certificate request for the Permissions CA
openssl req -config permissions_ca_files/permissions_ca_openssl.cnf -new -key permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem -out permissions_ca_files/permissions_ca.csr

# Generation of Certificate for the Permissions CA (signing of certificate request)
openssl x509 -req -days 3650 -in  permissions_ca_files/permissions_ca.csr  -signkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem  -out TESTONLY_permissions_ca_cert.pem

# Initialization of Index and Serial number files
touch permissions_ca_files/index.txt
echo "01" > permissions_ca_files/serial

# Signing the governance document
openssl smime -sign -in TESTONLY_governance.xml -text -out  TESTONLY_governance_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	

# Signing the permissions documents for RTI
openssl smime -sign -in rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_join.xml -text -out  rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_join_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_read.xml -text -out  rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_read_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_write.xml -text -out  rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_write_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_readwrite.xml -text -out  rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_readwrite_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_allowall.xml -text -out  rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_allowall_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	

# Signing the permissions documents for TwinOaks computing
openssl smime -sign -in twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_join.xml -text -out  twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_join_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_read.xml -text -out  twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_read_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_write.xml -text -out  twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_write_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_readwrite.xml -text -out  twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_readwrite_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_allowall.xml -text -out  twinoaks_coredx_certs/TESTONLY_twinoaks_coredx_permissions_allowall_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	

