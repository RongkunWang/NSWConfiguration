#ifndef NSWCONFIGURATION_ROCCONFIG_H
#define NSWCONFIGURATION_ROCCONFIG_H

#include "NSWConfiguration/I2cMasterConfig.h"

namespace nsw {

  /**
   * \brief Wrapper that holds both the analog and the digital configuration object of the ROC
   */
  class ROCConfig
  {
  public:
    ROCConfig(I2cMasterConfig analog, I2cMasterConfig digital) :
      m_analog{std::move(analog)}, m_digital{std::move(digital)}
    {}
    /**
     * \brief Get the analog part of the configuration
     *
     * \return const I2cMasterConfig& Analog configuration
     */
    [[nodiscard]] const I2cMasterConfig& getAnalog() const { return m_analog; }

    /**
     * \brief Get the analog part of the configuration
     *
     * \return I2cMasterConfig& Analog configuration
     */
    I2cMasterConfig& getAnalog() { return m_analog; }

    /**
     * \brief Get the digital part of the configuration
     *
     * \return const I2cMasterConfig& Digital configuration
     */
    [[nodiscard]] const I2cMasterConfig& getDigital() const { return m_digital; }

    /**
     * \brief Get the digital part of the configuration
     *
     * \return I2cMasterConfig& Digital configuration
     */
    I2cMasterConfig& getDigital() { return m_digital; }

  private:
    I2cMasterConfig m_analog;   //!< I2cMasterConfig for the analog part of the ROC
    I2cMasterConfig m_digital;  //!< I2cMasterConfig for the digital part of the ROC
  };

}  // namespace nsw

#endif