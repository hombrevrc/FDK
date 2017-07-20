﻿namespace SoftFX.Extended.Core
{
    using System;
    using System.Linq;
    using SoftFX.Extended.Data;
    using SoftFX.Lrp;

    unsafe struct FxDataTrade
    {
        #region Creating and Converting

        public static FxDataTrade Create(string name, string connectionString)
        {
            var handle = Native.TradeServer.Create(name, connectionString);
            return new FxDataTrade(handle);
        }

        public FxDataTrade(LPtr handle)
        {
            this.handle = handle;
        }

        public FxHandle Handle
        {
            get
            {
                return new FxHandle(this.handle);
            }
        }

        public FxDataClient DataClient
        {
            get
            {
                return new FxDataClient(this.handle);
            }
        }

        #endregion

        #region Server Methods

        public TradeServerInfo GetTradeServerInfo(int timeoutInMilliseconds)
        {
            this.VerifyInitialized();

            return Native.TradeServer.GetTradeServerInfo(handle, (uint)timeoutInMilliseconds);
        }

        public FxOrder OpenNewOrder(string operationId, string symbol, TradeCommand command, TradeRecordSide side, double volume, double? maxVisibleVolume, double? price, double? stopPrice, double? stopLoss, double? takeProfit, DateTime? expiration, string comment, string tag, int? magic, int timeoutInMilliseconds)
        {
            if (operationId == null)
                operationId = string.Empty;

            this.VerifyInitialized();

            var order = new FxOrder(symbol, (int)command, side, volume, maxVisibleVolume, price, stopPrice, stopLoss, takeProfit, expiration, comment, tag, magic);

            return Native.TradeServer.OpenNewOrder(this.handle, operationId, order, (uint)timeoutInMilliseconds);
        }

        public FxOrder ModifyOrder(string operationId, string orderId, string clientId, string symbol, TradeRecordType type, TradeRecordSide side,
                                        double? newVolume, double? newMaxVisibleVolume, double? newPrice, double? newStopPrice, double? newStopLoss, double? newTakeProfit, DateTime? newExpiration, string newComment, string newTag, int? newMagic, int timeoutInMilliseconds)
        {
            if (operationId == null)
                operationId = string.Empty;
            if (orderId == null)
                orderId = string.Empty;
            if (clientId == null)
                clientId = string.Empty;
            if (newComment == null)
                newComment = string.Empty;
            if (newComment == string.Empty)
                newComment = "<empty>";
            if (newTag == null)
                newTag = string.Empty;
            if (newTag == string.Empty)
                newTag = "<empty>";

            this.VerifyInitialized();

            var order = new FxOrder(orderId, clientId, symbol, (int)type, side, newVolume, newMaxVisibleVolume, newPrice, newStopPrice, newStopLoss, newTakeProfit, newExpiration, newComment, newTag, newMagic);

            return Native.TradeServer.ModifyOrder(this.handle, operationId, order, (uint)timeoutInMilliseconds);
        }

        public void DeleteOrder(string operationId, string orderId, string clientId, TradeRecordSide side, int timeoutInMilliseconds)
        {
            if (operationId == null)
                operationId = string.Empty;
            if (orderId == null)
                orderId = string.Empty;
            if (clientId == null)
                clientId = string.Empty;

            this.VerifyInitialized();

            Native.TradeServer.DeleteOrder(this.handle, operationId, orderId, clientId, side, (uint)timeoutInMilliseconds);
        }

        public ClosePositionResult CloseOrder(string operationId, string orderId, double? closingVolume, int timeoutInMilliseconds)
        {
            if (operationId == null)
                operationId = string.Empty;
            if (orderId == null)
                orderId = string.Empty;

            this.VerifyInitialized();

            return Native.TradeServer.CloseOrder(this.handle, operationId, orderId, closingVolume, (uint)timeoutInMilliseconds);
        }

        public bool CloseByOrders(string operationId, string firstOrderId, string secondOrderId, int timeoutInMilliseconds)
        {
            if (operationId == null)
                operationId = string.Empty;
            if (firstOrderId == null)
                firstOrderId = string.Empty;
            if (secondOrderId == null)
                secondOrderId = string.Empty;

            this.VerifyInitialized();

            return Native.TradeServer.CloseByPositions(this.handle, operationId, firstOrderId, secondOrderId, (uint)timeoutInMilliseconds);
        }

        public int CloseAllOrders(string operationId, int timeoutInMilliseconds)
        {
            if (operationId == null)
                operationId = string.Empty;

            this.VerifyInitialized();

            return (int)Native.TradeServer.CloseAllPositions(this.handle, operationId, (uint)timeoutInMilliseconds);
        }

        public TradeRecord[] GetOrders(DataTrade dataTrade, int timeoutInMilliseconds)
        {
            this.VerifyInitialized();

            var orders = Native.TradeServer.GetRecords(this.handle, (uint)timeoutInMilliseconds);

            return orders.Select(o => new TradeRecord(dataTrade, o))
                         .ToArray();
        }

        public LPtr GetTradeTransactionReportsAndSubscribeToNotifications(TimeDirection direction, bool subscribe, DateTime? from, DateTime? to, int preferedBufferSize, bool? skipCancel, int timeoutInMilliseconds)
        {
            this.VerifyInitialized();

            int? skipCancelInt = null;
            if (skipCancel.HasValue)
                skipCancelInt = skipCancel.Value ? 1 : 0;
            return Native.TradeServer.GetTradeTransactionReportsAndSubscribe(this.handle, (int)direction, subscribe, from, to, (UInt32)preferedBufferSize, skipCancelInt, (UInt32)timeoutInMilliseconds);
        }

        public void UnsubscribeTradeTransactionReports(int timeoutInMilliseconds)
        {
            this.VerifyInitialized();

            Native.TradeServer.UnsubscribeTradeTransactionReports(this.handle, (uint)timeoutInMilliseconds);
        }

        #endregion

        #region Local Methods

        public TradeRecord[] GetCacheOrders(DataTrade dataTrade)
        {
            this.VerifyInitialized();

            var orders = Native.TradeCache.GetRecords(this.handle);

            return orders.Select(o => new TradeRecord(dataTrade, o))
                         .ToArray();
        }

        public Position[] GetCachePositions()
        {
            this.VerifyInitialized();

            return Native.TradeCache.GetPositions(this.handle);
        }

        public AccountInfo GetCacheAccountInfo()
        {
            this.VerifyInitialized();

            return Native.TradeCache.GetAccountInfo(this.handle);
        }

        #endregion

        void VerifyInitialized()
        {
            if (this.handle.IsZero)
                throw new InvalidOperationException(string.Format("Cannot use not initialized {0} object.", this.GetType().Name));
        }

        #region Members

        readonly LPtr handle;

        #endregion
    }
}
