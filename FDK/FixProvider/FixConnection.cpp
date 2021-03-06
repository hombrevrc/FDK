#include "stdafx.h"
#include "resource.h"
#include "FixConnection.h"
#include "NullLogFactory.h"
#include "FixDialect.h"
#include "Conversion.h"

namespace
{
    const CFixVersion cExt19(1, 9);
}

namespace
{
    const FIX::UtcTimeStamp cZeroTime(time_t(0));
    map<string, MessageHandler> gMessageTypeToHandler;

    std::wstring& Utf8ToStd(std::wstring& dest, const std::string& src)
    {
        if (!src.length())
        {
            dest = L"";
            return dest;
        }

        int result = MultiByteToWideChar(CP_UTF8, 0, src.data(), src.length(), 0, 0);

        if (!result)
            throw std::logic_error("Invalid string to convert from UTF-8");

        dest.resize(result);

        result = MultiByteToWideChar(CP_UTF8, 0, src.data(), src.length(), const_cast<wchar_t*>(dest.data()), dest.length());

        if (!result)
            throw std::logic_error("Invalid string to convert from UTF-8");

        return dest;
    }

    std::wstring StringToWString(const std::string& s)
    {
        std::wstring utf8String;
        std::wstring result = Utf8ToStd(utf8String, s);

        return result;
    }
}

namespace
{
    const string cFixVersion = "FixVersion";
    const string cAddress = "Address";
    const string cPort = "Port";
    const string cSenderCompId = "SenderCompId";
    const string cTargetCompId = "TargetCompId";
    const string cUsername = "Username";
    const string cPassword = "Password";
    const string cDeviceId = "DeviceId";
    const string cAppId = "AppId";
    const string cAppSessionId = "AppSessionId";
    const string cSecureConnection = "SecureConnection";
    const string cFixLogDirectory = "FixLogDirectory";
    const string cFixEventsFileName = "FixEventsFileName";
    const string cFixMessagesFileName = "FixMessagesFileName";
    const string cProtocolVersion = "ProtocolVersion";
    const string cDecodeLogFixMessages = "DecodeLogFixMessages";
    const string cExcludeMessagesFromLogs = "ExcludeMessagesFromLogs";
    const string cEnableNetworkStatistics = "EnableNetworkStatistics";
    const string cProxyType = "ProxyType";
    const string cProxyAddress = "ProxyAddress";
    const string cProxyPort = "ProxyPort";
    const string cProxyUserName = "ProxyUserName";
    const string cProxyPassword = "ProxyPassword";
}

void CFixConnection::InitializeMessageHandlers()
{
    gMessageTypeToHandler[FIX::MsgType_Heartbeat] = &CFixConnection::OnEmpty;
    gMessageTypeToHandler[FIX::MsgType_TwoFactorLogon] = reinterpret_cast<MessageHandler>(&CFixConnection::OnTwoFactorAuth);
    gMessageTypeToHandler[FIX::MsgType_TradingSessionStatus] = reinterpret_cast<MessageHandler>(&CFixConnection::OnSessionInfo);
    gMessageTypeToHandler[FIX::MsgType_CurrencyList] = reinterpret_cast<MessageHandler>(&CFixConnection::OnCurrenciesInfo);
    gMessageTypeToHandler[FIX::MsgType_SecurityList] = reinterpret_cast<MessageHandler>(&CFixConnection::OnSymbolsInfo);
    gMessageTypeToHandler[FIX::MsgType_MarketDataRequestReject] = reinterpret_cast<MessageHandler>(&CFixConnection::OnSubscribeToQuotesReject);
    gMessageTypeToHandler[FIX::MsgType_MarketDataRequestAck] = reinterpret_cast<MessageHandler>(&CFixConnection::OnSubscribeToQuotesAck);
    gMessageTypeToHandler[FIX::MsgType_MarketDataSnapshotFullRefresh] = reinterpret_cast<MessageHandler>(&CFixConnection::OnTick);
    gMessageTypeToHandler[FIX::MsgType_ClosePositionRequestAck] = reinterpret_cast<MessageHandler>(&CFixConnection::OnClose);
    gMessageTypeToHandler[FIX::MsgType_ExecutionReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnExecution);
    gMessageTypeToHandler[FIX::MsgType_OrderCancelReject] = reinterpret_cast<MessageHandler>(&CFixConnection::OnCancel);
    gMessageTypeToHandler[FIX::MsgType_TradeServerInfoReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnTradeServerInfoReport);
    gMessageTypeToHandler[FIX::MsgType_AccountInfo] = reinterpret_cast<MessageHandler>(&CFixConnection::OnAccountInfo);
    gMessageTypeToHandler[FIX::MsgType_MarketDataHistory] = reinterpret_cast<MessageHandler>(&CFixConnection::OnMarketDataHistory);
    gMessageTypeToHandler[FIX::MsgType_MarketDataHistoryRequestReject] = reinterpret_cast<MessageHandler>(&CFixConnection::OnMarketDataHistoryReject);
    gMessageTypeToHandler[FIX::MsgType_TradeCaptureReportRequestAck] = reinterpret_cast<MessageHandler>(&CFixConnection::OnMarketTradeHistoryAck);
    gMessageTypeToHandler[FIX::MsgType_TradeCaptureReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnMarketTradeHistory);
    gMessageTypeToHandler[FIX::MsgType_FileChunk] = reinterpret_cast<MessageHandler>(&CFixConnection::OnFileChunk);
    gMessageTypeToHandler[FIX::MsgType_MarketDataHistoryMetadataReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnMarketDataHistoryMetadataReport);
    gMessageTypeToHandler[FIX::MsgType_BusinessMessageReject] = reinterpret_cast<MessageHandler>(&CFixConnection::OnBusinessReject);
    gMessageTypeToHandler[FIX::MsgType_TradeTransactionReportRequestAck] = reinterpret_cast<MessageHandler>(&CFixConnection::OnSuscribeToTradeTransactionReportsAck);
    gMessageTypeToHandler[FIX::MsgType_TradeTransactionReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnTradeTransactionReport);
    gMessageTypeToHandler[FIX::MsgType_PositionReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnPositionReport);
    gMessageTypeToHandler[FIX::MsgType_RequestForPositionsAck] = reinterpret_cast<MessageHandler>(&CFixConnection::OnEmpty);
    gMessageTypeToHandler[FIX::MsgType_Notification] = reinterpret_cast<MessageHandler>(&CFixConnection::OnNotification);
    gMessageTypeToHandler[FIX::MsgType_ComponentsInfoReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnComponentsInfoReport);
    gMessageTypeToHandler[FIX::MsgType_DailyAccountSnapshotRequestAck] = reinterpret_cast<MessageHandler>(&CFixConnection::OnDailyAccountSnapshotRequestAck);
    gMessageTypeToHandler[FIX::MsgType_DailyAccountSnapshotReport] = reinterpret_cast<MessageHandler>(&CFixConnection::OnDailyAccountSnapshotReport);
}

CFixConnection::CFixConnection(const string& name, const string& connectionString) :
    name_(name),
    m_sender(this),
#ifdef LOG_PERFORMANCE
    loggerIn_(service_),
    loggerOut_(service_),
