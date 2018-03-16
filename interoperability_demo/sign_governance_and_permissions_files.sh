#!/bin/bash

script_dir=`cd "\`dirname "$filename"\`"; pwd`

files_to_sign=(
	"$script_dir/governance/Governance_SC0_SecurityDisabled.xml"
	"$script_dir/governance/Governance_SC1_ProtectedDomain1.xml"
	"$script_dir/governance/Governance_SC2_ProtectedDomain2.xml"
	"$script_dir/governance/Governance_SC3_ProtectedDomain3.xml"
	"$script_dir/governance/Governance_SC4_ProtectedDomain4.xml"
	"$script_dir/governance/Governance_SC5_ProtectedDomain5.xml"
	"$script_dir/governance/Governance_SC6_ProtectedDomain6.xml"
	"$script_dir/permissions/Permissions_JoinDomain_ADLink.xml"
	"$script_dir/permissions/Permissions_JoinDomain_KDA.xml"
	"$script_dir/permissions/Permissions_JoinDomain_RTI.xml"
	"$script_dir/permissions/Permissions_JoinDomain_TOC.xml"
	"$script_dir/permissions/Permissions_JoinDomain_OCI.xml"
	"$script_dir/permissions/Permissions_TopicLevel_ADLink.xml"
	"$script_dir/permissions/Permissions_TopicLevel_KDA.xml"
	"$script_dir/permissions/Permissions_TopicLevel_RTI.xml"
	"$script_dir/permissions/Permissions_TopicLevel_TOC.xml"
	"$script_dir/permissions/Permissions_TopicLevel_OCI.xml")

permissions_ca_private_key=$script_dir/../permissions_ca_files/private/TESTONLY_permissions_ca_private_key.pem
permissions_ca_cert=$script_dir/../TESTONLY_permissions_ca_cert.pem

for file in ${files_to_sign[@]}
do
dirname=`dirname $file`
filename_wo_extension=`basename $file .xml`
mkdir -p $dirname/signed
openssl smime -sign -in $file -text \
	-out $dirname/signed/$filename_wo_extension.p7s \
	-signer $permissions_ca_cert \
	-inkey $permissions_ca_private_key
done
