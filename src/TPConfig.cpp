#include <string>

#include "boost/optional.hpp"

#include "NSWConfiguration/TPConfig.h"


nsw::TPConfig::TPConfig(ptree config):
FEConfig(config)
{
	for (int i = 0; i < NUM_REGISTER_FILES; i++)
	{
		std::string mastername = "i2cMaster" + std::to_string(i);
		if (config.find(mastername) != config.not_found())
		{
			// make map pair or register file object (alloc memory first) and it's index name
			ERS_DEBUG(3, "creating object: m_registerFiles[" << i << "] : " << registerFilesNamesArr[i]);

			m_I2cMasterConfigPtrArr[i] = new I2cMasterConfig(config.get_child(registerFilesNamesArr[i]), registerFilesNamesArr[i], registerFilesArr[i]);
			m_registerFiles.insert(std::make_pair(registerFilesNamesArr[i], m_I2cMasterConfigPtrArr[i]));
		}
		else
		{
			// register file name was declred but not found in ptree
			ERS_DEBUG(3, "mastername[" << mastername << "] : " << registerFilesNamesArr[i] << " not found!!");
		}
	}
}

void nsw::TPConfig::setRegisterValue(std::string master, std::string slave, uint32_t value, std::string register_name)
{
	// set value of a register which salve of is on a register file (master)
	m_registerFiles[master]->setRegisterValue(slave, register_name, value);

	return;
}

uint32_t nsw::TPConfig::getRegisterValue(std::string master, std::string slave, std::string register_name)
{
	// get value of a register which salve of is on a register file (master)
	return m_registerFiles[master]->getRegisterValue(slave, register_name);
}

void nsw::TPConfig::dump()
{
	for (int i = 0; i < NUM_REGISTER_FILES; i++)
	{
		m_registerFiles[registerFilesNamesArr[i]]->dump();
	}

	return;
}

nsw::TPConfig::~TPConfig( )
{
	// deallocate all memory taken in the constructor
	for (int i = 0; i < NUM_REGISTER_FILES; i++)
	{
		ERS_DEBUG(3, "deallocate object: m_registerFiles[" << i << "] : " << registerFilesNamesArr[i]  << " object address = " << static_cast<void*>(m_registerFiles[registerFilesNamesArr[i]]));
		delete m_registerFiles[registerFilesNamesArr[i]];
	}

	return;
}

