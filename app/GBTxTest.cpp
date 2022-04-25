#include <iostream>
#include "NSWConfiguration/GBTxConfig.h"
using namespace nsw;


int main ()
{
    std::cout << "GBTX Config Test"<<std::endl;
    std::string mapFile   ="/afs/cern.ch/user/a/aawhite/nsw/NSWConfiguration/data/gbtxRegisterMap.csv";
    std::string configJson="/afs/cern.ch/user/a/aawhite/nsw/NSWConfiguration/data/gbtxConfig.json";

    bool verbose = 0;
    nsw::gbtxConfig* g = new nsw::gbtxConfig(verbose);

    // Config registers with file
    // nsw::gbtxConfig* g = new nsw::gbtxConfig(mapFile,verbose);

    g->debugPrintRegisterMaps();
    g->setConfigFromPTree(*(g->loadJsonToPtree(configJson)));

    // g->debugPrintRegisterMaps();
    g->debugPrintConfig();

}
