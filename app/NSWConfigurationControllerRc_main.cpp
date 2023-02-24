// Main application to run in the partition

#include <string>
#include <memory>

#include <boost/program_options.hpp>

#include <ers/ers.h>
#include <ipc/core.h>
#include <RunControl/RunControl.h>

#include "NSWConfiguration/NSWConfigurationControllerRc.h"

int main(int argc, char** argv)
{
  try {
    IPCCore::init(argc, argv);
  } catch (daq::ipc::CannotInitialize& e) {
    ers::fatal(e);
    abort();
  } catch (daq::ipc::AlreadyInitialized& e) {
    ers::warning(e);
  }

  try {
    // Parser for the ItemCtrl
    daq::rc::CmdLineParser cmdParser(argc, argv, true);

    // Instantiate ItemCtrl with proper Controllable
    daq::rc::ItemCtrl itemCtrl(
      cmdParser, std::shared_ptr<daq::rc::Controllable>(new nsw::NSWConfigurationControllerRc()));
    itemCtrl.init();
    itemCtrl.run();
  } catch (daq::rc::CmdLineHelp& ex) {
    std::cout << ex.message() << std::endl;
  } catch (ers::Issue& ex) {
    ers::fatal(ex);
    return EXIT_FAILURE;
  } catch (boost::program_options::error& ex) {
    ers::fatal(daq::rc::CmdLineError(ERS_HERE, ex.what(), ex));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
