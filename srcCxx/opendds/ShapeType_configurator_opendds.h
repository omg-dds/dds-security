#include "ShapeTypeTypeSupportImpl.h"

#include "dds/DCPS/GuardCondition.h"
#include "dds/DCPS/Marked_Default_Qos.h"
#include "dds/DCPS/Service_Participant.h"

#include "dds/DCPS/RTPS/RtpsDiscovery.h"

#include "dds/DCPS/transport/framework/TransportConfig_rch.h"
#include "dds/DCPS/transport/framework/TransportRegistry.h"
#include "dds/DCPS/transport/rtps_udp/RtpsUdp.h"

#define BOOLEAN_TRUE true
#define STATUS_MASK_NONE OpenDDS::DCPS::NO_STATUS_MASK

using org::omg::dds::demo::ShapeType;
using org::omg::dds::demo::ShapeTypeSeq;
using org::omg::dds::demo::ShapeTypeDataReader;
using org::omg::dds::demo::ShapeTypeDataWriter;
using org::omg::dds::demo::ShapeTypeTypeSupport;

using OpenDDS::DCPS::operator>=; // for Time_t

const char auth_ca_file[] = "file:../TESTONLY_identity_ca_cert.pem";
const char perm_ca_file[] = "file:../TESTONLY_permissions_ca_cert.pem";
const char id_cert_file[] = "file:../oci_opendds_dds_certs/TESTONLY_oci_opendds_dds_identity_cert.pem";
const char id_key_file[] = "file:../oci_opendds_dds_certs/private/TESTONLY_oci_opendds_dds_identity_private_key.pem";

const char DDSSEC_PROP_IDENTITY_CA[] = "dds.sec.auth.identity_ca";
const char DDSSEC_PROP_IDENTITY_CERT[] = "dds.sec.auth.identity_certificate";
const char DDSSEC_PROP_IDENTITY_PRIVKEY[] = "dds.sec.auth.private_key";
const char DDSSEC_PROP_PERM_CA[] = "dds.sec.access.permissions_ca";
const char DDSSEC_PROP_PERM_GOV_DOC[] = "dds.sec.access.governance";
const char DDSSEC_PROP_PERM_DOC[] = "dds.sec.access.permissions";

const DDS::Duration_t DURATION_INFINITE = {DDS::DURATION_INFINITE_SEC,
                                           DDS::DURATION_INFINITE_NSEC};

const DDS::ReturnCode_t DDS_RETCODE_OK = DDS::RETCODE_OK;
const DDS::InstanceHandle_t DDS_HANDLE_NIL = DDS::HANDLE_NIL;

DDS::Time_t operator+(const DDS::Time_t& lhs, const DDS::Time_t& rhs)
{
  using namespace OpenDDS::DCPS;
  return time_value_to_time(time_to_time_value(lhs) + time_to_time_value(rhs));
}

DDS::Duration_t operator-(const DDS::Duration_t& lhs,
                          const DDS::Duration_t& rhs)
{
  using namespace OpenDDS::DCPS;
  return time_value_to_duration(duration_to_time_value(lhs) -
                                duration_to_time_value(rhs));
}

namespace ShapeTypeConfigurator
{

void print_data(const ShapeType& shape)
{
  std::printf("%-10s %03d %03d [%d]\n", shape.color.in(), shape.x, shape.y,
              shape.shapesize);
}

void append(DDS::PropertySeq& props, const char* name, const char* value)
{
  const DDS::Property_t prop = {name, value, false /*propagate*/};
  const unsigned int len = props.length();
  props.length(len + 1);
  props[len] = prop;
}

DDS::DomainParticipant* create_participant(int domain, bool use_security,
                                           const char* governance,
                                           const char* permissions,
                                           bool /*enable_logging*/)
{
  using namespace OpenDDS::DCPS;
  using namespace OpenDDS::RTPS;
  TransportConfig_rch config =
    TransportRegistry::instance()->create_config("rtps_interop_demo");
  TransportInst_rch inst =
    TransportRegistry::instance()->create_inst("rtps_transport", "rtps_udp");
  config->instances_.push_back(inst);
  TransportRegistry::instance()->global_config(config);

  RtpsDiscovery_rch disc = make_rch<RtpsDiscovery>("RtpsDiscovery");
  TheServiceParticipant->add_discovery(static_rchandle_cast<Discovery>(disc));
  TheServiceParticipant->set_default_discovery(disc->key());

  DDS::DomainParticipantFactory_var factory = TheParticipantFactory;
  DDS::DomainParticipantQos part_qos;
  factory->get_default_participant_qos(part_qos);

  if (use_security) {
    TheServiceParticipant->set_security(true);
    DDS::PropertySeq& props = part_qos.property.value;
    append(props, DDSSEC_PROP_IDENTITY_CA, auth_ca_file);
    append(props, DDSSEC_PROP_IDENTITY_CERT, id_cert_file);
    append(props, DDSSEC_PROP_IDENTITY_PRIVKEY, id_key_file);
    append(props, DDSSEC_PROP_PERM_CA, perm_ca_file);
    static const std::string file("file:");
    append(props, DDSSEC_PROP_PERM_GOV_DOC, (file + governance).c_str());
    append(props, DDSSEC_PROP_PERM_DOC, (file + permissions).c_str());
  }

  return factory->create_participant(domain, part_qos, 0, 0);
}

void destroy_participant(DDS::DomainParticipant* dp)
{
  if (!dp) return;
  DDS::DomainParticipantFactory_var factory = TheParticipantFactory;
  dp->delete_contained_entities();
  factory->delete_participant(dp);
}

}
