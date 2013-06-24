#ifndef __ERROR_QUEUE_H__

#include <string>
#include <queue>
#include <iostream>
#include <sstream>

class ErrorQueue{
public:
  void pushError(const std::string &message);
  void pushError(std::stringstream& stream);

  const std::string getMessages() const;
  void print(std::ostream &outs) const;

  void clear();

private:

  std::queue<std::string> errorMessages;

};

#endif //__ERROR_QUEUE_H__
