import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.domain.DomainParticipant;

public interface ShapeTypeConfigurator
{

   public ServiceEnvironment getServiceEnvironment();
   
   public DomainParticipant create_participant(
         int domain_id,
         boolean use_security,
         boolean logging,
         String governance_file,
         String permissions_file);

   public void print_data(ShapeType data);
   
   
}