#endif
    m_receiver(nullptr)
{
    CFxParams parameters(connectionString);
    const string fixVersion = parameters.GetString(cFixVersion);

    string senderCompId = parameters.GetString(cSenderCompId);
    if (senderCompId.empty())
    {
        senderCompId = FxGenerateGuid();
    }
    const string targetCompId = parameters.GetString(cTargetCompId);
    const string address = parameters.GetString(cAddress);
    const int32 port = parameters.GetInt32(cPort);
    const bool isSecureConnection = parameters.GetBoolean(cSecureConnection);
    const bool decodeLogFixMessages = parameters.GetBoolean(cDecodeLogFixMessages);
    const bool enableStats = parameters.GetBoolean(cEnableNetworkStatistics);

    const int32 mode = (enableStats ? FX_SOCKET_ENABLE_STATS : 0) | (isSecureConnection ? FX_SOCKET_MODE_SECURE : FX_SOCKET_MODE_SIMPLE);

    parameters.TryGetString(cProtocolVersion, m_protocolVersion);

#ifdef LOG_PERFORMANCE
    m_sender.setLogger(&loggerOut_);
    service_.start(0);
    loggerIn_.open(name + ".t2", name + " C++ In", ".\\Logs");
    loggerOut_.open(name + ".t1", name + " C++ Out", ".\\Logs");
#endif

    FIX::Dictionary sessionOptions;
    sessionOptions.setString(FIX::BEGINSTRING, fixVersion);
    sessionOptions.setString(FIX::SENDERCOMPID, senderCompId);
    sessionOptions.setString(FIX::TARGETCOMPID, targetCompId);
    sessionOptions.setString(FIX::CONNECTION_TYPE, "initiator");
    sessionOptions.setBool(FIX::USE_DATA_DICTIONARY, true);
#ifdef _MSC_VER
    sessionOptions.setLong(FIX::DATA_DICTIONARY, IDR_FIX44_XML);
#else
    sessionOptions.setString(FIX::DATA_DICTIONARY, "FIX44");
#endif
    sessionOptions.setString(FIX::START_TIME, "00:00:00");
    sessionOptions.setString(FIX::END_TIME, "00:00:00");
    sessionOptions.setLong(FIX::HEARTBTINT, 30);

    sessionOptions.setString(FIX::SOCKET_CONNECT_HOST, address);
    sessionOptions.setLong(FIX::SOCKET_CONNECT_PORT, port);
    sessionOptions.setBool(FIX::SOCKET_NODELAY, true);

    string proxyType;
    if (parameters.TryGetString(cProxyType, proxyType))
        sessionOptions.setString(FIX::PROXY_TYPE, proxyType);

    string proxyAddress;
    if (parameters.TryGetString(cProxyAddress, proxyAddress))
        sessionOptions.setString(FIX::PROXY_ADDRESS, proxyAddress);

    int proxyPort;
    if (parameters.TryGetInt32(cProxyPort, proxyPort))
        sessionOptions.setLong(FIX::PROXY_PORT, proxyPort);

    string proxyUserName;
    if (parameters.TryGetString(cProxyUserName, proxyUserName))
        sessionOptions.setString(FIX::PROXY_USERNAME, proxyUserName);

    string proxyPassword;
    if (parameters.TryGetString(cProxyPassword, proxyPassword))
        sessionOptions.setString(FIX::PROXY_PASSWORD, proxyPassword);

    FIX::SessionID sessionID(fixVersion, senderCompId, targetCompId);
    m_sessionID = sessionID;
    m_sender.SessionID(sessionID);
    m_settings.set(sessionID, sessionOptions);

    // create default settings
    m_settings.set(FIX::Dictionary());

    const wstring fixLogDirectory = StringToWString(parameters.GetString(cFixLogDirectory));

    if (fixLogDirectory.length())
    {
        const wstring fixEventsFileName = StringToWString(parameters.GetString(cFixEventsFileName));
        const wstring fixMessagesFileName = StringToWString(parameters.GetString(cFixMessagesFileName));
        const string excludeMessagesFromLogs = parameters.GetString(cExcludeMessagesFromLogs);

        m_logFactory = new FIX::FileLogFactory(fixLogDirectory, fixEventsFileName, fixMessagesFileName, excludeMessagesFromLogs, decodeLogFixMessages);
    }
    else
        m_logFactory = new NullLogFactory();

    m_username = parameters.GetString(cUsername);
    m_password = parameters.GetString(cPassword);
    m_deviceId = parameters.GetString(cDeviceId);
    m_appId = parameters.GetString(cAppId);
    m_sender.AppId(m_appId);
    m_appSessionId = parameters.GetString(cAppSessionId);

    m_initiator = new FIX::SocketInitiator(*this, m_messageStorefactory, m_settings, *m_logFactory, mode);
}

CFixConnection::~CFixConnection()
{
    delete m_initiator;
    delete m_logFactory;
}

bool CFixConnection::IsSubscribed(const std::string& symbol)
{
    return m_subscriptions.find(symbol) != m_subscriptions.end();
}

bool CFixConnection::IsSnapshotSend(const std::string& symbol)
{
    return IsSubscribed(symbol) && m_subscriptions[symbol];
}

void CFixConnection::Subscribe(const std::string& symbol)
{
    m_subscriptions[symbol] = false;
}

void CFixConnection::Snapshot(const std::string& symbol)
{
    if (IsSubscribed(symbol))
        m_subscriptions[symbol] = true;
}

void CFixConnection::Unsubscribe(const std::string& symbol)
{
    m_subscriptions.erase(symbol);
}

void CFixConnection::VReceiver(IReceiver* pReceiver)
{
    m_receiver = pReceiver;
}

ISender* CFixConnection::VSender()
{
    return &m_sender;
}

void CFixConnection::VShutdown()
{
    m_initiator->stop();
}

void CFixConnection::VStop()
{
}

void CFixConnection::VStart()
{
    m_initiator->start();
}

void CFixConnection::VGetActivity(uint64* pLogicalBytesSent, uint64* pPhysicalBytesSent, uint64* pLogicalBytesReceived, uint64* pPhysicalBytesReceived)
{
    m_initiator->GetNetworkActivity(pLogicalBytesSent, pPhysicalBytesSent, pLogicalBytesReceived, pPhysicalBytesReceived);
}

void CFixConnection::toAdmin(FIX::Message& message, const FIX::SessionID& /*sessionID*/)
{
    FIX44::Header& header = static_cast<FIX44::Header&>(message.getHeader());
    const string type = header.GetMsgType();
    if (FIX::MsgType_Logon == type)
    {
        FIX44::Logon& logon = static_cast<FIX44::Logon&>(message);
        logon.SetUsername(m_username);
        logon.SetPassword(m_password);
        if (!m_deviceId.empty())
            logon.SetDeviceID(m_deviceId);
        if (!m_appSessionId.empty())
            logon.SetAppSessionID(m_appSessionId);
        logon.SetResetSeqNumFlag(true);
        if (!m_protocolVersion.empty() && ("ext.0.0" != m_protocolVersion))
        {
            logon.SetProtocolSpec(m_protocolVersion);
        }
    }
}

void CFixConnection::fromApp(const Message& message, const FIX::SessionID& /*sessionID*/)throw (FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType)
{
    const FIX44::Header& header = static_cast<const FIX44::Header&>(message.getHeader());
    const string beginString = header.GetBeginString();
    if (FIX::BeginString_FIX44 != beginString)
    {
        throw FIX::UnsupportedVersion(beginString);
    }
    const string messageType = header.GetMsgType();

    auto it = gMessageTypeToHandler.find(messageType);
    if (gMessageTypeToHandler.end() == it)
    {
        throw FIX::UnsupportedMessageType(messageType);
    }
    MessageHandler handler = it->second;
    (this->*handler)(message);
}

void CFixConnection::onLogon(const Message& message, const SessionID& /*sessionID*/)
{
    const FIX44::Logon& logon = static_cast<const FIX44::Logon&>(message);

    string protocolVersion = "ext.0.0";
    logon.TryGetProtocolSpec(protocolVersion);

    bool twofactor = false;
    logon.TryGetTwoFactorAuthFlag(twofactor);

    m_version = CFixVersion(protocolVersion);
    m_sender.SendVersion(m_version);

    CFxEventInfo eventInfo;
    m_receiver->VLogon(eventInfo, protocolVersion, twofactor);
}

void CFixConnection::onLogout(const Message& message, const SessionID& /*sessionID*/)
{
    CFxEventInfo eventInfo;
    string description;
    const FIX44::Logout& logout = static_cast<const FIX44::Logout&>(message);
    logout.TryGetText(description);

    FxLogoutReason reason = FxLogoutReason_None;
    int fixReason = 0;
    if (logout.TryGetLogoutReason(fixReason))
    {
        if (FIX::LogoutReason_TIMEOUT == fixReason)
        {
            reason = FxLogoutReason_Timeout;
        }
        else if (FIX::LogoutReason_NETWORK_ERROR == fixReason)
        {
            reason = FxLogoutReason_NetworkError;
        }
        else if (FIX::LogoutReason_BLOCKED_LOGIN == fixReason)
        {
            reason = FxLogoutReason_BlockedAccount;
        }
        else if (FIX::LogoutReason_CLIENT_LOGOUT == fixReason)
        {
            reason = FxLogoutReason_ClientInitiated;
        }
        else if (FIX::LogoutReason_INVALID_CREDS == fixReason)
        {
            reason = FxLogoutReason_InvalidCredentials;
        }
        else if (FIX::LogoutReason_SLOW_CONNECTION == fixReason)
        {
            reason = FxLogoutReason_SlowConnection;
        }
        else if (FIX::LogoutReason_INVALID_SESSION_ID == fixReason)
        {
            reason = FxLogoutReason_InvalidSession;
        }
        else if (FIX::LogoutReason_INTERNAL_SERVER_ERROR == fixReason)
        {
            reason = FxLogoutReason_ServerError;
        }
        else if (FIX::LogoutReason_TIMEOUT_LOGIN == fixReason)
        {
            reason = FxLogoutReason_LoginTimeout;
        }
        else if (FIX::LogoutReason_DELETED_LOGIN == fixReason)
        {
            reason = FxLogoutReason_LoginDeleted;
        }
        else if (FIX::LogoutReason_SERVER_LOGOUT == fixReason)
        {
            reason = FxLogoutReason_ServerLogout;
        }
        else
        {
            reason = FxLogoutReason_Unknown;
        }
    }
    m_receiver->VLogout(eventInfo, reason, description);
}

void CFixConnection::OnTwoFactorAuth(const FIX44::TwoFactorLogon& message)
{
    FxTwoFactorReason reason = FxTwoFactorReason_Unknown;
    FIX::UtcTimeStamp expire;
    std::string text;

    char temp;
    if (message.TryGetTwoFactorReason(temp))
        reason = static_cast<FxTwoFactorReason>(temp - '0');
    message.TryGetExpireTime(expire);
    message.TryGetText(text);

    CFxEventInfo eventInfo;
    m_receiver->VTwoFactorAuth(eventInfo, reason, text, expire.toFileTime());
}

