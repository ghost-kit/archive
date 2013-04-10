#ifndef __DEPRECATEDHDF4_H__
#define __DEPRECATEDHDF4_H__

#include "DeprecatedIo.h"

#ifndef MAX_VAR_DIMS
#define MAX_VAR_DIMS H4_MAX_VAR_DIMS
#endif
#ifndef MAX_NC_NAME
#define MAX_NC_NAME H4_MAX_NC_NAME
#endif

#if defined(APLUSPLUS) || defined(PPLUSPLUS)
#include <A++.h>
#endif
#include <mfhdf.h>

#include <cassert>
#include <sstream>
#include <iostream>

#define DEPRECATEDERRORCHECK(STATUS) errorCheck(STATUS, __FILE__, __LINE__, __FUNCTION__, #STATUS)

class DeprecatedHdf4 : public DeprecatedIo
{
public:
  DeprecatedHdf4();
  ~DeprecatedHdf4();
  void open(const std::string &filename, const int &ioType);

  /// Routines for Reading
  //@{
  void readMetaData(std::map<std::string, double> &metaDoubles, 
		    std::map<std::string, float> &metaFloats,
		    std::map<std::string, int> &metaInts,
		    std::map<std::string, std::string> &metaStrings);
  void readString(const std::string&name, std::string &data);
  bool hasVariable(const std::string &variable);
#if defined(APLUSPLUS) || defined(PPLUSPLUS)
  void readVariable(const std::string &variable, floatArray &data);
#endif
  void readVariable(const std::string &variable, float *&data, int &rank, int *&dims) ;
  //@}

  /// Routines for Writing
  //@{
  void writeMetaData(const std::map<std::string, double> &metaDoubles,
		     const std::map<std::string, float> &metaFloats,
		     const std::map<std::string, int> &metaInts,
		     const std::map<std::string, std::string> &metaStrings);
  void writeMetaData(const std::map<std::string, double> &metaDoubles);
  void writeMetaData(const std::map<std::string, float> &metaFloats);
  void writeMetaData(const std::map<std::string, int> &metaInts);
  void writeMetaData(const std::map<std::string, std::string> &metaStrings);
  void writeString(const std::string &name, const std::string &data);
#if defined(APLUSPLUS) || defined(PPLUSPLUS)
  void writeVariable(const std::string &variable, const floatArray &data, const std::string &units);
#endif
  void writeVariable(const std::string &variable, const float *data, const int &rank, const int *dims, const std::string &units);
  //@}

  void close();

private:
  void errorCheck(const int &status, const char *file, const int &line, const char *func, const char *command);

  int32_t fileId;
};

#endif
