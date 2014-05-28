#ifndef AISLINN_MPI_ACTION
#define AISLINN_MPI_ACTION

#include <vector>
#include <string>

namespace aislinn {

enum ActionType {
  ACTION_SEND,
  ACTION_ISEND,
  ACTION_RECV,
  ACTION_IRECV,
  ACTION_WAIT,
  ACTION_WAITALL,
  ACTION_TEST,
  ACTION_TESTALL
};

class XML;

class Action
{
  ActionType Type;
  int Rank;

  union {
    struct {
      int Target;
      int Tag;
      int RequestId;
    } Ptp; // PointToPoint communication
    struct {
      int RequestId;
    } WaitTest;
  };

  std::vector<int> RequestIds;

  public:
  void write(XML &Report) const;

  int getRank() const {
    return Rank;
  }

  static Action makePtp(
      ActionType Type, int Rank, int Target, int Tag, int RequestId);
  static Action makeWaitTest(ActionType Type, int Rank, int ReqestId);
  static Action makeWaitTestAll(
      ActionType Type, int Rank, const std::vector<int> &RequestIds);
  static std::string getTypeAsString(ActionType Type);
};

}

#endif