void CFixConnection::OnSessionInfo(const FIX44::TradingSessionStatus& message)
{
    CFxEventInfo eventInfo;

    CFxSessionInfo sessionInfo;
    sessionInfo.TradingSessionId = message.GetTradingSessionID();
    message.TryGetTradSesReqID(eventInfo.ID);
    sessionInfo.StartTime = message.GetTradSesStartTime().toFileTime();
    sessionInfo.OpenTime = message.GetTradSesOpenTime().toFileTime();
    sessionInfo.CloseTime = message.GetTradSesCloseTime().toFileTime();
    sessionInfo.EndTime = message.GetTradSesEndTime().toFileTime();
    sessionInfo.Status = (::SessionStatus)message.GetTradSesStatus();
    message.TryGetPlatformTimezoneOffset(sessionInfo.ServerTimeZoneOffset);
    message.TryGetPlatformCompany(sessionInfo.PlatformCompany);
    message.TryGetPlatformName(sessionInfo.PlatformName);

    int32 count;
    if (message.TryGetNoStatusGroups(count))
    {
        sessionInfo.StatusGroups.reserve(count);

        for (int32 index = 1; index <= count; ++index)
        {
            FIX44::TradingSessionStatus::NoStatusGroups group;
            message.getGroup(index, group);

            CFxStatusGroupInfo statusGroupInfo;
            statusGroupInfo.StatusGroupId = group.GetStatusGroupID();
            statusGroupInfo.Status = (::SessionStatus) group.GetTradSesStatus();
            statusGroupInfo.StartTime = group.GetTradSesStartTime().toFileTime();
            statusGroupInfo.EndTime = group.GetTradSesEndTime().toFileTime();

            if (group.isSet(FIX::TradSesOpenTime()))
                statusGroupInfo.OpenTime = group.GetTradSesOpenTime().toFileTime();
            if (group.isSet(FIX::TradSesCloseTime()))
                statusGroupInfo.CloseTime = group.GetTradSesCloseTime().toFileTime();

            sessionInfo.StatusGroups.push_back(statusGroupInfo);
        }
    }

    m_receiver->VSessionInfo(eventInfo, sessionInfo);
}

void CFixConnection::OnEmpty(const FIX::Message& message)
{
    UNREFERENCED_PARAMETER(message);
}

void CFixConnection::OnCurrenciesInfo(const FIX44::CurrencyList& message)
{
    CFxEventInfo evenInfo;
    vector<CFxCurrencyInfo> currencies;
    evenInfo.ID = message.GetCurrencyReqID();

    int32 count = 0;
    message.TryGetNoRelatedSym(count);

    for (int32 index = 1; index <= count; ++index)
    {
        FIX44::CurrencyList::NoRelatedSym group;
        message.getGroup(index, group);

        const string name = group.GetCurrency();

        int sortOder = 0;
        group.TryGetCurrencySortOrder(sortOder);

        int precision = 0;
        group.TryGetCurrencyPrecision(precision);

        wstring description;

        string encodedDescription;
        if (group.TryGetEncodedText(encodedDescription))
            Utf8ToStd(description, encodedDescription);

        CFxCurrencyInfo info(name, description, sortOder, precision);

        currencies.push_back(info);
    }

    m_receiver->VGetCurrencies(evenInfo, currencies);
}

void CFixConnection::OnSymbolsInfo(const FIX44::SecurityList& message)
{
    CFxEventInfo evenInfo;
    vector<CFxSymbolInfo> symbols;
    evenInfo.ID = message.GetSecurityReqID();

    int32 count = 0;
    message.TryGetNoRelatedSym(count);

    for (int32 index = 1; index <= count; ++index)
    {
        FIX44::SecurityList::NoRelatedSym group;
        message.getGroup(index, group);
        const string name = group.GetSymbol();

        string currency;
        group.TryGetCurrency(currency);

        string settlementCurrency;
        group.TryGetSettlCurrency(settlementCurrency);

        CFxSymbolInfo info(name, currency, settlementCurrency);

        string encodedDescription;
        if (group.TryGetEncodedText(encodedDescription))
            Utf8ToStd(info.Description, encodedDescription);

        if (!group.TryGetCurrencySortOrder(info.CurrencySortOrder))
            info.CurrencySortOrder = 0;

        if (!group.TryGetSettlCurrencySortOrder(info.SettlementCurrencySortOrder))
            info.SettlementCurrencySortOrder = 0;

        if (!group.TryGetCurrencyPrecision(info.CurrencyPrecision))
            info.CurrencyPrecision = 5;

        if (!group.TryGetSettlCurrencyPrecision(info.SettlementCurrencyPrecision))
            info.SettlementCurrencyPrecision = 5;

        info.ContractMultiplier = group.GetContractMultiplier();
        group.TryGetPxPrecision(info.Precision);
        group.TryGetRoundLot(info.RoundLot);
        group.TryGetMinTradeVol(info.MinTradeVolume);
        group.TryGetMaxTradeVolume(info.MaxTradeVolume);
        group.TryGetTradeVolStep(info.TradeVolumeStep);
        group.TryGetMarginHedge(info.MarginHedge);
        group.TryGetMarginFactor(info.MarginFactor);
        double marginFactorFractional;
        if (group.TryGetMarginFactorFractional(marginFactorFractional))
            info.MarginFactorFractional = marginFactorFractional;

        group.TryGetColorRef(info.Color);
        group.TryGetLimitsCommission(info.LimitsCommission);
        group.TryGetCommission(info.Commission);

        char marginCalcMode;
        if (group.TryGetMarginCalcMode(marginCalcMode))
        {
            if (marginCalcMode == 'f')
                info.MarginCalcMode = FxMarginCalcMode_Forex;
            else if (marginCalcMode == 'c')
                info.MarginCalcMode = FxMarginCalcMode_Cfd;
            else if (marginCalcMode == 't')
                info.MarginCalcMode = FxMarginCalcMode_Futures;
            else if (marginCalcMode == 'i')
                info.MarginCalcMode = FxMarginCalcMode_CfdIndex;
            else if (marginCalcMode == 'l')
                info.MarginCalcMode = FxMarginCalcMode_CfdLeverage;
        }

        char profitCalcMode;
        if (group.TryGetProfitCalcMode(profitCalcMode))
        {
            if (profitCalcMode == 'f')
                info.ProfitCalcMode = FxProfitCalcMode_Forex;
            else if (profitCalcMode == 'c')
                info.ProfitCalcMode = FxProfitCalcMode_Cfd;
            else if (profitCalcMode == 't')
                info.ProfitCalcMode = FxProfitCalcMode_Futures;
            else if (profitCalcMode == 'i')
                info.ProfitCalcMode = FxProfitCalcMode_CfdIndex;
            else if (profitCalcMode == 'l')
                info.ProfitCalcMode = FxProfitCalcMode_CfdLeverage;
        }

        auto commType = '\0';
        if (group.TryGetCommType(commType))
            info.CommissionType = (FxCommissionType)(commType - FIX::CommType_PER_UNIT);

        auto commChargeType = 0;
        if (group.TryGetCommChargeType(commChargeType))
            info.CommissionChargeType = (FxCommissionChargeType)commChargeType;

        auto commChargeMethod = 0;
        if (group.TryGetCommChargeMethod(commChargeType))
            info.CommissionChargeMethod = (FxCommissionChargeMethod)commChargeMethod;

        double minCommission = 0.0;
        if (group.TryGetMinCommission(minCommission))
            info.MinCommission = minCommission;

        string minCommissionCurrency;
        if (group.TryGetMinCommissionCurrency(minCommissionCurrency))
            Utf8ToStd(info.MinCommissionCurrency, minCommissionCurrency);

        auto swapType = 0;
        if (group.TryGetSwapType(swapType))
            info.SwapType = (::SwapType)swapType;

        int tripleSwapDay = 0;
        if (group.TryGetTripleSwapDay(tripleSwapDay))
            info.TripleSwapDay = tripleSwapDay;

        double swapSizeShort;
        if (group.TryGetSwapSizeShort(swapSizeShort))
            info.SwapSizeShort = swapSizeShort;

        double swapSizeLong;
        if (group.TryGetSwapSizeLong(swapSizeLong))
            info.SwapSizeLong = swapSizeLong;

        double defaultSlippage;
        if (group.TryGetDefaultSlippage(defaultSlippage))
            info.DefaultSlippage = defaultSlippage;

        group.TryGetTradeEnabled(info.IsTradeEnabled);

        group.TryGetGroupSortOrder(info.GroupSortOrder);
        group.TryGetSortOrder(info.SortOrder);

        group.TryGetStatusGroupID(info.StatusGroupId);

        group.TryGetSecurityID(info.SecurityName);

        string encodedSecurityDescription;
        if (group.TryGetEncodedSecurityDesc(encodedSecurityDescription))
            Utf8ToStd(info.SecurityDescription, encodedSecurityDescription);

        double stopOrderMarginReduction;
        if (group.TryGetStopOrderMarginReduction(stopOrderMarginReduction))
            info.StopOrderMarginReduction = stopOrderMarginReduction;

        double hiddenLimitOrderMarginReduction;
        if (group.TryGetHiddenLimitOrderMarginReduction(hiddenLimitOrderMarginReduction))
            info.HiddenLimitOrderMarginReduction = hiddenLimitOrderMarginReduction;

        symbols.push_back(info);
    }

    if (m_version >= cExt19)
    {
        for (int32 index = 0; index < count; ++index)
        {
            CFxSymbolInfo& info = symbols[index];
            info.MinTradeVolume *= info.RoundLot;
            info.TradeVolumeStep *= info.RoundLot;
            info.MaxTradeVolume *= info.RoundLot;
        }
    }

    m_receiver->VGetSupportedSymbols(evenInfo, symbols);
}

