
#include "ShapeType.h"
#include "ShapeTypeSupport.h"
#include "ndds/ndds_namespace_cpp.h"

using namespace DDS;

class ShapeTypeConfigurator {
    public:
    static DomainParticipant *create_participant(
            int domain_id,
            bool use_security,
            const char *governance_file,
            const char *permissions_file,
            const char *key_agreement_algorithm,
            bool enable_logging)
    {
        DomainParticipant *participant = NULL;

        participant = TheParticipantFactory->create_participant(
            domain_id, PARTICIPANT_QOS_DEFAULT,
            NULL /* listener */, STATUS_MASK_NONE);
        if ( participant == NULL ) {
            printf("create_participant error for domain_id '%d'\n", domain_id);
            return NULL;
        }

        return participant;
    }
};
