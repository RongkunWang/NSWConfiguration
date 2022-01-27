#ifndef NSWCONFIGURATION_HW_OPCCLIENTPTR_H
#define NSWCONFIGURATION_HW_OPCCLIENTPTR_H

#include "NSWConfiguration/OpcClient.h"


namespace nsw {
  class OpcManager;
  namespace internal {
    /**
     * \brief Class with non-owning pointers to the OPC Client and Manager
     *
     * This wrapper is required since it can notify the OPC Manager whenever an
     * OPC connection goes out of scope.
     */
    class OpcClientPtr
    {
    public:
      /**
       * \brief Construct a new Opc Pointer object
       *
       * \param manager Pointer to the manager
       * \param client Pointer to the OPC client
       */
      OpcClientPtr(OpcManager* manager, OpcClient* client);

      /**
       * \brief Destroy the Opc Pointer object
       *
       * Notify the manager that this connection is not required anymore
       */
      ~OpcClientPtr();
      OpcClientPtr(const OpcClientPtr&) = default;
      OpcClientPtr(OpcClientPtr&&) = default;
      OpcClientPtr& operator=(OpcClientPtr&&) = default;
      OpcClientPtr& operator=(const OpcClientPtr&) = default;

      /**
       * \brief Access the OpcClient object
       *
       * \return OpcClient& OpcClient object
       */
      OpcClient& operator*();

      /**
       * \brief Access the OpcClient object
       *
       * \return OpcClient& OpcClient object
       */
      const OpcClient* operator->() const;

      /**
       * \brief Notify this class that the manager does not exist anymore
       *
       * Prevents this class from calling a dangling pointer. Will create runtime
       * errors if this object is used after calling that function.
       */
      void invalidateManager();

    private:
      OpcClient* m_data;      //<! non-owning pointer to the OpcClient
      OpcManager* m_manager;  //<! non-owning pointer to the OpcManager
    };
  }  // namespace internal
}  // namespace nsw

#endif