void CFixConnection::OnSubscribeToQuotesReject(const FIX44::MarketDataRequestReject& message)
{
    CFxEventInfo eventInfo;
    eventInfo.Status = FX_CODE_ERROR_REJECT;
    eventInfo.ID = message.GetMDReqID();
    char reason = 0;
    if (message.TryGetMDReqRejReason(reason))
    {
        if (FIX::MDReqRejReason_UNKNOWN_SYMBOL == reason)
        {
            eventInfo.Message = "Unknown symbol";
        }
        else if (FIX::MDReqRejReason_UNSUPPMKTDEPTH == reason)
        {
            eventInfo.Message = "Unsupported market depth";
        }
    }
    m_receiver->VSubscribeToQuotes(eventInfo, FX_CODE_ERROR_REJECT);
}

void CFixConnection::OnSubscribeToQuotesAck(const FIX44::MarketDataRequestAck& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = message.GetMDReqID();
    m_receiver->VSubscribeToQuotes(eventInfo, S_OK);
}

void CFixConnection::OnTick(const FIX44::MarketDataSnapshotFullRefresh& message)
{
#ifdef LOG_PERFORMANCE
    uint64_t timestamp = loggerIn_.getTimestamp();
    string id = message.GetTickId();
    loggerIn_.logTimestamp(id.c_str(), timestamp, "Tick");
#endif
    CFxEventInfo eventInfo;
    const string symbol = message.GetSymbol();

    FIX::UtcTimeStamp creatingTickTime;
    message.TryGetOrigTime(creatingTickTime);
    eventInfo.SendingTime = message.getHeader().GetSendingTime().toFileTime();


    CFxQuote quote(symbol, creatingTickTime.toFileTime());
    message.TryGetTickId(quote.Id);
    message.TryGetIndicativeTick(quote.IndicativeTick);

    const int32 count = message.GetNoMDEntries();
    for (int32 index = 1; index <= count; ++index)
    {
        FIX44::MarketDataSnapshotFullRefresh::NoMDEntries entries;
        message.getGroup(index, entries);

        const double price = entries.GetMDEntryPx();
        const double volume = entries.GetMDEntrySize();

        const char type = entries.GetMDEntryType();
        if (FIX::MDEntryType_BID == type)
        {
            quote.AddBid(price, volume);
        }
        else if (FIX::MDEntryType_OFFER == type)
        {
            quote.AddAsk(price, volume);
        }
    }
    quote.Sort();

    // Rise subscribed event
    if (IsSubscribed(symbol))
    {
        if (!IsSnapshotSend(symbol))
        {
            m_receiver->VSubscribed(eventInfo, quote);
            Snapshot(symbol);
        }
        else
            m_receiver->VTick(eventInfo, quote);
    }
}

void CFixConnection::OnClose(const FIX44::ClosePositionRequestAck& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = message.GetClosePosReqID();
    CFxClosePositionsResponse response;
    const FIX::ClosePosReqResult status = message.GetClosePosReqResult();
    if (FIX::ClosePosReqResult_SUCCESSFUL == status)
    {
        response.Status = S_OK;
    }
    else
    {
        response.Status = E_FAIL;
    }

    int32 count = 0;
    message.TryGetNoPositions(count);
    for (int32 index = 1; index <= count; ++index)
    {
        FIX44::ClosePositionRequestAck::NoPositions group;
        message.getGroup(index, group);
        const string orderId = group.GetOrderID();
        response.Orders.push_back(orderId);
    }
    message.TryGetText(response.Description);
    message.TryGetText(eventInfo.Message);
    m_receiver->VClosePositions(eventInfo, response);
}

void CFixConnection::OnExecution(const CFixExecutionReport& message)
{
#ifdef LOG_PERFORMANCE
    if (message.GetFxExecutionType() == FxExecutionType_Trade)
    {
        uint64_t timestamp = loggerIn_.getTimestamp();
        string id = message.GetFxClientOrderId();
        loggerIn_.logTimestamp(id.c_str(), timestamp, "ExecReport");
    }
#endif

    CFxEventInfo eventInfo;
    CFxExecutionReport report;

    report.ExecutedVolume = message.GetFxExecutedVolume();
    report.InitialVolume = message.GetFxInitialVolume();
    report.LeavesVolume = message.GetFxLeavesVolume();
    report.MaxVisibleVolume = message.GetFxMaxVisibleVolume();
    report.TradeAmount = message.GetFxTradeAmount();
    report.AveragePrice = message.GetFxAveragePrice();
    report.Price = message.GetFxPrice();
    report.TradePrice = message.GetFxTradePrice();
    report.StopPrice = message.GetFxStopPrice();
    report.TakeProfit = message.GetFxTakeProfit();
    report.StopLoss = message.GetFxStopLoss();
    report.Balance = message.GetFxBalance();

    report.Expiration = message.GetFxExpiration();

    report.OrderId = message.GetFxOrderId();
    report.ClientOrderId = message.GetFxClientOrderId();
    report.TradeRequestId = message.GetFxTradeRequestId();
    report.Symbol = message.GetFxSymbol();
    report.Text = message.GetFxText();
    report.ClosePositionRequestId = message.GetFxClosePositionRequestId();

    string encodedComment = message.GetComment();
    Utf8ToStd(report.Comment, encodedComment);
    string encodedTag = message.GetTag();
    Utf8ToStd(report.Tag, encodedTag);
    report.Magic = message.GetMagic();

    report.ExecutionType = message.GetFxExecutionType();
    report.OrderStatus = message.GetFxOrderStatus();
    report.InitialOrderType = message.GetFxInitialOrderType();
    report.OrderType = message.GetFxOrderType();
    report.ReportsNumber = message.GetFxReportsNumber();
    report.RejectReason = message.GetFxRejectReason();
    report.OrderSide = message.GetFxSide();
    report.Created = message.GetCreatedDateTime();
    report.Modified = message.GetModifiedDateTime();
    report.Commission = message.GetFxCommission();
    report.AgentCommission = message.GetFxAgentCommission();
    report.IsReducedOpenCommission = message.GetReducedOpenCommissionFlag();
    report.IsReducedCloseCommission = message.GetReducedCloseCommissionFlag();
    report.ImmediateOrCancel = message.GetImmediateOrCancelFlag();
    report.MarketWithSlippage = message.GetMarketWithSlippageFlag();
    report.Swap = message.GetFxSwap();
    report.ReqOpenPrice = message.GetFxReqOpenPrice();
    report.ReqOpenVolume = message.GetFxReqOpenVolume();

    message.GetAssets(report.Assets);

    if (!message.TryGetMassStatusRequestId(eventInfo.ID))
    {
        string requestId = report.ClosePositionRequestId;
        if (requestId.empty())
            requestId = report.TradeRequestId;
        if (requestId.empty())
            requestId = report.ClientOrderId;
        eventInfo.ID = requestId;
    }
    m_receiver->VExecution(eventInfo, report);
}

void CFixConnection::OnCancel(const FIX44::OrderCancelReject& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = message.GetClOrdID();
    message.TryGetText(eventInfo.Message);
    CFxExecutionReport executionReport;

    message.TryGetOrderID(executionReport.OrderId);
    message.TryGetClOrdID(executionReport.TradeRequestId);
    message.TryGetText(executionReport.Text);
    executionReport.OrderStatus = FxOrderStatus_Rejected;
    m_receiver->VExecution(eventInfo, executionReport);
}

void CFixConnection::OnTradeServerInfoReport(const FIX44::TradeServerInfoReport& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = message.GetTrdSrvReqID();

    CFxTradeServerInfo tradeServerInfo;
    message.TryGetCompanyName(tradeServerInfo.CompanyName);
    message.TryGetCompanyFullName(tradeServerInfo.CompanyFullName);
    string encodedCompanyDescription;
    if (message.TryGetEncodedCompanyDescription(encodedCompanyDescription))
        Utf8ToStd(tradeServerInfo.CompanyDescription, encodedCompanyDescription);
    message.TryGetCompanyAddress(tradeServerInfo.CompanyAddress);
    message.TryGetCompanyEmail(tradeServerInfo.CompanyEmail);
    message.TryGetCompanyPhone(tradeServerInfo.CompanyPhone);
    message.TryGetCompanyWebSite(tradeServerInfo.CompanyWebSite);
    message.TryGetServerName(tradeServerInfo.ServerName);
    message.TryGetServerFullName(tradeServerInfo.ServerFullName);
    message.TryGetServerFullName(tradeServerInfo.ServerFullName);
    string encodedServerDescription;
    if (message.TryGetEncodedServerDescription(encodedServerDescription))
        Utf8ToStd(tradeServerInfo.ServerDescription, encodedServerDescription);
    message.TryGetServerAddress(tradeServerInfo.ServerAddress);
    int port;
    if (message.TryGetServerFixFeedSslPort(port))
        tradeServerInfo.ServerFixFeedSslPort = port;
    if (message.TryGetServerFixTradeSslPort(port))
        tradeServerInfo.ServerFixTradeSslPort = port;
    if (message.TryGetServerWebSocketFeedPort(port))
        tradeServerInfo.ServerWebSocketFeedPort = port;
    if (message.TryGetServerWebSocketTradePort(port))
        tradeServerInfo.ServerWebSocketTradePort = port;
    if (message.TryGetServerRestPort(port))
        tradeServerInfo.ServerRestPort = port;

    m_receiver->VTradeServerInfoReport(eventInfo, tradeServerInfo);
}

