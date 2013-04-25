#include "ErrorStack.h"

using namespace std;

void ErrorStack::pushError(const string &message)
{
  errorMessages.push(message);
}

void ErrorStack::pushError(std::ostream& stream)
{
  std::string message = dynamic_cast<std::ostringstream&>(stream).str(); 
  errorMessages.push(message);
}

string ErrorStack::getMessages()
{
  string messages;
  while (not errorMessages.empty()){
    messages += errorMessages.top();
    messages += "\n";
    errorMessages.pop();
  }
  
  return messages;
}

void ErrorStack::print(std::ostream &outs)
{
  outs << getMessages();
}
