#ifndef __Native_Client__
#define __Native_Client__

#include "FxQueue.h"
#include "DataCache.h"
#include "ClientState.h"
#include "SynchInvoker.h"
#include "IdGenerator.h"
#include "ProtocolVersion.h"

extern const string cExternalSynchCall;
extern const string cInternalASynchCall;

class CClient : public CFxQueue, public IReceiver
{
protected:

    CClient(CDataCache& cache, const string& connectionString);

public:
    virtual ~CClient();

public:
    bool Start();
    bool WaitForLogon(size_t timeoutInMilliseconds);
    HRESULT Shutdown();
    HRESULT Stop();
    const CDataCache& Cache()const;
    ISender& Sender();

public:
    void GetNetworkActivity(uint64* pLogicalBytesSent, uint64* pPhysicalBytesSent, uint64* pLogicalBytesReceived, uint64* pPhysicalBytesReceived);

public:
    const string NextId();
    const string NextId(const string& prefix);
    const string NextIdIfEmpty(const string& prefix, const string& externalId);

public:
    void RegisterWaiter(const type_info& info, const string& id, IWaiter* pWaiter);
    void ReleaseWaiter(const type_info& info, const string& id);

public:
    void SendTwoFactorResponse(const FxTwoFactorReason reason, const std::string& otp);
    CFxSessionInfo GetSessionInfo(const size_t timeoutInMilliseconds);
    CFxFileChunk GetFileChunk(const string& fileId, uint32 chunkId, const size_t timeoutInMilliseconds);
    string GetProtocolVersion()const;

public:
    virtual void VLogon(const CFxEventInfo& eventInfo, const string& protocolVersion, bool twofactor);
    virtual void VTwoFactorAuth(const CFxEventInfo& eventInfo, const FxTwoFactorReason reason, const std::string& text, const CDateTime& expire);
    virtual void VLogout(const CFxEventInfo& eventInfo, const FxLogoutReason reason, const string& description);
    virtual void VBusinessReject(const CFxEventInfo& eventInfo);
    virtual void VTick(const CFxEventInfo& eventInfo, const CFxQuote& quotes);
    virtual void VSessionInfo(const CFxEventInfo& eventInfo, CFxSessionInfo& sessionInfo);
    virtual void VAccountInfo(const CFxEventInfo& eventInfo, CFxAccountInfo& accountInfo);
    virtual void VGetCurrencies(const CFxEventInfo& eventInfo, const vector<CFxCurrencyInfo>& currencies);
    virtual void VGetSupportedSymbols(const CFxEventInfo& eventInfo, const vector<CFxSymbolInfo>& symbols);
    virtual void VSubscribeToQuotes(const CFxEventInfo& eventInfo, HRESULT status);
    virtual void VClosePositions(const CFxEventInfo& eventInfo, CFxClosePositionsResponse& response);
    virtual void VExecution(const CFxEventInfo& eventInfo, CFxExecutionReport& executionReport);
    virtual void VDataHistoryResponse(const CFxEventInfo& eventInfo, CFxDataHistoryResponse& response);
    virtual void VTradeHistoryResponse(const CFxEventInfo& eventInfo, CFxTradeHistoryResponse& response);
    virtual void VTradeHistoryReport(const CFxEventInfo& eventInfo, CFxTradeHistoryReport& report);
    virtual void VFileChunk(const CFxEventInfo& eventInfo, CFxFileChunk& chunk);
    virtual void VMetaInfoFile(const CFxEventInfo& eventInfo, string& file);
    virtual void VGetTradeTransactionReportsAndSubscribeToNotifications(const CFxEventInfo& info, const int32 curReportsNumber, const int32 totReportsNumber, const bool endOfStream);
    virtual void VTradeTransactionReport(const CFxEventInfo& info, CFxTradeTransactionReport& report);
    virtual void VUnsubscribeTradeTransactionReportsNotifications(const CFxEventInfo& info);
    virtual void VPositionReport(const CFxEventInfo& info, CFxPositionReport& positionReport);
    virtual void VNotify(const CFxEventInfo& eventInfo, const CNotification& notification);
    virtual void VQuotesHistoryResponse(const CFxEventInfo& eventInfo, const int version);

protected:
    bool CheckProtocolVersion(const CProtocolVersion& requiredVersion) const;
    virtual void AfterLogon();

private:
    CClient(const CClient&);
    CClient& operator = (const CClient&);

protected:
    ISender* m_sender;

private:
    HANDLE m_stateEvent;

private:
    CDataCache& m_cache;
    ClientState m_state;
    CIdGenerator m_idGenerator;
    CSynchInvoker m_synchInvoker;
    IConnection* m_connection;
    string m_protocolVersion;
    mutable CCriticalSection m_stateSynchronizer;
    mutable CCriticalSection m_dataSynchronizer;
    bool m_afterLogonInvoked;
};


#pragma region inline methods

inline const CDataCache& CClient::Cache() const
{
    return m_cache;
}

inline ISender& CClient::Sender()
{
    return *m_sender;
}

#pragma endregion

#endif