void CFixConnection::OnAccountInfo(const FIX44::AccountInfo& message)
{
    CFxEventInfo eventInfo;
    message.TryGetAcInfReqID(eventInfo.ID);

    CFxAccountInfo accountInfo;
    const FIX::AccountingType accountingType = message.GetAccountingType();
    if (FIX::AccountingType_GROSS == accountingType)
    {
        accountInfo.Type = FxAccountType_Gross;
    }
    else if (FIX::AccountingType_NET == accountingType)
    {
        accountInfo.Type = FxAccountType_Net;
    }
    else if (FIX::AccountingType_CASH == accountingType)
    {
        accountInfo.Type = FxAccountType_Cash;
    }
    else
    {
        return;
    }
    accountInfo.Leverage = message.GetLeverage();
    accountInfo.Balance = message.GetBalance();
    accountInfo.Margin = message.GetMargin();
    accountInfo.Equity = message.GetEquity();

    accountInfo.Currency = message.GetCurrency();
    accountInfo.AccountId = message.GetAccount();
    message.TryGetAccMarginCallLevel(accountInfo.MarginCallLevel);
    message.TryGetAccStopOutLevel(accountInfo.StopOutLevel);
    message.TryGetAccountName(accountInfo.Name);
    message.TryGetAccountValidFlag(accountInfo.IsValid);
    message.TryGetInvestorLoginFlag(accountInfo.IsReadOnly);
    message.TryGetAccountBlockedFlag(accountInfo.IsBlocked);
    message.TryGetRegistEmail(accountInfo.Email);

    string encodedComment;
    if (message.TryGetEncodedComment(encodedComment))
        Utf8ToStd(accountInfo.Comment, encodedComment);

    FIX::UtcTimeStamp registred(time_t(0));
    if (message.TryGetRegistDate(registred))
        accountInfo.RegistredDate = registred.toFileTime();
    FIX::UtcTimeStamp modified(time_t(0));
    if (message.TryGetModifyTime(modified))
        accountInfo.ModifiedTime = modified.toFileTime();

    auto count = 0;
    if (message.TryGetNoAssets(count))
    {
        accountInfo.Assets.reserve(static_cast<size_t>(count));
        for (int index = 1; index <= count; ++index)
        {
            FIX44::AccountInfo::NoAssets group;
            message.getGroup(index, group);

            CAssetInfo asset;
            asset.Currency = group.GetAssetCurrency();
            asset.Balance = group.GetAssetBalance();
            group.TryGetAssetLockedAmt(asset.LockedAmount);
            group.TryGetAssetTradeAmt(asset.TradeAmount);

            if (asset.Balance == 0.0)
                continue;

            accountInfo.Assets.push_back(asset);
        }
    }

    message.TryGetSessionsPerAccount(accountInfo.SessionsPerAccount);
    message.TryGetRequestsPerSecond(accountInfo.RequestsPerSecond);
    count = 0;
    if (message.TryGetThrottlingMethodsInfo(count))
    {
        accountInfo.ThrottlingMethods.reserve(static_cast<size_t>(count));
        for (int index = 1; index <= count; ++index)
        {
            FIX44::AccountInfo::ThrottlingMethodsInfo group;
            message.getGroup(index, group);

            CThrottlingMethodInfo method;
            FIX::ThrottlingMethod throttlingMethod = group.GetThrottlingMethod();
            switch (throttlingMethod)
            {
            case FIX::ThrottlingMethod_Account:
                method.Method = FxThrottlingMethod_Account;
                break;

            case FIX::ThrottlingMethod_Assets:
                method.Method = FxThrottlingMethod_Assets;
                break;

            case FIX::ThrottlingMethod_Currencies:
                method.Method = FxThrottlingMethod_Currencies;
                break;

            case FIX::ThrottlingMethod_DailyAccountSnapshots:
                method.Method = FxThrottlingMethod_DailyAccountSnapshots;
                break;

            case FIX::ThrottlingMethod_FeedSubscribe:
                method.Method = FxThrottlingMethod_FeedSubscribe;
                break;

            case FIX::ThrottlingMethod_Level2:
                method.Method = FxThrottlingMethod_Level2;
                break;

            case FIX::ThrottlingMethod_Login:
                method.Method = FxThrottlingMethod_Login;
                break;

            case FIX::ThrottlingMethod_Positions:
                method.Method = FxThrottlingMethod_Positions;
                break;

            case FIX::ThrottlingMethod_QuoteHistory:
                method.Method = FxThrottlingMethod_QuoteHistory;
                break;

            case FIX::ThrottlingMethod_QuoteHistoryCache:
                method.Method = FxThrottlingMethod_QuoteHistoryCache;
                break;

            case FIX::ThrottlingMethod_SessionInfo:
                method.Method = FxThrottlingMethod_SessionInfo;
                break;

            case FIX::ThrottlingMethod_Symbols:
                method.Method = FxThrottlingMethod_Symbols;
                break;

            case FIX::ThrottlingMethod_Tickers:
                method.Method = FxThrottlingMethod_Tickers;
                break;

            case FIX::ThrottlingMethod_Ticks:
                method.Method = FxThrottlingMethod_Ticks;
                break;

            case FIX::ThrottlingMethod_TradeCreate:
                method.Method = FxThrottlingMethod_TradeCreate;
                break;

            case FIX::ThrottlingMethod_TradeDelete:
                method.Method = FxThrottlingMethod_TradeDelete;
                break;

            case FIX::ThrottlingMethod_TradeHistory:
                method.Method = FxThrottlingMethod_TradeHistory;
                break;

            case FIX::ThrottlingMethod_TradeModify:
                method.Method = FxThrottlingMethod_TradeModify;
                break;

            case FIX::ThrottlingMethod_TradeServerInfo:
                method.Method = FxThrottlingMethod_TradeServerInfo;
                break;

            case FIX::ThrottlingMethod_TradeSessionInfo:
                method.Method = FxThrottlingMethod_TradeSessionInfo;
                break;

            case FIX::ThrottlingMethod_Trades:
                method.Method = FxThrottlingMethod_Trades;
                break;

            case FIX::ThrottlingMethod_TwoFactor:
                method.Method = FxThrottlingMethod_TwoFactor;
                break;

            case FIX::ThrottlingMethod_UnknownMethod:
                method.Method = FxThrottlingMethod_UnknownMethod;
            }

            group.TryGetRequestsPerSecond(method.RequestsPerSecond);

            accountInfo.ThrottlingMethods.push_back(method);
        }
    }
    m_receiver->VAccountInfo(eventInfo, accountInfo);
}

void CFixConnection::OnMarketDataHistory(const FIX44::MarketDataHistory& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = message.GetMarketHistReqID();

    CFxDataHistoryResponse response;

    response.FromAll = message.GetAllHstFrom().toFileTime();
    response.ToAll = message.GetAllHstTo().toFileTime();
    response.From = message.GetHstFrom().toFileTime();
    response.To = message.GetHstTo().toFileTime();



    message.TryGetLastTickId(response.LastTickId);

    int32 count = 0;
    message.TryGetNoBars(count);
    for (int32 index = 1; index <= count; ++index)
    {
        FIX44::MarketDataHistory::NoBars group;
        message.getGroup(index, group);

        CFxBar bar;
        bar.Open = group.GetBarOpen();
        bar.Close = group.GetBarClose();
        bar.High = group.GetBarHi();
        bar.Low = group.GetBarLow();
        if (!group.TryGetBarVolumeEx(bar.Volume))
        {
            bar.Volume = group.GetBarVolume();
        }
        bar.From = group.GetBarTime().toFileTime();
        response.Bars.push_back(bar);
    }

    count = 0;
    message.TryGetNoFiles(count);
    for (int32 index = 1; index <= count; ++index)
    {
        FIX44::MarketDataHistory::NoFiles group;
        message.getGroup(index, group);
        const string file =  group.GetAttachedFileId();
        response.Files.push_back(file);
    }

    m_receiver->VDataHistoryResponse(eventInfo, response);
}

void CFixConnection::OnMarketDataHistoryReject(const FIX44::MarketDataHistoryRequestReject& message)
{
    CFxEventInfo eventInfo;
    eventInfo.Status = FX_CODE_ERROR_REJECT;
    eventInfo.ID = message.GetMarketHistReqID();
    message.TryGetText(eventInfo.Description);
    const FIX::MHstRejReason reason = message.GetMHstRejReason();
    if (FIX::MHstRejReason_UNSUP_PERIOD == reason)
    {
        eventInfo.Message = "Unsupported bar period";
    }
    else if (FIX::MHstRejReason_UNSUP_SYM == reason)
    {
        eventInfo.Message = "Unsupported symbol";
    }
    else if (FIX::MHstRejReason_UNSUP_RP_TYPE == reason)
    {
        eventInfo.Message = "Unsupported report type";
    }
    else
    {
        eventInfo.Message = "Unknown error";
    }
    CFxDataHistoryResponse response;
    m_receiver->VDataHistoryResponse(eventInfo, response);
}

