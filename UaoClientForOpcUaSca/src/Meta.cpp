

#include <iostream>
#include <Meta.h>
#include <uaclient/uasession.h>
#include <stdexcept>
#include <UaoClientForOpcUaScaArrayTools.h>
#include <UaoClientForOpcUaScaUaoExceptions.h>

namespace UaoClientForOpcUaSca
{


Meta::
Meta
(
    UaSession* session,
    UaNodeId objId
) :
    m_session(session),
    m_objId (objId)
{

}



UaString Meta::readVersionString (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".versionString"), m_objId.namespaceIndex() );

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

    UaString out;


    out = UaVariant(dataValues[0].Value).toString();


    if (sourceTimeStamp)
        *sourceTimeStamp = dataValues[0].SourceTimestamp;
    if (serverTimeStamp)
        *serverTimeStamp = dataValues[0].ServerTimestamp;

    return out;
}



}


