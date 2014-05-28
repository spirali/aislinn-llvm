
#include "Action.h"
#include "../Utils/XML.h"

using namespace aislinn;

std::string Action::getTypeAsString(ActionType Type)
{
  switch(Type) {
    case ACTION_SEND:
      return "Send";
    case ACTION_ISEND:
      return "Isend";
    case ACTION_RECV:
      return "Recv";
    case ACTION_IRECV:
      return "Irecv";
    case ACTION_WAIT:
      return "Wait";
    case ACTION_WAITALL:
      return "Waitall";
    case ACTION_TEST:
      return "Test";
    case ACTION_TESTALL:
      return "Testall";
    default:
      return "Invalid";
  }
}

void Action::write(XML &Report) const
{
  Report.child("action");
  Report.set("name", getTypeAsString(Type));
  Report.set("rank", Rank);
  // TODO: Write other attributes
  Report.back();
}

Action Action::makePtp(
    ActionType Type, int Rank, int Target, int Tag, int RequestId)
{
  Action A;
  A.Type = Type;
  A.Rank = Rank;
  A.Ptp.Target = Target;
  A.Ptp.Tag = Tag;
  A.Ptp.RequestId = RequestId;
  return A;
}

Action Action::makeWaitTest(ActionType Type, int Rank, int RequestId)
{
  Action A;
  A.Rank = Rank;
  A.Type = Type;
  A.WaitTest.RequestId = RequestId;
  return A;
}

Action Action::makeWaitTestAll(
    ActionType Type, int Rank, const std::vector<int> &RequestIds)
{
  Action A;
  A.Rank = Rank;
  A.Type = Type;
  A.RequestIds = RequestIds;
  return A;
}