void CFixConnection::OnMarketTradeHistoryAck(const FIX44::TradeCaptureReportRequestAck& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = message.GetTradeRequestID();

    CFxTradeHistoryResponse response;
    response.TradeReportsNumber = message.GetTotNumTradeReports();
    int32 value = message.GetTradeRequestResult();
    response.Result = (FIX::TradeRequestResult_SUCCESSFUL == value);
    response.EndOfStream = message.GetEndOfStrm();
    m_receiver->VTradeHistoryResponse(eventInfo, response);
}

void CFixConnection::OnMarketTradeHistory(const FIX44::TradeCaptureReport& message)
{
    CFxEventInfo eventInfo;
    message.TryGetTradeRequestID(eventInfo.ID);

    CFxTradeHistoryReport report;


    message.TryGetSymbol(report.Symbol);
    report.TradeType = message.GetTrdType();
    char executionType = 0;
    if (message.TryGetExecType(executionType))
    {
        if (FIX::ExecType_NEW == executionType)
        {
            report.ExecutionType = FxExecutionType_New;
        }
        else if (FIX::ExecType_TRADE == executionType)
        {
            report.ExecutionType = FxExecutionType_Trade;
        }
        else if (FIX::ExecType_CALCULATED == executionType)
        {
            report.ExecutionType = FxExecutionType_Calculated;
        }
        else if (FIX::ExecType_CANCELED == executionType)
        {
            report.ExecutionType = FxExecutionType_Canceled;
        }
        else
        {
            return;
        }
    }
    report.TransactTime = message.GetTransactTime().toFileTime();
    report.Amount = message.GetLastQty();
    report.OpenPrice = message.GetLastPx();

    m_receiver->VTradeHistoryReport(eventInfo, report);
}

void CFixConnection::OnFileChunk(const FIX44::FileChunk& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = "ES" + message.GetFileId();

    CFxFileChunk fileChunk;
    fileChunk.FileId = message.GetFileId();
    message.TryGetFileName(fileChunk.FileName);
    fileChunk.FileSize = message.GetFileSize();
    fileChunk.ChunkId = message.GetChunkId();
    fileChunk.TotalChunks = message.GetChunksNo();
    string data = message.GetRawData();
    fileChunk.Data.insert(fileChunk.Data.end(), data.begin(), data.end());
    m_receiver->VFileChunk(eventInfo, fileChunk);
}

void CFixConnection::OnFileChunkReject(const FIX44::FileChunkReqReject& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = "ES" + message.GetFileId();
    eventInfo.Status = E_FAIL;

    CFxFileChunk fileChunk;
    m_receiver->VFileChunk(eventInfo, fileChunk);
}

void CFixConnection::OnMarketDataHistoryMetadataReport(const FIX44::MarketDataHistoryMetadataReport& message)
{
    CFxEventInfo eventInfo;
    eventInfo.ID = message.GetMDHstMetaReqID();
    const FIX::MDHstMetaReqResult status = message.GetMDHstMetaReqResult();
    if (FIX::MDHstMetaReqResult_SUCCESS != status)
    {
        eventInfo.Status = E_FAIL;
    }
    if (FIX::MDHstMetaReqResult_INVALID_SYMBOL == status)
    {
        eventInfo.Message = "Unsupported symbol";
        string symbol;
        if (message.TryGetSymbol(symbol))
        {
            eventInfo.Message += " = " + symbol;
        }
    }
    else if (FIX::MDHstMetaReqResult_INVALID_PERIODICITY == status)
    {
        eventInfo.Message = "Unsupported periodicity";
    }
    else if (FIX::MDHstMetaReqResult_SUCCESS != status)
    {
        eventInfo.Message = "Unknown error";
    }
    string st;
    message.TryGetFileId(st);

    m_receiver->VMetaInfoFile(eventInfo, st);
}

void CFixConnection::OnBusinessReject(const FIX44::BusinessMessageReject& message)
{
    CFxEventInfo eventInfo;
    if (!message.TryGetBusinessRejectRefID(eventInfo.ID))
    {
        return;
    }
    eventInfo.Status = E_FAIL;
    message.TryGetText(eventInfo.Message);
    m_receiver->VBusinessReject(eventInfo);
}

void CFixConnection::OnSuscribeToTradeTransactionReportsAck(const FIX44::TradeTransactionReportRequestAck& message)
{
    CFxEventInfo info;
    info.ID = message.GetTradeRequestID();
    const int status = message.GetTradeTransRepRequestResult();

    if (FIX::TradeTransRepRequestResult_SUCCESS != status)
    {
        info.Status = E_FAIL;
        message.TryGetText(info.Message);
    }

    const char type = message.GetSubscriptionRequestType();
    if ((FIX::SubscriptionRequestType_SNAPSHOT == type) || (FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES == type))
    {
        int32 curNumTradeReports = message.GetCurNumTradeReports();
        int32 totNumTradeReports = message.GetTotNumTradeReports();
        bool endOfStrm = message.GetEndOfStrm();
        m_receiver->VGetTradeTransactionReportsAndSubscribeToNotifications(info, curNumTradeReports, totNumTradeReports, endOfStrm);
    }
    else if (FIX::SubscriptionRequestType_UNSUBSCRIBE == type)
    {
        m_receiver->VUnsubscribeTradeTransactionReportsNotifications(info);
    }
}

