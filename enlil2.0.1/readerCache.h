#ifndef READERCACHE_H
#define READERCACHE_H

#include <map>
#include <vector>
#include <QString>
#include "vtkAbstractArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkStringArray.h"


class vtkStringArray;
class vtkDoubleArray;
class vtkFloatArray;
class vtkDoubleArray;

namespace RCache
{


//this is the actuall reader cache object
class ReaderCache
{
public:
    ReaderCache();
};


//this is a helper class for keeping track of extents
class extents
{
public:

    extents(int x1, int x2, int x3, int x4, int x5, int x6);
    extents(const int xtents[6]);
    ~extents();

    //operators
     bool operator==(const extents &rhs) const;
     bool operator!=(const extents &rhs) const;
     bool operator< (const extents &rhs) const;
     bool operator<=(const extents &rhs) const;
     bool operator> (const extents &rhs) const;
     bool operator>=(const extents &rhs) const;

    //get a specified extent
    int getExtent(int x);

    //gets all extents as an int array of 6 elements (int x[6])
    int* getExtents();

 protected:
    //the actual extents being kept track of
    int xtents[6];
};

class cacheMap
{
public:

    cacheMap();
    ~cacheMap();

    //adds a cache element to the maping if it doesn't already exist
    void addCacheElement(extents xtents, vtkAbstractArray* data);

    //returns the appropriate data segment from the cache
    vtkAbstractArray* getCacheElement(extents xtents);

    //returns a dataset that CONTAINS the requested extent
    //user must parse to get required elements
    vtkAbstractArray* getCacheElementContains(extents xtents);

    //removes a specific element from the cache map
    void removeCacheElement(extents xtents);

    //removes all elements from the cache map
    void cleanCache();

    //return the amount of memory being used by cache
    double getCacheUsage() { return this->cacheSize; }

protected:
    //this is the map for caching objects
    std::map<extents, vtkAbstractArray*> map;

    //running total of memory usage
    double cacheSize;
};

//end namespace READER_CACHE
}


#endif // READERCACHE_H
