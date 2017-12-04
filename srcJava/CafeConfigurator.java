import java.util.ArrayList;
import java.util.Collection;

import org.omg.dds.core.Property;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.PropertyQoS;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;


public class CafeConfigurator
   implements ShapeTypeConfigurator
{

   private static final String IDENTITY_CERTIFICATE =
         "../ad_vortex_dds_certs/TESTONLY_ad_vortex_dds_identity_cert.pem";
   private static final String PRIVATE_KEY =
         "../ad_vortex_dds_certs/private/TESTONLY_ad_vortex_dds_identity_private_key.pem";
   private static final String IDENTITY_CA =
         "../TESTONLY_identity_ca_cert.pem";
   private static final String PERMISSIONS_CA =
         "../TESTONLY_permissions_ca_cert.pem";
   
   
   private ServiceEnvironment env;
   private boolean use_security; 
   
   public CafeConfigurator(boolean use_security, boolean logging)
   {
      System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
            "com.prismtech.cafe.core.ServiceEnvironmentImpl");
      this.env = ServiceEnvironment.createInstance(
            CafeConfigurator.class.getClassLoader());
      this.use_security = use_security;
      
      if (this.use_security)
      {
         System.setProperty("ddsi.security.authClassName",
               "com.prismtech.cafe.security.plugins.BuiltinAuthenticationImpl");
         System.setProperty("ddsi.security.accessCtrlClassName",
               "com.prismtech.cafe.security.plugins.BuiltinAccessControlImpl");
         System.setProperty("ddsi.security.cryptoClassName",
               "com.prismtech.cafe.dds.security.BuiltinCrypto");
      }
      System.setProperty("log.level", (logging) ? "DEBUG" : "ERROR");       
   }
   
   @Override
   public ServiceEnvironment getServiceEnvironment()
   {
     return this.env;
   }

   @Override
   public DomainParticipant create_participant(
         int domain_id,
         boolean use_security,
         boolean logging,
         String governance_file,
         String permissions_file)
   {
      DomainParticipantFactory participant_factory = DomainParticipantFactory.getInstance(this.env);
      
      PropertyQoS propertyQoS = PolicyFactory.getPolicyFactory(env).PropertyQoS();
      
      if (use_security) 
      {
         Collection<Property> properties = new ArrayList<Property>();

         for(String name : System.getProperties().stringPropertyNames())
         {
            if(name.startsWith("dds.sec"))
            {
               properties.add (new Property(
                     name, 
                     System.getProperty(name)));
            }
         }
         
         if(System.getProperty("dds.sec.auth.identity_certificate") == null)
         {
            properties.add(new Property(
                  "dds.sec.auth.identity_certificate",
                  IDENTITY_CERTIFICATE));
         }
         
         if(System.getProperty("dds.sec.auth.private_key") == null)
         {
            properties.add(new Property(
                  "dds.sec.auth.private_key",
                  PRIVATE_KEY));            
         }

         
         if(System.getProperty("dds.sec.auth.identity_ca") == null)
         {
            properties.add(new Property(
                  "dds.sec.auth.identity_ca",
                  IDENTITY_CA));
         }
         
         if(System.getProperty("dds.sec.access.permissions_ca") == null)
         {
            properties.add(new Property(
                  "dds.sec.access.permissions_ca",
                  PERMISSIONS_CA));
         }
         
         if(System.getProperty("dds.sec.access.governance") == null)
         {
            properties.add(new Property(
                  "dds.sec.access.governance",
                  governance_file));
         }
         
         if(System.getProperty("dds.sec.access.permissions") == null)
         {
            properties.add(new Property(
                  "dds.sec.access.permissions",
                  permissions_file));
         }
         
         propertyQoS = propertyQoS.withValue(properties);
      }
      
      return participant_factory.createParticipant(
            domain_id,
            participant_factory.getDefaultParticipantQos().withPolicy(propertyQoS),
            null,
            (Collection<Class<? extends Status>>)null);
   }

   @Override
   public void print_data(ShapeType data)
   {
      System.out.println(data.color + "("+data.shapesize+")@"+data.x+"."+data.y);
   }

}
