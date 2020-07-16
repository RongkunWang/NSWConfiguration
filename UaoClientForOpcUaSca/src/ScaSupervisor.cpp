

#include <iostream>
#include <ScaSupervisor.h>
#include <uaclient/uasession.h>
#include <stdexcept>
#include <UaoClientForOpcUaScaArrayTools.h>
#include <UaoClientForOpcUaScaUaoExceptions.h>

namespace UaoClientForOpcUaSca
{


ScaSupervisor::
ScaSupervisor
(
    UaSession* session,
    UaNodeId objId
) :
    m_session(session),
    m_objId (objId)
{

}



OpcUa_UInt16 ScaSupervisor::readNumberOffline (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".numberOffline"), m_objId.namespaceIndex() );

    nodesToRead.create(1);
    nodeId.copyTo( &nodesToRead[0].NodeId );
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;

    dataValues.create (1);

    UaStatus status = m_session->read(
                          ss,
                          0 /*max age*/,
                          OpcUa_TimestampsToReturn_Both,
                          nodesToRead,
                          dataValues,
                          diagnosticInfos
                      );
    if (status.isBad())
        throw Exceptions::BadStatusCode("OPC-UA read failed", status.statusCode());
    if (out_status)
        *out_status = dataValues[0].StatusCode;
    else
    {
        if (! UaStatus(dataValues[0].StatusCode).isGood())
            throw Exceptions::BadStatusCode("OPC-UA read: variable status is not good", dataValues[0].StatusCode );
    }

    OpcUa_UInt16 out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toUInt16 (out);
    if (! conversionStatus.isGood())
    {
        throw std::runtime_error(std::string("OPC-UA read: read succeeded but conversion to native type failed (was it NULL value?): ") + UaStatus(dataValues[0].StatusCode).toString().toUtf8() );
    }


    if (sourceTimeStamp)
        *sourceTimeStamp = dataValues[0].SourceTimestamp;
    if (serverTimeStamp)
        *serverTimeStamp = dataValues[0].ServerTimestamp;

    return out;
}



}


