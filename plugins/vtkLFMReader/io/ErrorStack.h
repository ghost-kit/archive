#ifndef __ERROR_STACK_H__

#include <string>
#include <stack>
#include <iostream>
#include <sstream>

class ErrorStack{
public:
  void pushError(const std::string &message);
  void pushError(std::ostream& stream);

  std::string getMessages();
  void print(std::ostream &outs);

private:

  std::stack<std::string> errorMessages;

};

#endif //__ERROR_STACK_H__
