#ifndef FIX44_LISTEXECUTE_H
#define FIX44_LISTEXECUTE_H

#include "Message.h"

namespace FIX44
{

  class ListExecute : public Message
  {
  public:
    ListExecute() : Message(MsgType()) {}
    ListExecute(const FIX::Message& m) : Message(m) {}
    ListExecute(const Message& m) : Message(m) {}
    ListExecute(const ListExecute& m) : Message(m) {}
    static FIX::MsgType MsgType() { return FIX::MsgType("L"); }

    ListExecute(
      const FIX::ListID& aListID,
      const FIX::TransactTime& aTransactTime )
    : Message(MsgType())
    {
      set(aListID);
      set(aTransactTime);
    }

    FIELD_SET(*this, FIX::ListID);
    FIELD_SET_EX(std::string, ListID);
    FIELD_SET(*this, FIX::ClientBidID);
    FIELD_SET_EX(std::string, ClientBidID);
    FIELD_SET(*this, FIX::BidID);
    FIELD_SET_EX(std::string, BidID);
    FIELD_SET(*this, FIX::TransactTime);
    FIELD_SET_EX(FIX::UtcTimeStamp, TransactTime);
    FIELD_SET(*this, FIX::Text);
    FIELD_SET_EX(std::string, Text);
    FIELD_SET(*this, FIX::EncodedTextLen);
    FIELD_SET_EX(int, EncodedTextLen);
    FIELD_SET(*this, FIX::EncodedText);
    FIELD_SET_EX(std::string, EncodedText);
  };

}

#endif
