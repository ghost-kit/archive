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

const string ErrorQueue::getMessages() const
{
  queue<string> messagesToGet(errorMessages); 

  string messages;
  while (not messagesToGet.empty()){
    messages += messagesToGet.front();
    messages += "\n";
    messagesToGet.pop();
  }
  
  return messages;
}

void ErrorQueue::print(std::ostream &outs) const
{
  outs << getMessages();
}


void ErrorQueue::clear()
{
  while (not errorMessages.empty()){
    errorMessages.pop();
  }
}
