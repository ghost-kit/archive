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
#include "vtkSetGet.h"


class vtkStringArray;
class vtkDoubleArray;
class vtkFloatArray;
class vtkDoubleArray;
class vtkAbstractArray;
class extents;
class cacheMap;

namespace RCache
{

//=========================================================================================
//this is a helper class for keeping track of extents
class extents
{
public:

    extents(int x1, int x2, int x3, int x4, int x5, int x6, bool persists=false);
    extents(const int xtents[6], bool persists=false);
    extents();

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

    //set the persistance flag
    void setPersistance(bool persists);

 protected:
    //the actual extents being kept track of
    int xtents[6];
    bool persists;
};

//=========================================================================================
class cacheElement
{
public:
    cacheElement()
    {
        //nothing to do
    }

    ~cacheElement()
    {
        this->data->Delete();
    }

    extents xtents;
    vtkAbstractArray* data;
};


//=========================================================================================
class cacheMap
{
public:

    cacheMap();
    ~cacheMap();

    //adds a cache element to the maping if it doesn't already exist
    void addCacheElement(extents xtents, vtkAbstractArray* data);

    //returns the appropriate data segment from the cache
    cacheElement* getCacheElement(extents xtents);

    //returns a dataset that CONTAINS the requested extent
    //user must parse to get required elements
    cacheElement* getCacheElementContains(extents xtents);

    //removes a specific element from the cache map
    void removeCacheElement(extents xtents);

    //removes all elements from the cache map
    void cleanCache();

    //return the amount of memory being used by cache
    double getCacheUsage() { return this->cacheSize; }

protected:
    //this is the map for caching objects
    std::map<extents, cacheElement*> map;

    //running total of memory usage
    double cacheSize;
};

//=========================================================================================
//this is the actuall reader cache object
class ReaderCache
{
public:
    ReaderCache();
    ~ReaderCache();

    //this will either cache the element or not
    //depending on wether the extents are in the cache already or not
    void addCacheElement(double time, extents xtents, vtkAbstractArray* array);

    //this will get the requested extents from the Cache, or return NULL if
    // the extents are NOT in the cache
    cacheElement *getExtentsFromCache(double time, extents xtents);

    //build a new data array of proper type and
    // extract requested elements from cacheElement in map
    static RCache::cacheElement* extractFromArray(extents xtetnts,  RCache::cacheElement* cacheArray);
    static RCache::cacheElement* extractFromArray(extents newXtetnts, extents oldXtents,  vtkDoubleArray* cacheArray);
    static RCache::cacheElement* extractFromArray(extents newXtetnts, extents oldXtents,  vtkFloatArray* cacheArray);
    static RCache::cacheElement* extractFromArray(extents newXtetnts, extents oldXtents,  vtkIntArray* cacheArray);

protected:

    //will return true if extents are already in the cache
    bool isInCache(double time, extents xtents);

    //will remove time steps from the cache
    void pruneTimeFromEndTo(double time);

    //will remove extents (if they are discrete) from time series
    void pruneExtentsFromTime(extents xtents);

private:
    //this maps time to a specific cachemap
    std::map<double, cacheMap*> cache;

};

//end namespace READER_CACHE
}


#endif // READERCACHE_H