void CFixConnection::OnTradeTransactionReport(const FIX44::TradeTransactionReport& message)
{
    CFxEventInfo info;
    message.TryGetTradeRequestID(info.ID);

    CFxTradeTransactionReport report;

    int tradeTransactionReportType = 0;
    message.TryGetTradeTransReportType(tradeTransactionReportType);

    if (FIX::TradeTransReportType_OrderOpened == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_OrderOpened;
    }
    else if (FIX::TradeTransReportType_OrderCanceled == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_OrderCanceled;
    }
    else if (FIX::TradeTransReportType_OrderExpired == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_OrderExpired;
    }
    else if (FIX::TradeTransReportType_OrderFilled == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_OrderFilled;
    }
    else if (FIX::TradeTransReportType_PositionClosed == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_PositionClosed;
    }
    else if (FIX::TradeTransReportType_BalanceTrans == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_BalanceTransaction;
    }
    else if (FIX::TradeTransReportType_Credit == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_Credit;
    }
    else if (FIX::TradeTransReportType_PositionOpened == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_PositionOpened;
    }
    else if (FIX::TradeTransReportType_OrderActivated == tradeTransactionReportType)
    {
        report.TradeTransactionReportType = FxTradeTransactionReportType_OrderActivated;
    }

    int reason = 0;
    message.TryGetTradeTransReason(reason);

    if (FIX::TradeTransReason_ClientRequest == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_ClientRequest;
    }
    else if (FIX::TradeTransReason_PndOrdAct == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_PendingOrderActivation;
    }
    else if (FIX::TradeTransReason_StopOut == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_StopOut;
    }
    else if (FIX::TradeTransReason_StopLossAct == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_StopLossActivation;
    }
    else if (FIX::TradeTransReason_TakeProfitAct == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_TakeProfitActivation;
    }
    else if (FIX::TradeTransReason_DealerDecision == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_DealerDecision;
    }
    else if (FIX::TradeTransReason_Rollover == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_Rollover;
    }
    else if (FIX::TradeTransReason_DeleteAccount == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_DeleteAccount;
    }
    else if (FIX::TradeTransReason_Expired == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_Expired;
    }
    else if (FIX::TradeTransReason_TransferMoney == reason)
    {
        report.TradeTransactionReason = FxTradeTransactionReason_TransferMoney;
    }

    message.TryGetAccBalance(report.AccountBalance);
    message.TryGetAccTrAmount(report.TransactionAmount);


    message.TryGetAccTrCurry(report.TransactionCurrency);
    message.TryGetOrderID(report.Id);
    message.TryGetClOrdID(report.ClientId);
    message.TryGetOrderQty(report.Quantity);
    message.TryGetOrderLeavesQty(report.LeavesQuantity);
    message.TryGetOrderPrice(report.Price);
    message.TryGetOrderStopPx(report.StopPrice);

    char initialOrderType = 0;
    message.TryGetParentOrderType(initialOrderType);
    if (FIX::OrdType_MARKET == initialOrderType)
    {
        report.InitialTradeRecordType = FxOrderType_Market;
    }
    else if (FIX::OrdType_LIMIT == initialOrderType)
    {
        report.InitialTradeRecordType = FxOrderType_Limit;
    }
    else if (FIX::OrdType_STOP == initialOrderType)
    {
        report.InitialTradeRecordType = FxOrderType_Stop;
    }
    else if (FIX::OrdType_POSITION == initialOrderType)
    {
        report.InitialTradeRecordType = FxOrderType_Position;
    }
    else if (FIX::OrdType_STOPLIMIT == initialOrderType)
    {
        report.InitialTradeRecordType = FxOrderType_StopLimit;
    }

    char orderType = 0;
    message.TryGetOrdType(orderType);
    if (FIX::OrdType_MARKET == orderType)
    {
        report.TradeRecordType = FxOrderType_Market;
    }
    else if (FIX::OrdType_LIMIT == orderType)
    {
        report.TradeRecordType = FxOrderType_Limit;
    }
    else if (FIX::OrdType_STOP == orderType)
    {
        report.TradeRecordType = FxOrderType_Stop;
    }
    else if (FIX::OrdType_POSITION == orderType)
    {
        report.TradeRecordType = FxOrderType_Position;
    }
    else if (FIX::OrdType_STOPLIMIT == orderType)
    {
        report.TradeRecordType = FxOrderType_StopLimit;
    }

    char side = 0;
    message.TryGetSide(side);
    if (FIX::Side_BUY == side)
    {
        report.TradeRecordSide = FxTradeRecordSide_Buy;
    }
    else if (FIX::Side_SELL == side)
    {
        report.TradeRecordSide = FxTradeRecordSide_Sell;
    }
    message.TryGetSymbol(report.Symbol);
    FIX::UtcTimeStamp time(time_t(0));

    if (message.TryGetOrdCreated(time))
    {
        report.OrderCreated = time.toFileTime();
    }
    if (message.TryGetOrdModified(time))
    {
        report.OrderModified = time.toFileTime();
    }
    message.TryGetPosID(report.PositionId);
    message.TryGetPosByID(report.PositionById);
    if (message.TryGetPosOpened(time))
    {
        report.PositionOpened = time.toFileTime();
    }

    message.TryGetPosOpenReqPrice(report.PosOpenReqPrice);

    message.TryGetPosOpenPrice(report.PosOpenPrice);

    message.TryGetPosQty(report.PositionQuantity);
    message.TryGetPosLastQty(report.PositionLastQuantity);
    message.TryGetPosLeavesQty(report.PositionLeavesQuantity);

    message.TryGetPosCloseReqPrice(report.PositionCloseRequestedPrice);
    message.TryGetPosClosePrice(report.PositionClosePrice);
    if (message.TryGetPosClosed(time))
    {
        report.PositionClosed = time.toFileTime();
    }

    if (message.TryGetPosModified(time))
    {
        report.PositionModified = time.toFileTime();
    }

    message.TryGetCommission(report.Commission);
    message.TryGetAgentCommission(report.AgentCommission);

    message.TryGetSwap(report.Swap);
    message.TryGetCommCurrency(report.CommCurrency);
    message.TryGetStopLoss(report.StopLoss);
    message.TryGetTakeProfit(report.TakeProfit);
    message.TryGetTradeReportID(report.NextStreamPositionId);

    string encodedComment;
    if (message.TryGetEncodedComment(encodedComment))
        Utf8ToStd(report.Comment, encodedComment);

    string encodedTag;
    if (message.TryGetEncodedTag(encodedTag))
        Utf8ToStd(report.Tag, encodedTag);

    int magic;
    if (message.TryGetMagic(magic))
        report.Magic = magic;

    bool reducedOpenCommission;
    if (message.TryGetCommOpenReducedFlag(reducedOpenCommission))
        report.IsReducedOpenCommission = reducedOpenCommission;
    bool reducedCloseCommission;
    if (message.TryGetCommCloseReducedFlag(reducedCloseCommission))
        report.IsReducedCloseCommission = reducedCloseCommission;

    bool ioc;
    if (message.TryGetImmediateOrCancelFlag(ioc))
        report.ImmediateOrCancel = ioc;
    bool mws;
    if (message.TryGetMarketWithSlippageFlag(mws))
        report.MarketWithSlippage = mws;

    if (message.TryGetTransactTime(time))
        report.TransactionTime = time.toFileTime();

    double orderFillPrice;
    if (message.TryGetLastPx(orderFillPrice))
        report.OrderFillPrice = orderFillPrice;

    double orderLastFillAmount;
    if (message.TryGetLastQty(orderLastFillAmount))
        report.OrderLastFillAmount = orderLastFillAmount;

    double openConversionRate;
    if (message.TryGetOpenConversionRate(openConversionRate))
        report.OpenConversionRate = openConversionRate;

    double closeConversionRate;
    if (message.TryGetCloseConversionRate(closeConversionRate))
        report.CloseConversionRate = closeConversionRate;

    double reqOpenPrice;
    if (message.TryGetReqOpenPrice(reqOpenPrice))
        report.ReqOpenPrice = reqOpenPrice;
    double reqOpenQuantity;
    if (message.TryGetReqOpenQty(reqOpenQuantity))
        report.ReqOpenQuantity = reqOpenQuantity;
    double reqClosePrice;
    if (message.TryGetReqClosePrice(reqClosePrice))
        report.ReqClosePrice = reqClosePrice;
    double reqCloseQuantity;
    if (message.TryGetReqCloseQty(reqCloseQuantity))
        report.ReqCloseQuantity = reqCloseQuantity;

    message.TryGetActionID(report.ActionId);
    if (message.TryGetPosRemainingSide(side))
    {
        if (FIX::PosRemainingSide_BUY == side)
        {
            report.PosRemainingSide = FxTradeRecordSide_Buy;
        }
        else if (FIX::PosRemainingSide_SELL == side)
        {
            report.PosRemainingSide = FxTradeRecordSide_Sell;
        }
    }
    double posRemainingPrice;
    if (message.TryGetPosRemainingPrice(posRemainingPrice))
        report.PosRemainingPrice = posRemainingPrice;

    UtcTimeStamp expiration(time_t(0));
    if (message.TryGetExpireTime(expiration))
        report.Expiration = expiration.toFileTime();

    message.TryGetSrcAssetCurrency(report.SrcAssetCurrency);
    double srcAssetAmount;
    if (message.TryGetSrcAssetAmount(srcAssetAmount))
        report.SrcAssetAmount = srcAssetAmount;
    double srcAssetMovement;
    if (message.TryGetSrcAssetMovement(srcAssetMovement))
        report.SrcAssetMovement = srcAssetMovement;
    message.TryGetDstAssetCurrency(report.DstAssetCurrency);
    double dstAssetAmount;
    if (message.TryGetDstAssetAmount(dstAssetAmount))
        report.DstAssetAmount = dstAssetAmount;
    double dstAssetMovement;
    if (message.TryGetDstAssetMovement(dstAssetMovement))
        report.DstAssetMovement = dstAssetMovement;
    double maxVisibleQty;
    if (message.TryGetMaxVisibleQty(maxVisibleQty))
        report.MaxVisibleQuantity = maxVisibleQty;

    double marginCurrencyToUsdConversionRate;
    if (message.TryGetMarginCurrencyToUsdConversionRate(marginCurrencyToUsdConversionRate))
        report.MarginCurrencyToUsdConversionRate = marginCurrencyToUsdConversionRate;
    double usdToMarginCurrencyConversionRate;
    if (message.TryGetUsdToMarginCurrencyConversionRate(usdToMarginCurrencyConversionRate))
        report.UsdToMarginCurrencyConversionRate = usdToMarginCurrencyConversionRate;
    message.TryGetMarginCurrency(report.MarginCurrency);
    double profitCurrencyToUsdConversionRate;
    if (message.TryGetProfitCurrencyToUsdConversionRate(profitCurrencyToUsdConversionRate))
        report.ProfitCurrencyToUsdConversionRate = profitCurrencyToUsdConversionRate;
    double usdToProfitCurrencyConversionRate;
    if (message.TryGetUsdToProfitCurrencyConversionRate(usdToProfitCurrencyConversionRate))
        report.UsdToProfitCurrencyConversionRate = usdToProfitCurrencyConversionRate;
    message.TryGetProfitCurrency(report.ProfitCurrency);
    double srcAssetToUsdConversionRate;
    if (message.TryGetSrcAssetToUsdConversionRate(srcAssetToUsdConversionRate))
        report.SrcAssetToUsdConversionRate = srcAssetToUsdConversionRate;
    double usdToSrcAssetConversionRate;
    if (message.TryGetUsdToSrcAssetConversionRate(usdToSrcAssetConversionRate))
        report.UsdToSrcAssetConversionRate = usdToSrcAssetConversionRate;
    double dstAssetToUsdConversionRate;
    if (message.TryGetDstAssetToUsdConversionRate(dstAssetToUsdConversionRate))
        report.DstAssetToUsdConversionRate = dstAssetToUsdConversionRate;
    double usdToDstAssetConversionRate;
    if (message.TryGetUsdToDstAssetConversionRate(usdToDstAssetConversionRate))
        report.UsdToDstAssetConversionRate = usdToDstAssetConversionRate;
    message.TryGetMinCommissionCurrency(report.MinCommissionCurrency);
    double minCommissionConversionRate;
    if (message.TryGetMinCommissionConversionRate(minCommissionConversionRate))
        report.MinCommissionConversionRate = minCommissionConversionRate;

    m_receiver->VTradeTransactionReport(info, report);
}

