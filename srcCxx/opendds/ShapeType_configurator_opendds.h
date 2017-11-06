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

DDS::DomainParticipant* create_participant(int domain, bool use_security,
                                           const char* /*governance*/,
                                           const char* /*permissions*/)
{
  if (use_security) {
    printf("ERROR: security options are not yet supported\n");
    return 0;
  }

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
  return factory->create_participant(domain, PARTICIPANT_QOS_DEFAULT, 0, 0);
}

void destroy_participant(DDS::DomainParticipant* dp)
{
  if (!dp) return;
  DDS::DomainParticipantFactory_var factory = TheParticipantFactory;
  dp->delete_contained_entities();
  factory->delete_participant(dp);
}

}
