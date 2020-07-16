

#include <iostream>
#include <DacOutput.h>
#include <uaclient/uasession.h>
#include <stdexcept>
#include <UaoClientForOpcUaScaArrayTools.h>
#include <UaoClientForOpcUaScaUaoExceptions.h>

namespace UaoClientForOpcUaSca
{


DacOutput::
DacOutput
(
    UaSession* session,
    UaNodeId objId
) :
    m_session(session),
    m_objId (objId)
{

}



OpcUa_Double DacOutput::readVoltage (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".voltage"), m_objId.namespaceIndex() );

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

    OpcUa_Double out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toDouble (out);
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


void  DacOutput::writeVoltage (
    OpcUa_Double & data,
    UaStatus                                 *out_status)
{
    ServiceSettings   ss;
    UaWriteValues    nodesToWrite;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;
    UaStatusCodeArray results;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".voltage"), m_objId.namespaceIndex()  );

    nodesToWrite.create(1);
    nodeId.copyTo( &nodesToWrite[0].NodeId );
    nodesToWrite[0].AttributeId = OpcUa_Attributes_Value;

    UaVariant v ( data );


    dataValues.create (1);
    v.copyTo( &nodesToWrite[0].Value.Value );

    UaStatus status = m_session->write(
                          ss,
                          nodesToWrite,
                          results,
                          diagnosticInfos
                      );
    if (out_status)
    {
        *out_status = status;
    }
    else
    {
        if (status.isBad())
            throw Exceptions::BadStatusCode("OPC-UA write failed", status.statusCode() );
        if (results[0] != OpcUa_Good)
            throw Exceptions::BadStatusCode ("OPC-UA write failed", results[0] );
    }

}


}


