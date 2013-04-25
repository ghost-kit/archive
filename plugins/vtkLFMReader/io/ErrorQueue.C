#include "ErrorQueue.h"

using namespace std;

void ErrorQueue::pushError(const string &message)
{
  errorMessages.push(message);
}

void ErrorQueue::pushError(stringstream& stream)
{
  errorMessages.push(stream.str());
}

string ErrorQueue::getMessages()
{
  string messages;
  while (not errorMessages.empty()){
    messages += errorMessages.front();
    messages += "\n";
    errorMessages.pop();
  }
  
  return messages;
}

void ErrorQueue::print(std::ostream &outs)
{
  outs << getMessages();
}
