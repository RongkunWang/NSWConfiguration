

#include <iostream>
#include <SCA.h>
#include <uaclient/uasession.h>
#include <stdexcept>
#include <UaoClientForOpcUaScaArrayTools.h>
#include <UaoClientForOpcUaScaUaoExceptions.h>

namespace UaoClientForOpcUaSca
{


SCA::
SCA
(
    UaSession* session,
    UaNodeId objId
) :
    m_session(session),
    m_objId (objId)
{

}



UaString SCA::readAddress (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".address"), m_objId.namespaceIndex() );

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

OpcUa_Boolean SCA::readOnline (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".online"), m_objId.namespaceIndex() );

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

    OpcUa_Boolean out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toBool (out);
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

OpcUa_UInt32 SCA::readId (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".id"), m_objId.namespaceIndex() );

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

    OpcUa_UInt32 out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toUInt32 (out);
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

OpcUa_UInt64 SCA::readNumberRequests (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".numberRequests"), m_objId.namespaceIndex() );

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

    OpcUa_UInt64 out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toUInt64 (out);
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

OpcUa_UInt64 SCA::readNumberReplies (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".numberReplies"), m_objId.namespaceIndex() );

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

    OpcUa_UInt64 out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toUInt64 (out);
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

OpcUa_UInt64 SCA::readLastReplySecondsAgo (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".lastReplySecondsAgo"), m_objId.namespaceIndex() );

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

    OpcUa_UInt64 out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toUInt64 (out);
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

OpcUa_Double SCA::readRequestRate (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".requestRate"), m_objId.namespaceIndex() );

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

OpcUa_UInt64 SCA::readNumberLostReplies (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".numberLostReplies"), m_objId.namespaceIndex() );

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

    OpcUa_UInt64 out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toUInt64 (out);
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

OpcUa_Float SCA::readLostRepliesRate (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".lostRepliesRate"), m_objId.namespaceIndex() );

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

    OpcUa_Float out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toFloat (out);
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


void SCA::reset (

)
{

    ServiceSettings serviceSettings;
    CallIn callRequest;
    CallOut co;

    callRequest.objectId = m_objId;
    callRequest.methodId = UaNodeId( UaString(m_objId.identifierString()) + UaString(".reset"), 2 );


    UaVariant v;




    UaStatus status =
        m_session->call(
            serviceSettings,
            callRequest,
            co
        );
    if (status.isBad())
        throw Exceptions::BadStatusCode("In OPC-UA call", status.statusCode());




}


void SCA::ping (
    OpcUa_Boolean & out_pong
)
{

    ServiceSettings serviceSettings;
    CallIn callRequest;
    CallOut co;

    callRequest.objectId = m_objId;
    callRequest.methodId = UaNodeId( UaString(m_objId.identifierString()) + UaString(".ping"), 2 );


    UaVariant v;




    UaStatus status =
        m_session->call(
            serviceSettings,
            callRequest,
            co
        );
    if (status.isBad())
        throw Exceptions::BadStatusCode("In OPC-UA call", status.statusCode());


    v = co.outputArguments[0];

    v.toBool (out_pong);



}



}