void CFixConnection::OnPositionReport(const FIX44::PositionReport& message)
{
    CFxEventInfo info;
    const string symbol = message.GetSymbol();
    CFxPositionReport positionReport(symbol);
    positionReport.SettlementPrice = message.GetSettlPrice();
    FIX44::PositionReport::NoPositions position;
    message.getGroup(1, position);
    positionReport.BuyAmount = position.GetLongQty();
    positionReport.SellAmount = position.GetShortQty();
    double balance = 0;
    if (message.TryGetAccBalance(balance))
    {
        positionReport.Balance = balance;
    }
    double buyPrice = 0;
    if (position.TryGetLongPrice(buyPrice))
    {
        positionReport.BuyPrice = buyPrice;
    }
    double sellPrice = 0;
    if (position.TryGetShortPrice(sellPrice))
    {
        positionReport.SellPrice = sellPrice;
    }
    message.TryGetCommission(positionReport.Commission);
    message.TryGetAgentCommission(positionReport.AgentCommission);
    message.TryGetSwap(positionReport.Swap);
    FIX::PosReportType ptype = message.GetPosReportType();
    switch (ptype)
    {
        case FIX::PosReportType_Login:
            positionReport.PosReportType = FxPosReportType_Login;
            break;

        case FIX::PosReportType_Response:
            positionReport.PosReportType = FxPosReportType_Response;
            break;

        case FIX::PosReportType_Rollover:
            positionReport.PosReportType = FxPosReportType_Rollover;
            break;

        case FIX::PosReportType_CreatePosition:
            positionReport.PosReportType = FxPosReportType_CreatePosition;
            break;

        case FIX::PosReportType_ModifyPosition:
            positionReport.PosReportType = FxPosReportType_ModifyPosition;
            break;

        case FIX::PosReportType_CancelPosition:
            positionReport.PosReportType = FxPosReportType_CancelPosition;
            break;

        case FIX::PosReportType_ClosePosition:
            positionReport.PosReportType = FxPosReportType_ClosePosition;
            break;

        default:
            positionReport.PosReportType = FxPosReportType_UNKNOWN;
    }

    m_receiver->VPositionReport(info, positionReport);
    UNREFERENCED_PARAMETER(message);
}

void CFixConnection::OnNotification(const FIX44::Notification& message)
{
    CFxEventInfo info;
    CNotification notification;
    message.TryGetText(notification.Text);

    const int severity = message.GetNotifSeverity();
    if (FIX::NotifSeverity_INFO == severity)
    {
        notification.Severity = Severity_Event;
    }
    else if (FIX::NotifSeverity_WARNING == severity)
    {
        notification.Severity = Severity_Warning;
    }
    else if (FIX::NotifSeverity_ERROR == severity)
    {
        notification.Severity = Severity_Error;
    }
    else
    {
        notification.Severity = Severity_Event;
    }

    const int notifyCode = message.GetNotifCode();
    if (FIX::NotifCode_MARGIN_CALL == notifyCode)
    {
        notification.Type = NotificationType_MarginCall;
    }
    else if (FIX::NotifCode_MARGIN_CALL_REVOCATION == notifyCode)
    {
        notification.Type = NotificationType_MarginCallRevocation;
    }
    else if (FIX::NotifCode_STOP_OUT == notifyCode)
    {
        notification.Type = NotificationType_StopOut;
    }
    else if (FIX::NotifCode_BALANCE == notifyCode)
    {
        notification.Type = NotificationType_Balance;
    }
    else if (FIX::NotifCode_CONFIG_UPDATED == notifyCode)
    {
        notification.Type = NotificationType_ConfigUpdated;
    }
    else
    {
        return;
    }

    if (NotificationType_Balance == notification.Type)
    {
        notification.Balance = message.GetAccBalance();
        notification.TransactionAmount = message.GetAccTrAmount();
        notification.TransactionCurrency = message.GetAccTrCurry();
    }

    m_receiver->VNotify(info, notification);
}

void CFixConnection::OnComponentsInfoReport(const FIX44::ComponentsInfoReport& message)
{
    CFxEventInfo info;
    message.TryGetCompReqID(info.ID);
    const int version = message.GetServerQuoteHistoryVersion();
    m_receiver->VQuotesHistoryResponse(info, version);
}

void CFixConnection::OnDailyAccountSnapshotRequestAck(const FIX44::DailyAccountSnapshotRequestAck& message)
{
    CFxEventInfo info;
    info.ID = message.GetSnapshotRequestID();
    const int status = message.GetSnapshotRequestResult();

    if (FIX::SnapshotRequestResult_SUCCESS != status)
    {
        info.Status = E_FAIL;
        message.TryGetText(info.Message);
    }

    int32 curNumTradeReports = message.GetCurNumTradeReports();
    int32 totNumTradeReports = message.GetTotNumTradeReports();
    bool endOfStrm = message.GetEndOfStrm();
    m_receiver->VGetDailyAccountSnapshotReports(info, curNumTradeReports, totNumTradeReports, endOfStrm);
}

void CFixConnection::OnDailyAccountSnapshotReport(const FIX44::DailyAccountSnapshotReport& message)
{
    CFxEventInfo info;
    message.TryGetSnapshotRequestID(info.ID);

    CFxDailyAccountSnapshotReport report;

    FIX::UtcTimeStamp timestamp(time_t(0));
    if (message.TryGetTimestamp(timestamp))
        report.Timestamp = timestamp.toFileTime();

    report.AccountId = message.GetAccount();

    const FIX::AccountingType accountingType = message.GetAccountingType();
    if (FIX::AccountingType_GROSS == accountingType)
    {
        report.Type = FxAccountType_Gross;
    }
    else if (FIX::AccountingType_NET == accountingType)
    {
        report.Type = FxAccountType_Net;
    }
    else if (FIX::AccountingType_CASH == accountingType)
    {
        report.Type = FxAccountType_Cash;
    }
    else
    {
        return;
    }

    report.BalanceCurrency = message.GetCurrency();
    report.Leverage = message.GetLeverage();
    report.Balance = message.GetBalance();
    report.Margin = message.GetMargin();
    report.MarginLevel = message.GetMarginLevel();
    report.Equity = message.GetEquity();
    report.Swap = message.GetSwap();
    report.Profit = message.GetProfit();
    report.Commission = message.GetCommission();
    report.AgentCommission = message.GetAgentCommission();

    auto count = 0;
    if (message.TryGetNoAssets(count))
    {
        report.Assets.reserve(static_cast<size_t>(count));
        for (int index = 1; index <= count; ++index)
        {
            FIX44::AccountInfo::NoAssets group;
            message.getGroup(index, group);

            CAssetInfo asset;
            asset.Currency = group.GetAssetCurrency();
            asset.Balance = group.GetAssetBalance();
            group.TryGetAssetLockedAmt(asset.LockedAmount);
            group.TryGetAssetTradeAmt(asset.TradeAmount);
            double currencyToUsdConversionRate;
            if (group.TryGetSrcAssetToUsdConversionRate(currencyToUsdConversionRate))
                asset.CurrencyToUsdConversionRate = currencyToUsdConversionRate;
            double usdToCurrencyConversionRate;
            if (group.TryGetUsdToSrcAssetConversionRate(usdToCurrencyConversionRate))
                asset.UsdToCurrencyConversionRate = usdToCurrencyConversionRate;

            if (asset.Balance == 0.0)
                continue;

            report.Assets.push_back(asset);
        }
    }

    count = 0;
    if (message.TryGetNoPositions(count))
    {
        report.Positions.reserve(static_cast<size_t>(count));
        for (int index = 1; index <= count; ++index)
        {
            FIX44::DailyAccountSnapshotReport::NoPositions group;
            message.getGroup(index, group);

            CFxPositionReport position;
            position.Symbol = group.GetSymbol();
            position.SettlementPrice = group.GetSettlPrice();
            position.AgentCommission = 0;
            position.Commission = 0;
            group.TryGetCommission(position.Commission);
            position.Swap = 0;
            group.TryGetSwap(position.Swap);
            if (group.GetSide() == '1')
            {
                position.BuyAmount = group.GetLeavesQty();
                position.SellAmount = 0;
                position.BuyPrice = group.GetSettlPrice();
            }
            else if (group.GetSide() == '2')
            {
                position.BuyAmount = 0;
                position.SellAmount = group.GetLeavesQty();
                position.SellPrice = group.GetSettlPrice();
            }
            FIX::UtcTimeStamp posmodified(time_t(0));
            if (group.TryGetPosModified(posmodified))
                position.PosModified = posmodified.toFileTime();
            group.TryGetPosID(position.PosID);
            double margin;
            if (group.TryGetMargin(margin))
                position.Margin = margin;
            double profit;
            if (group.TryGetProfit(profit))
                position.Profit = profit;
            double currentBestAsk;
            if (group.TryGetCurrentBestAsk(currentBestAsk))
                position.CurrentBestAsk = currentBestAsk;
            double currentBestBid;
            if (group.TryGetCurrentBestBid(currentBestBid))
                position.CurrentBestBid = currentBestBid;
            position.PosReportType = FxPosReportType_Response;
            report.Positions.push_back(position);
        }
    }

    message.TryGetBalance(report.Balance);
    message.TryGetSnapshotRequestID(report.NextStreamPositionId);

    double balanceCurrencyToUsdConversionRate;
    if (message.TryGetBalanceCurrencyToUsdConversionRate(balanceCurrencyToUsdConversionRate))
        report.BalanceCurrencyToUsdConversionRate = balanceCurrencyToUsdConversionRate;
    double usdToBalanceCurrencyConversionRate;
    if (message.TryGetUsdToBalanceCurrencyConversionRate(usdToBalanceCurrencyConversionRate))
        report.UsdToBalanceCurrencyConversionRate = usdToBalanceCurrencyConversionRate;
    double profitCurrencyToUsdConversionRate;
    if (message.TryGetProfitCurrencyToUsdConversionRate(profitCurrencyToUsdConversionRate))
        report.ProfitCurrencyToUsdConversionRate = profitCurrencyToUsdConversionRate;
    double usdToProfitCurrencyConversionRate;
    if (message.TryGetUsdToProfitCurrencyConversionRate(usdToProfitCurrencyConversionRate))
        report.UsdToProfitCurrencyConversionRate = usdToProfitCurrencyConversionRate;

    m_receiver->VDailyAccountSnapshotReport(info, report);
}
