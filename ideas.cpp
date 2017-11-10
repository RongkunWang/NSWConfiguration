// Prototype ideas on how the classes will be used


// Get list of e-links/sectors/resources from OKS. Get the components to be configured

std::vector<std::string> components = {"A01","A02"}
std::vector<std::string> components = {"A01.VMM_L01_M01_01", "A01.VMM_L01_M01_02", "A01.VMM_L01_M01_02"};
std::string dbconnection = "json://myfile.json";

ConfigReader reader(dbconnection, components);
auto & config = reader.read();

// Here one could start a new thread for each Opc Server
ConfigSender sender(opcserver_address);

for (auto vmm: components){
    std::bitset bytestream = config.createbtyestream(vmm); // string?
    std::string vmmaddress = config.get_opc_address(vmm);
    sender.send(vmmaddress, bytestream);
}

VMM vmm(vmm_address);

MMSet vmms(); // Set of VMMs, for instance 

for (auto address : address_list){ // One can loop over e-links that belongs to swROD
  vmms.add(address);
}

auto config = reader.read_config(vmm_address) ; // Read configuration for specific FE.
config.set_attribute("attribute_name",value);   // Change some attribute from read configuration
vmm.write_config(config.bytestream()); // Write the configuration to VMM

auto current_config = vmm.read_config();

auto allconfig = reader.read_config() ; // Read all configuration, as bytestream?
vmms.write_config(allconfig.bytestream());           // Write all configuration to set of VMMs

// What happens if we store some configuration in OKS, and some in configuration database?
// Then we'd need access to OKS data from this class, or have to pass it as argument (as in set_attribute)
// We can access OKS in the DAQ-aware part of the code (swROD?) and send configuration as ptree to this class




