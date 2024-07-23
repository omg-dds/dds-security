
//#include "ShapeType.h"
#include "ShapeTypePubSubTypes.h"

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <condition_variable>
#include <iostream>

const char auth_ca_file[] = "file://../TESTONLY_identity_ca_cert.pem";
const char perm_ca_file[] = "file://../TESTONLY_permissions_ca_cert.pem";
const char id_cert_file[] = "file://../eprosima_fastrtps_certs/TESTONLY_eprosima_fastrtps_identity_cert.pem";
const char id_key_file[] = "file://../eprosima_fastrtps_certs/private/TESTONLY_eprosima_fastrtps_identity_private_key.pem";

// set these environment variables to override the defaults above:
const char ID_CERT[] = "OPENDDS_ID_CERTIFICATE";
const char ID_KEY[] = "OPENDDS_ID_PRIVATE_KEY";

const char DDSSEC_PROP_IDENTITY_CA[] = "dds.sec.auth.builtin.PKI-DH.identity_ca";
const char DDSSEC_PROP_IDENTITY_CERT[] = "dds.sec.auth.builtin.PKI-DH.identity_certificate";
const char DDSSEC_PROP_IDENTITY_PRIVKEY[] = "dds.sec.auth.builtin.PKI-DH.private_key";
const char DDSSEC_PROP_PERM_CA[] = "dds.sec.access.builtin.Access-Permissions.permissions_ca";
const char DDSSEC_PROP_PERM_GOV_DOC[] = "dds.sec.access.builtin.Access-Permissions.governance";
const char DDSSEC_PROP_PERM_DOC[] = "dds.sec.access.builtin.Access-Permissions.permissions";

const char EPROSIMA_AUTH_PLUGIN[] = "dds.sec.auth.plugin";
const char EPROSIMA_ACCESS_PLUGIN[] = "dds.sec.access.plugin";
const char EPROSIMA_CRYPTO_PLUGIN[] = "dds.sec.crypto.plugin";
const char auth_plugin[] = "builtin.PKI-DH";
const char access_plugin[] = "builtin.Access-Permissions";
const char crypto_plugin[] = "builtin.AES-GCM-GMAC";

std::string use_env(const char* key, const char* default_val)
{
  const char* const env = getenv(key);
  return env ? env : default_val;
}

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

static std::ostream& operator<<(std::ostream &os, const ShapeType &shape)
{
    os << shape.color() << " @ X: " << shape.x() << ", Y: " << shape.y() << ", size: " << shape.shapesize();
    return os;
}

class ShapeSubscriber {
public:
	ShapeSubscriber() : cv_guard(nullptr), m_listener(this), mp_participant(nullptr), mp_subscriber(nullptr) {}
	virtual ~ShapeSubscriber() {}
    void set_read_guard(std::condition_variable *guard) { cv_guard = guard; }
    bool take_next_sample(ShapeType &shape, SampleInfo_t &info)
    {
        if (m_listener.read_sub == nullptr) return false;
        return m_listener.read_sub->takeNextData((void*)&shape, &info);
    }
private:
    std::condition_variable *cv_guard;
public:
	class SubListener : public SubscriberListener
	{
	public:
		SubListener(ShapeSubscriber *parent) : read_sub(nullptr), m_parent(parent) {};
		~SubListener() {};
		void onSubscriptionMatched(Subscriber* sub, MatchingInfo& info) {}
		void onNewDataMessage(Subscriber* sub)
        {
            read_sub = sub;
            m_parent->cv_guard->notify_one();
        }
        Subscriber *read_sub;
        ShapeSubscriber *m_parent;
	}m_listener;
	Participant* mp_participant;
	Subscriber* mp_subscriber;
};

class ShapePublisher {
public:
	ShapePublisher() : mp_participant(nullptr), mp_publisher(nullptr) {}
	virtual ~ShapePublisher() {}
	//!Publish a sample
	bool write(void* data)
    {
        return mp_publisher->write(data);
    }
	class PubListener : public PublisherListener
	{
	public:
		PubListener() {};
		~PubListener() {};
		void onPublicationMatched(Publisher* pub, MatchingInfo& info) {}
	}m_listener;
	Participant* mp_participant;
	Publisher* mp_publisher;
private:
};

class ShapeTypeConfigurator {
    public:
    static Participant *create_participant(
            int domain_id,
            bool use_security,
            const char *governance_file,
            const char *permissions_file,
            const char *key_establishment_algorithm,
            bool enable_logging)
    {
        Participant *participant = NULL;

        ParticipantAttributes PParam;

        if (enable_logging)
        {
            Log::SetVerbosity(Log::Kind::Info);
        }

        if (use_security)
        {
            std::string gov_file = std::string("file://") + governance_file;
            std::string perm_file = std::string("file://") + permissions_file;
            PropertyPolicy participant_property_policy;
            auto& properties = participant_property_policy.properties();
            properties.emplace_back(EPROSIMA_ACCESS_PLUGIN, access_plugin);
            properties.emplace_back(EPROSIMA_AUTH_PLUGIN, auth_plugin);
            properties.emplace_back(EPROSIMA_CRYPTO_PLUGIN, crypto_plugin);
            properties.emplace_back(DDSSEC_PROP_IDENTITY_CA, auth_ca_file);
            properties.emplace_back(DDSSEC_PROP_IDENTITY_CERT, use_env(ID_CERT, id_cert_file));
            properties.emplace_back(DDSSEC_PROP_IDENTITY_PRIVKEY, use_env(ID_KEY, id_key_file));
            properties.emplace_back(DDSSEC_PROP_PERM_CA, perm_ca_file);
            properties.emplace_back(DDSSEC_PROP_PERM_GOV_DOC, gov_file);
            properties.emplace_back(DDSSEC_PROP_PERM_DOC, perm_file);
            PParam.rtps.properties = participant_property_policy;

            std::cout << std::string(DDSSEC_PROP_IDENTITY_CA) << ": " << std::string(auth_ca_file) << std::endl;
            std::cout << std::string(DDSSEC_PROP_IDENTITY_CERT) << ": " << std::string(id_cert_file) << std::endl;
            std::cout << std::string(DDSSEC_PROP_IDENTITY_PRIVKEY) << ": " << std::string(id_key_file) << std::endl;
            std::cout << std::string(DDSSEC_PROP_PERM_CA) << ": " << std::string(perm_ca_file) << std::endl;
            std::cout << std::string(DDSSEC_PROP_PERM_GOV_DOC) << ": " << gov_file << std::endl;
            std::cout << std::string(DDSSEC_PROP_PERM_DOC) << ": " << perm_file << std::endl;
        }

        PParam.rtps.builtin.domainId = domain_id;

        participant = Domain::createParticipant(PParam);

        if ( participant == NULL ) {
            printf("create_participant error for domain_id '%d'\n", domain_id);
            return NULL;
        }

        return participant;
    }

    static void destroy_participant(Participant *participant)
    {
        Domain::removeParticipant(participant);
    }
};
