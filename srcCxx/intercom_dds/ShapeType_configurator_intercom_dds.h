#include "ShapeType.h"
#include <ccpp_dds_dcps.h>
#include <InterCOM/JsonSerializer.h>
#include <string>

namespace {
  const bool BOOLEAN_TRUE = true;
  const bool BOOLEAN_FALSE = false;
  const DDS::StatusMask STATUS_MASK_NONE = 0;
  const DDS::InstanceHandle_t DDS_HANDLE_NIL = DDS::HANDLE_NIL;
}

using namespace DDS;

std::ostream& operator<<( std::ostream& stream, const ShapeType& shape )
{
  intercom::dcps::cts::JsonMarshal json( stream, true );
  json.io( shape );
  return stream;
}

inline Time_t normalizeTime( const Time_t& operand )
{
   if ( operand == TIME_INVALID || operand == TIME_INFINITE ) return operand;
   Time_t newValue;
   newValue.sec = operand.sec + (Long)(operand.nanosec/1000000000);
   newValue.nanosec = operand.nanosec % 1000000000;
   return newValue;
}

inline Duration_t operator-(const Duration_t& lhs, const Duration_t& rhs)
{
   Duration_t d;

   d.sec = lhs.sec - rhs.sec;
   if(lhs.nanosec < rhs.nanosec)
   {
      d.sec--;
      d.nanosec = 1000000000 + lhs.nanosec - rhs.nanosec;
   }
   else
   {
      d.nanosec = lhs.nanosec - rhs.nanosec;
   }

   return d;
}

inline Time_t operator+( const Time_t& operand1, const Time_t& operand2 )
{
   Time_t rval;
   if ( operand1 == TIME_INFINITE || operand2 == TIME_INFINITE )
   {
      rval = TIME_INFINITE;
   }
   else if ( operand1 == TIME_INVALID )
   {
      rval = TIME_INVALID;
   }
   else
   {
      Time_t norm_o1 = normalizeTime(operand1);
      Time_t norm_o2 = normalizeTime(operand2);
      rval.sec = norm_o1.sec + norm_o2.sec;
      rval.nanosec = norm_o1.nanosec + norm_o2.nanosec;
      rval = normalizeTime(rval);
   }
   return rval;
}

class ShapeTypeConfigurator
{
   public:
      static DomainParticipant *create_participant(
      int domain_id,
      bool use_security,
      const char *governance_file,
      const char *permissions_file,
      bool enable_logging)
      {
         DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
         DomainParticipantQos participantQos;
         factory->get_default_participant_qos(participantQos);

         std::string governance_str = "file:";
         governance_str += governance_file;
         std::string permissions_str = "file:";
         permissions_str += permissions_file;

         if (!use_security)
           printf("TODO: Handle !use_security...\n");

         participantQos.property.value.push_back( Property_t("InterCOM.sec.log_file", "intercom_security_log.txt", false ) );
         participantQos.property.value.push_back( Property_t("InterCOM.sec.distribute", "false", false ) );

         participantQos.property.value.push_back(Property_t("dds.sec.auth.identity_ca", "file:../TESTONLY_identity_ca_cert.pem", false));
         participantQos.property.value.push_back(Property_t("dds.sec.auth.identity_certificate", "file:../kda_intercom_dds_certs/TESTONLY_kda_intercom_dds_identity_cert.pem", false));
         participantQos.property.value.push_back(Property_t("dds.sec.auth.private_key", "file:../kda_intercom_dds_certs/private/TESTONLY_kda_intercom_dds_identity_cert.key.pem", false));
         participantQos.property.value.push_back(Property_t("dds.sec.auth.password", "intercom", false));
         participantQos.property.value.push_back(Property_t("dds.sec.access.permissions_ca", "file:../TESTONLY_permissions_ca_cert.pem", false));
         participantQos.property.value.push_back(Property_t("dds.sec.access.governance", governance_str.c_str(), false));
         participantQos.property.value.push_back(Property_t("dds.sec.access.permissions", permissions_str.c_str(), false));

         DomainParticipant* participant = factory->create_participant(domain_id, participantQos, NULL, 0);
         if ( participant == NULL )
         {
            printf("Could not create participant error for domain_id '%d'\n", domain_id);
         }

         return participant;
      }

      static void destroy_participant( DomainParticipant * dp )
      {
        DDS::DomainParticipantFactory* dpf = DomainParticipantFactory::get_instance();
        if (dp) dp->delete_contained_entities();
        if (dp) dpf->delete_participant(dp);
      }
};
