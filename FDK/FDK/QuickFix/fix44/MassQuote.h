#ifndef FIX44_MASSQUOTE_H
#define FIX44_MASSQUOTE_H

#include "Message.h"

namespace FIX44
{

  class MassQuote : public Message
  {
  public:
    MassQuote() : Message(MsgType()) {}
    MassQuote(const FIX::Message& m) : Message(m) {}
    MassQuote(const Message& m) : Message(m) {}
    MassQuote(const MassQuote& m) : Message(m) {}
    static FIX::MsgType MsgType() { return FIX::MsgType("i"); }

    MassQuote(
      const FIX::QuoteID& aQuoteID )
    : Message(MsgType())
    {
      set(aQuoteID);
    }

    FIELD_SET(*this, FIX::QuoteReqID);
    FIELD_SET_EX(std::string, QuoteReqID);
    FIELD_SET(*this, FIX::QuoteID);
    FIELD_SET_EX(std::string, QuoteID);
    FIELD_SET(*this, FIX::QuoteType);
    FIELD_SET_EX(int, QuoteType);
    FIELD_SET(*this, FIX::QuoteResponseLevel);
    FIELD_SET_EX(int, QuoteResponseLevel);
    FIELD_SET(*this, FIX::NoPartyIDs);
    FIELD_SET_EX(int, NoPartyIDs);
    class NoPartyIDs: public FIX::Group
    {
    public:
    NoPartyIDs() : FIX::Group(453,448,FIX::message_order(448,447,452,802,0)) {}
      FIELD_SET(*this, FIX::PartyID);
      FIELD_SET_EX(std::string, PartyID);
      FIELD_SET(*this, FIX::PartyIDSource);
      FIELD_SET_EX(char, PartyIDSource);
      FIELD_SET(*this, FIX::PartyRole);
      FIELD_SET_EX(int, PartyRole);
      FIELD_SET(*this, FIX::NoPartySubIDs);
      FIELD_SET_EX(int, NoPartySubIDs);
      class NoPartySubIDs: public FIX::Group
      {
      public:
      NoPartySubIDs() : FIX::Group(802,523,FIX::message_order(523,803,0)) {}
        FIELD_SET(*this, FIX::PartySubID);
        FIELD_SET_EX(std::string, PartySubID);
        FIELD_SET(*this, FIX::PartySubIDType);
        FIELD_SET_EX(int, PartySubIDType);
      };
    };
    FIELD_SET(*this, FIX::Account);
    FIELD_SET_EX(std::string, Account);
    FIELD_SET(*this, FIX::AcctIDSource);
    FIELD_SET_EX(int, AcctIDSource);
    FIELD_SET(*this, FIX::AccountType);
    FIELD_SET_EX(int, AccountType);
    FIELD_SET(*this, FIX::DefBidSize);
    FIELD_SET_EX(double, DefBidSize);
    FIELD_SET(*this, FIX::DefOfferSize);
    FIELD_SET_EX(double, DefOfferSize);
    FIELD_SET(*this, FIX::NoQuoteSets);
    FIELD_SET_EX(int, NoQuoteSets);
    class NoQuoteSets: public FIX::Group
    {
    public:
    NoQuoteSets() : FIX::Group(296,302,FIX::message_order(302,311,312,309,305,457,462,463,310,763,313,542,241,242,243,244,245,246,256,595,592,593,594,247,316,941,317,436,435,308,306,362,363,307,364,365,877,878,318,879,810,882,883,884,885,886,367,304,893,295,0)) {}
      FIELD_SET(*this, FIX::QuoteSetID);
      FIELD_SET_EX(std::string, QuoteSetID);
      FIELD_SET(*this, FIX::UnderlyingSymbol);
      FIELD_SET_EX(std::string, UnderlyingSymbol);
      FIELD_SET(*this, FIX::UnderlyingSymbolSfx);
      FIELD_SET_EX(std::string, UnderlyingSymbolSfx);
      FIELD_SET(*this, FIX::UnderlyingSecurityID);
      FIELD_SET_EX(std::string, UnderlyingSecurityID);
      FIELD_SET(*this, FIX::UnderlyingSecurityIDSource);
      FIELD_SET_EX(std::string, UnderlyingSecurityIDSource);
      FIELD_SET(*this, FIX::UnderlyingProduct);
      FIELD_SET_EX(int, UnderlyingProduct);
      FIELD_SET(*this, FIX::UnderlyingCFICode);
      FIELD_SET_EX(std::string, UnderlyingCFICode);
      FIELD_SET(*this, FIX::UnderlyingSecurityType);
      FIELD_SET_EX(std::string, UnderlyingSecurityType);
      FIELD_SET(*this, FIX::UnderlyingSecuritySubType);
      FIELD_SET_EX(std::string, UnderlyingSecuritySubType);
      FIELD_SET(*this, FIX::UnderlyingMaturityMonthYear);
      FIELD_SET(*this, FIX::UnderlyingMaturityDate);
      FIELD_SET_EX(std::string, UnderlyingMaturityDate);
      FIELD_SET(*this, FIX::UnderlyingCouponPaymentDate);
      FIELD_SET_EX(std::string, UnderlyingCouponPaymentDate);
      FIELD_SET(*this, FIX::UnderlyingIssueDate);
      FIELD_SET_EX(std::string, UnderlyingIssueDate);
      FIELD_SET(*this, FIX::UnderlyingRepoCollateralSecurityType);
      FIELD_SET_EX(int, UnderlyingRepoCollateralSecurityType);
      FIELD_SET(*this, FIX::UnderlyingRepurchaseTerm);
      FIELD_SET_EX(int, UnderlyingRepurchaseTerm);
      FIELD_SET(*this, FIX::UnderlyingRepurchaseRate);
      FIELD_SET_EX(double, UnderlyingRepurchaseRate);
      FIELD_SET(*this, FIX::UnderlyingFactor);
      FIELD_SET_EX(double, UnderlyingFactor);
      FIELD_SET(*this, FIX::UnderlyingCreditRating);
      FIELD_SET_EX(std::string, UnderlyingCreditRating);
      FIELD_SET(*this, FIX::UnderlyingInstrRegistry);
      FIELD_SET_EX(std::string, UnderlyingInstrRegistry);
      FIELD_SET(*this, FIX::UnderlyingCountryOfIssue);
      FIELD_SET(*this, FIX::UnderlyingStateOrProvinceOfIssue);
      FIELD_SET_EX(std::string, UnderlyingStateOrProvinceOfIssue);
      FIELD_SET(*this, FIX::UnderlyingLocaleOfIssue);
      FIELD_SET_EX(std::string, UnderlyingLocaleOfIssue);
      FIELD_SET(*this, FIX::UnderlyingRedemptionDate);
      FIELD_SET_EX(std::string, UnderlyingRedemptionDate);
      FIELD_SET(*this, FIX::UnderlyingStrikePrice);
      FIELD_SET_EX(double, UnderlyingStrikePrice);
      FIELD_SET(*this, FIX::UnderlyingStrikeCurrency);
      FIELD_SET_EX(std::string, UnderlyingStrikeCurrency);
      FIELD_SET(*this, FIX::UnderlyingOptAttribute);
      FIELD_SET_EX(char, UnderlyingOptAttribute);
      FIELD_SET(*this, FIX::UnderlyingContractMultiplier);
      FIELD_SET_EX(double, UnderlyingContractMultiplier);
      FIELD_SET(*this, FIX::UnderlyingCouponRate);
      FIELD_SET_EX(double, UnderlyingCouponRate);
      FIELD_SET(*this, FIX::UnderlyingSecurityExchange);
      FIELD_SET(*this, FIX::UnderlyingIssuer);
      FIELD_SET_EX(std::string, UnderlyingIssuer);
      FIELD_SET(*this, FIX::EncodedUnderlyingIssuerLen);
      FIELD_SET_EX(int, EncodedUnderlyingIssuerLen);
      FIELD_SET(*this, FIX::EncodedUnderlyingIssuer);
      FIELD_SET_EX(std::string, EncodedUnderlyingIssuer);
      FIELD_SET(*this, FIX::UnderlyingSecurityDesc);
      FIELD_SET_EX(std::string, UnderlyingSecurityDesc);
      FIELD_SET(*this, FIX::EncodedUnderlyingSecurityDescLen);
      FIELD_SET_EX(int, EncodedUnderlyingSecurityDescLen);
      FIELD_SET(*this, FIX::EncodedUnderlyingSecurityDesc);
      FIELD_SET_EX(std::string, EncodedUnderlyingSecurityDesc);
      FIELD_SET(*this, FIX::UnderlyingCPProgram);
      FIELD_SET_EX(std::string, UnderlyingCPProgram);
      FIELD_SET(*this, FIX::UnderlyingCPRegType);
      FIELD_SET_EX(std::string, UnderlyingCPRegType);
      FIELD_SET(*this, FIX::UnderlyingCurrency);
      FIELD_SET_EX(std::string, UnderlyingCurrency);
      FIELD_SET(*this, FIX::UnderlyingQty);
      FIELD_SET_EX(double, UnderlyingQty);
      FIELD_SET(*this, FIX::UnderlyingPx);
      FIELD_SET_EX(double, UnderlyingPx);
      FIELD_SET(*this, FIX::UnderlyingDirtyPrice);
      FIELD_SET_EX(double, UnderlyingDirtyPrice);
      FIELD_SET(*this, FIX::UnderlyingEndPrice);
      FIELD_SET_EX(double, UnderlyingEndPrice);
      FIELD_SET(*this, FIX::UnderlyingStartValue);
      FIELD_SET_EX(double, UnderlyingStartValue);
      FIELD_SET(*this, FIX::UnderlyingCurrentValue);
      FIELD_SET_EX(double, UnderlyingCurrentValue);
      FIELD_SET(*this, FIX::UnderlyingEndValue);
      FIELD_SET_EX(double, UnderlyingEndValue);
      FIELD_SET(*this, FIX::NoUnderlyingSecurityAltID);
      FIELD_SET_EX(int, NoUnderlyingSecurityAltID);
      class NoUnderlyingSecurityAltID: public FIX::Group
      {
      public:
      NoUnderlyingSecurityAltID() : FIX::Group(457,458,FIX::message_order(458,459,0)) {}
        FIELD_SET(*this, FIX::UnderlyingSecurityAltID);
        FIELD_SET_EX(std::string, UnderlyingSecurityAltID);
        FIELD_SET(*this, FIX::UnderlyingSecurityAltIDSource);
        FIELD_SET_EX(std::string, UnderlyingSecurityAltIDSource);
      };
      FIELD_SET(*this, FIX::QuoteSetValidUntilTime);
      FIELD_SET_EX(FIX::UtcTimeStamp, QuoteSetValidUntilTime);
      FIELD_SET(*this, FIX::TotNoQuoteEntries);
      FIELD_SET_EX(int, TotNoQuoteEntries);
      FIELD_SET(*this, FIX::LastFragment);
      FIELD_SET_EX(bool, LastFragment);
      FIELD_SET(*this, FIX::NoQuoteEntries);
      FIELD_SET_EX(int, NoQuoteEntries);
      class NoQuoteEntries: public FIX::Group
      {
      public:
      NoQuoteEntries() : FIX::Group(295,299,FIX::message_order(299,55,65,48,22,454,460,461,167,762,200,541,224,225,239,226,227,228,255,543,470,471,472,240,202,947,206,231,223,207,106,348,349,107,350,351,691,667,875,876,864,873,874,555,132,133,134,135,62,188,190,189,191,631,632,633,634,60,336,625,64,40,193,192,642,643,15,0)) {}
        FIELD_SET(*this, FIX::QuoteEntryID);
        FIELD_SET_EX(std::string, QuoteEntryID);
        FIELD_SET(*this, FIX::Symbol);
        FIELD_SET_EX(std::string, Symbol);
        FIELD_SET(*this, FIX::SymbolSfx);
        FIELD_SET_EX(std::string, SymbolSfx);
        FIELD_SET(*this, FIX::SecurityID);
        FIELD_SET_EX(std::string, SecurityID);
        FIELD_SET(*this, FIX::SecurityIDSource);
        FIELD_SET_EX(std::string, SecurityIDSource);
        FIELD_SET(*this, FIX::Product);
        FIELD_SET_EX(int, Product);
        FIELD_SET(*this, FIX::CFICode);
        FIELD_SET_EX(std::string, CFICode);
        FIELD_SET(*this, FIX::SecurityType);
        FIELD_SET_EX(std::string, SecurityType);
        FIELD_SET(*this, FIX::SecuritySubType);
        FIELD_SET_EX(std::string, SecuritySubType);
        FIELD_SET(*this, FIX::MaturityMonthYear);
        FIELD_SET(*this, FIX::MaturityDate);
        FIELD_SET_EX(std::string, MaturityDate);
        FIELD_SET(*this, FIX::CouponPaymentDate);
        FIELD_SET_EX(std::string, CouponPaymentDate);
        FIELD_SET(*this, FIX::IssueDate);
        FIELD_SET_EX(std::string, IssueDate);
        FIELD_SET(*this, FIX::RepoCollateralSecurityType);
        FIELD_SET_EX(int, RepoCollateralSecurityType);
        FIELD_SET(*this, FIX::RepurchaseTerm);
        FIELD_SET_EX(int, RepurchaseTerm);
        FIELD_SET(*this, FIX::RepurchaseRate);
        FIELD_SET_EX(double, RepurchaseRate);
        FIELD_SET(*this, FIX::Factor);
        FIELD_SET_EX(double, Factor);
        FIELD_SET(*this, FIX::CreditRating);
        FIELD_SET_EX(std::string, CreditRating);
        FIELD_SET(*this, FIX::InstrRegistry);
        FIELD_SET_EX(std::string, InstrRegistry);
        FIELD_SET(*this, FIX::CountryOfIssue);
        FIELD_SET(*this, FIX::StateOrProvinceOfIssue);
        FIELD_SET_EX(std::string, StateOrProvinceOfIssue);
        FIELD_SET(*this, FIX::LocaleOfIssue);
        FIELD_SET_EX(std::string, LocaleOfIssue);
        FIELD_SET(*this, FIX::RedemptionDate);
        FIELD_SET_EX(std::string, RedemptionDate);
        FIELD_SET(*this, FIX::StrikePrice);
        FIELD_SET_EX(double, StrikePrice);
        FIELD_SET(*this, FIX::StrikeCurrency);
        FIELD_SET_EX(std::string, StrikeCurrency);
        FIELD_SET(*this, FIX::OptAttribute);
        FIELD_SET_EX(char, OptAttribute);
        FIELD_SET(*this, FIX::ContractMultiplier);
        FIELD_SET_EX(double, ContractMultiplier);
        FIELD_SET(*this, FIX::CouponRate);
        FIELD_SET_EX(double, CouponRate);
        FIELD_SET(*this, FIX::SecurityExchange);
        FIELD_SET(*this, FIX::Issuer);
        FIELD_SET_EX(std::string, Issuer);
        FIELD_SET(*this, FIX::EncodedIssuerLen);
        FIELD_SET_EX(int, EncodedIssuerLen);
        FIELD_SET(*this, FIX::EncodedIssuer);
        FIELD_SET_EX(std::string, EncodedIssuer);
        FIELD_SET(*this, FIX::SecurityDesc);
        FIELD_SET_EX(std::string, SecurityDesc);
        FIELD_SET(*this, FIX::EncodedSecurityDescLen);
        FIELD_SET_EX(int, EncodedSecurityDescLen);
        FIELD_SET(*this, FIX::EncodedSecurityDesc);
        FIELD_SET_EX(std::string, EncodedSecurityDesc);
        FIELD_SET(*this, FIX::Pool);
        FIELD_SET_EX(std::string, Pool);
        FIELD_SET(*this, FIX::ContractSettlMonth);
        FIELD_SET(*this, FIX::CPProgram);
        FIELD_SET_EX(int, CPProgram);
        FIELD_SET(*this, FIX::CPRegType);
        FIELD_SET_EX(std::string, CPRegType);
        FIELD_SET(*this, FIX::DatedDate);
        FIELD_SET_EX(std::string, DatedDate);
        FIELD_SET(*this, FIX::InterestAccrualDate);
        FIELD_SET_EX(std::string, InterestAccrualDate);
        FIELD_SET(*this, FIX::NoSecurityAltID);
        FIELD_SET_EX(int, NoSecurityAltID);
        class NoSecurityAltID: public FIX::Group
        {
        public:
        NoSecurityAltID() : FIX::Group(454,455,FIX::message_order(455,456,0)) {}
          FIELD_SET(*this, FIX::SecurityAltID);
          FIELD_SET_EX(std::string, SecurityAltID);
          FIELD_SET(*this, FIX::SecurityAltIDSource);
          FIELD_SET_EX(std::string, SecurityAltIDSource);
        };
        FIELD_SET(*this, FIX::NoEvents);
        FIELD_SET_EX(int, NoEvents);
        class NoEvents: public FIX::Group
        {
        public:
        NoEvents() : FIX::Group(864,865,FIX::message_order(865,866,867,868,0)) {}
          FIELD_SET(*this, FIX::EventType);
          FIELD_SET_EX(int, EventType);
          FIELD_SET(*this, FIX::EventDate);
          FIELD_SET_EX(std::string, EventDate);
          FIELD_SET(*this, FIX::EventPx);
          FIELD_SET_EX(double, EventPx);
          FIELD_SET(*this, FIX::EventText);
          FIELD_SET_EX(std::string, EventText);
        };
        FIELD_SET(*this, FIX::BidPx);
        FIELD_SET_EX(double, BidPx);
        FIELD_SET(*this, FIX::OfferPx);
        FIELD_SET_EX(double, OfferPx);
        FIELD_SET(*this, FIX::BidSize);
        FIELD_SET_EX(double, BidSize);
        FIELD_SET(*this, FIX::OfferSize);
        FIELD_SET_EX(double, OfferSize);
        FIELD_SET(*this, FIX::ValidUntilTime);
        FIELD_SET_EX(FIX::UtcTimeStamp, ValidUntilTime);
        FIELD_SET(*this, FIX::BidSpotRate);
        FIELD_SET_EX(double, BidSpotRate);
        FIELD_SET(*this, FIX::OfferSpotRate);
        FIELD_SET_EX(double, OfferSpotRate);
        FIELD_SET(*this, FIX::BidForwardPoints);
        FIELD_SET(*this, FIX::OfferForwardPoints);
        FIELD_SET(*this, FIX::MidPx);
        FIELD_SET_EX(double, MidPx);
        FIELD_SET(*this, FIX::BidYield);
        FIELD_SET_EX(double, BidYield);
        FIELD_SET(*this, FIX::MidYield);
        FIELD_SET_EX(double, MidYield);
        FIELD_SET(*this, FIX::OfferYield);
        FIELD_SET_EX(double, OfferYield);
        FIELD_SET(*this, FIX::TransactTime);
        FIELD_SET_EX(FIX::UtcTimeStamp, TransactTime);
        FIELD_SET(*this, FIX::TradingSessionID);
        FIELD_SET_EX(std::string, TradingSessionID);
        FIELD_SET(*this, FIX::TradingSessionSubID);
        FIELD_SET_EX(std::string, TradingSessionSubID);
        FIELD_SET(*this, FIX::SettlDate);
        FIELD_SET_EX(std::string, SettlDate);
        FIELD_SET(*this, FIX::OrdType);
        FIELD_SET_EX(char, OrdType);
        FIELD_SET(*this, FIX::SettlDate2);
        FIELD_SET_EX(std::string, SettlDate2);
        FIELD_SET(*this, FIX::OrderQty2);
        FIELD_SET_EX(double, OrderQty2);
        FIELD_SET(*this, FIX::BidForwardPoints2);
        FIELD_SET(*this, FIX::OfferForwardPoints2);
        FIELD_SET(*this, FIX::Currency);
        FIELD_SET_EX(std::string, Currency);
        FIELD_SET(*this, FIX::NoLegs);
        FIELD_SET_EX(int, NoLegs);
        class NoLegs: public FIX::Group
        {
        public:
        NoLegs() : FIX::Group(555,600,FIX::message_order(600,601,602,603,604,607,608,609,764,610,611,248,249,250,251,252,253,257,599,596,597,598,254,612,942,613,614,615,616,617,618,619,620,621,622,623,624,556,740,739,955,956,0)) {}
          FIELD_SET(*this, FIX::LegSymbol);
          FIELD_SET_EX(std::string, LegSymbol);
          FIELD_SET(*this, FIX::LegSymbolSfx);
          FIELD_SET_EX(std::string, LegSymbolSfx);
          FIELD_SET(*this, FIX::LegSecurityID);
          FIELD_SET_EX(std::string, LegSecurityID);
          FIELD_SET(*this, FIX::LegSecurityIDSource);
          FIELD_SET_EX(std::string, LegSecurityIDSource);
          FIELD_SET(*this, FIX::LegProduct);
          FIELD_SET_EX(int, LegProduct);
          FIELD_SET(*this, FIX::LegCFICode);
          FIELD_SET_EX(std::string, LegCFICode);
          FIELD_SET(*this, FIX::LegSecurityType);
          FIELD_SET_EX(std::string, LegSecurityType);
          FIELD_SET(*this, FIX::LegSecuritySubType);
          FIELD_SET_EX(std::string, LegSecuritySubType);
          FIELD_SET(*this, FIX::LegMaturityMonthYear);
          FIELD_SET(*this, FIX::LegMaturityDate);
          FIELD_SET_EX(std::string, LegMaturityDate);
          FIELD_SET(*this, FIX::LegCouponPaymentDate);
          FIELD_SET_EX(std::string, LegCouponPaymentDate);
          FIELD_SET(*this, FIX::LegIssueDate);
          FIELD_SET_EX(std::string, LegIssueDate);
          FIELD_SET(*this, FIX::LegRepoCollateralSecurityType);
          FIELD_SET_EX(int, LegRepoCollateralSecurityType);
          FIELD_SET(*this, FIX::LegRepurchaseTerm);
          FIELD_SET_EX(int, LegRepurchaseTerm);
          FIELD_SET(*this, FIX::LegRepurchaseRate);
          FIELD_SET_EX(double, LegRepurchaseRate);
          FIELD_SET(*this, FIX::LegFactor);
          FIELD_SET_EX(double, LegFactor);
          FIELD_SET(*this, FIX::LegCreditRating);
          FIELD_SET_EX(std::string, LegCreditRating);
          FIELD_SET(*this, FIX::LegInstrRegistry);
          FIELD_SET_EX(std::string, LegInstrRegistry);
          FIELD_SET(*this, FIX::LegCountryOfIssue);
          FIELD_SET(*this, FIX::LegStateOrProvinceOfIssue);
          FIELD_SET_EX(std::string, LegStateOrProvinceOfIssue);
          FIELD_SET(*this, FIX::LegLocaleOfIssue);
          FIELD_SET_EX(std::string, LegLocaleOfIssue);
          FIELD_SET(*this, FIX::LegRedemptionDate);
          FIELD_SET_EX(std::string, LegRedemptionDate);
          FIELD_SET(*this, FIX::LegStrikePrice);
          FIELD_SET_EX(double, LegStrikePrice);
          FIELD_SET(*this, FIX::LegStrikeCurrency);
          FIELD_SET_EX(std::string, LegStrikeCurrency);
          FIELD_SET(*this, FIX::LegOptAttribute);
          FIELD_SET_EX(char, LegOptAttribute);
          FIELD_SET(*this, FIX::LegContractMultiplier);
          FIELD_SET_EX(double, LegContractMultiplier);
          FIELD_SET(*this, FIX::LegCouponRate);
          FIELD_SET_EX(double, LegCouponRate);
          FIELD_SET(*this, FIX::LegSecurityExchange);
          FIELD_SET(*this, FIX::LegIssuer);
          FIELD_SET_EX(std::string, LegIssuer);
          FIELD_SET(*this, FIX::EncodedLegIssuerLen);
          FIELD_SET_EX(int, EncodedLegIssuerLen);
          FIELD_SET(*this, FIX::EncodedLegIssuer);
          FIELD_SET_EX(std::string, EncodedLegIssuer);
          FIELD_SET(*this, FIX::LegSecurityDesc);
          FIELD_SET_EX(std::string, LegSecurityDesc);
          FIELD_SET(*this, FIX::EncodedLegSecurityDescLen);
          FIELD_SET_EX(int, EncodedLegSecurityDescLen);
          FIELD_SET(*this, FIX::EncodedLegSecurityDesc);
          FIELD_SET_EX(std::string, EncodedLegSecurityDesc);
          FIELD_SET(*this, FIX::LegRatioQty);
          FIELD_SET_EX(double, LegRatioQty);
          FIELD_SET(*this, FIX::LegSide);
          FIELD_SET_EX(char, LegSide);
          FIELD_SET(*this, FIX::LegCurrency);
          FIELD_SET_EX(std::string, LegCurrency);
          FIELD_SET(*this, FIX::LegPool);
          FIELD_SET_EX(std::string, LegPool);
          FIELD_SET(*this, FIX::LegDatedDate);
          FIELD_SET_EX(std::string, LegDatedDate);
          FIELD_SET(*this, FIX::LegContractSettlMonth);
          FIELD_SET(*this, FIX::LegInterestAccrualDate);
          FIELD_SET_EX(std::string, LegInterestAccrualDate);
          FIELD_SET(*this, FIX::NoLegSecurityAltID);
          FIELD_SET_EX(std::string, NoLegSecurityAltID);
          class NoLegSecurityAltID: public FIX::Group
          {
          public:
          NoLegSecurityAltID() : FIX::Group(604,605,FIX::message_order(605,606,0)) {}
            FIELD_SET(*this, FIX::LegSecurityAltID);
            FIELD_SET_EX(std::string, LegSecurityAltID);
            FIELD_SET(*this, FIX::LegSecurityAltIDSource);
            FIELD_SET_EX(std::string, LegSecurityAltIDSource);
          };
        };
      };
    };
  };

}

#endif
