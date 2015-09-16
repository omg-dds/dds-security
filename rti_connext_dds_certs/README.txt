# These commands assume the CWD is the repository root (dds-security)

# Generation of RTI Identity private key:
openssl genrsa -out rti_connext_files/private/TESTONLY_rti_connext_dds_identity_private_key.pem  2048

# Generation of certificate request to be signed by the Identity CA:
openssl req -config rti_connext_files/rti_connext_dds_openssl.cnf -new -key rti_connext_files/private/TESTONLY_rti_connext_dds_identity_private_key.pem -out rti_connext_files/identity_rti_connext_dds.csr

# Generation of RTI Identity Certificate by the Identity CA (signing of certificate request)
openssl ca -config identity_ca_files/identity_ca_openssl.cnf -days 3650 -in rti_connext_files/identity_rti_connext_dds.csr -out  rti_connext_files/TESTONLY_rti_connext_dds_identity_cert.pem	

# Signing a governance document:
openssl smime -sign -in TESTONLY_governance.xml -text -out  TESTONLY_governance_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	

# Signing the permission documents:
openssl smime -sign -in rti_connext_files/TESTONLY_rti_connext_dds_permissions_join.xml -text -out  rti_connext_files/TESTONLY_rti_connext_dds_permissions_join_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_files/TESTONLY_rti_connext_dds_permissions_read.xml -text -out  rti_connext_files/TESTONLY_rti_connext_dds_permissions_read_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_files/TESTONLY_rti_connext_dds_permissions_write.xml -text -out  rti_connext_files/TESTONLY_rti_connext_dds_permissions_write_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_files/TESTONLY_rti_connext_dds_permissions_readwrite.xml -text -out  rti_connext_files/TESTONLY_rti_connext_dds_permissions_readwrite_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	
openssl smime -sign -in rti_connext_files/TESTONLY_rti_connext_dds_permissions_allowall.xml -text -out  rti_connext_files/TESTONLY_rti_connext_dds_permissions_allowall_signed.p7s -signer TESTONLY_permissions_ca_cert.pem -inkey permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem	



