#include "readerCache.h"

//=========================================================================================
RCache::ReaderCache::ReaderCache()
{
    this->dirty = false;

}

//=========================================================================================
RCache::ReaderCache::~ReaderCache()
{
    this->cleanCache();

}

//=========================================================================================
void RCache::ReaderCache::addCacheElement(double time, RCache::extents xtents, vtkAbstractArray *array)
{

    std::cout << "Adding Element to time " << time << std::endl;

    std::map<double,cacheMap*>::iterator timeElement;
    cacheMap* currentMap=NULL;

    if((timeElement=this->cache.find(time)) != this->cache.end())
    {
        currentMap = timeElement->second;
        if(currentMap->getCacheElementContains(xtents) == NULL)
        {
            //the cache element is not in the map, so lets add it
            currentMap->addCacheElement(xtents, array);

            //keep track of it so we can kill it when needed
            std::cout << "reference count before caching: " << array->GetReferenceCount() << std::endl;
            this->cacheVector[xtents] = array;
            std::cout << "reference count after caching:  " << array->GetReferenceCount() << std::endl;

            std::cout << "Added Element to Cache for time " << time  << std::endl;

        }
        else
        {
            //we are not adding this array, so mark it for deletion
            array->Delete();
        }
    }
    else
    {
        std::cout << "Adding new cache table for time " << time << std::endl;
        //add the time to the cache table
        this->cache[time] = new cacheMap;
        currentMap = this->cache[time];

        std::cout << "Adding Element at time " << time << std::endl;
        //add the new element to the new time
        currentMap->addCacheElement(xtents, array);

        //keep track of it so we can kill it when needed
        std::cout << "reference count before caching: " << array->GetReferenceCount() << std::endl;
        this->cacheVector[xtents] = array;
        std::cout << "reference count after caching:  " << array->GetReferenceCount() << std::endl;

        std::cout << "Added Element to Cache for time " << time  << std::endl;
    }

}

//=========================================================================================
RCache::cacheElement *RCache::ReaderCache::getExtentsFromCache(double time, RCache::extents xtents)
{

    std::cout << __LINE__ << " " << __FUNCTION__ << std::endl << std::flush;

    std::map<double,cacheMap*>::iterator timeElement;
    cacheMap* currentMap=NULL;


    //this must be NULL to start with or algorithm won't work
    RCache::cacheElement *currentArray = NULL;


    //this will search the cache and return the element that is needed
    if((timeElement=this->cache.find(time)) != this->cache.end())
    {
        //the time segment is actually in the cache
        currentMap = timeElement->second;


        //check to see if the exact extents exist
        if((currentArray=currentMap->getCacheElement(xtents)) == NULL)
        {

            //if exact extents don't exist, check if subextent exist
            if((currentArray=currentMap->getCacheElementContains(xtents)) != NULL)
            {

                //extract the correct extents from the superset
                currentArray = ReaderCache::extractFromArray(xtents, currentArray);
            }
        }

    }


    std::cout << "ReturnValue: " << ((currentArray != NULL) ? "GOOD" : "NULL" ) << std::endl;
    //return the array if found... otherwise, return NULL
    return currentArray;

}

//=========================================================================================
void RCache::ReaderCache::cleanCache()
{
    std::map<RCache::extents, vtkAbstractArray*>::iterator iter;

    //kill the cache map
    this->cache.clear();

    //mark arrays for deletion
    for(iter = this->cacheVector.begin(); iter != this->cacheVector.end(); ++iter)
    {
        std::cout << "Reference Count: " <<  iter->second->GetReferenceCount() << std::endl;
        iter->second->Delete();

    }
    this->cacheVector.clear();
    this->dirty = true;


}

//=========================================================================================
RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents xtents,  RCache::cacheElement *cacheArray)
{
    if(cacheArray->data->GetDataType() == VTK_FLOAT)
    {
        //cast to float array and call float function
        vtkFloatArray* current = vtkFloatArray::SafeDownCast(cacheArray->data);
        return RCache::ReaderCache::extractFromArray(xtents, cacheArray->xtents, current);
    }
    else if(cacheArray->data->GetDataType() == VTK_DOUBLE)
    {
        //cast to double array and call function
        vtkDoubleArray* current = vtkDoubleArray::SafeDownCast(cacheArray->data);
        return RCache::ReaderCache::extractFromArray(xtents, cacheArray->xtents, current);
    }
    else if(cacheArray->data->GetDataType() == VTK_INT)
    {
        //cast to int array and call function
        vtkIntArray* current = vtkIntArray::SafeDownCast(cacheArray->data);
        return RCache::ReaderCache::extractFromArray(xtents, cacheArray->xtents, current);
    }
    else
    {
        return NULL;
    }
}

RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents newXtetnts, RCache::extents oldXtents,  vtkDoubleArray *cacheArray)
{
    //extract the proper extents and put in new object
    RCache::cacheElement *newElement = new cacheElement;
    newElement->data = vtkDoubleArray::New();

    return NULL;

}

RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents newXtetnts, RCache::extents oldXtents, vtkFloatArray *cacheArray)
{
    //extract the proper extents and put in new object
    RCache::cacheElement *newElement = new cacheElement;
    newElement->data = vtkFloatArray::New();

    return NULL;
}

RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents newXtetnts, RCache::extents oldXtents,  vtkIntArray *cacheArray)
{
    //extract the proper extents and put in new object
    RCache::cacheElement *newElement = new cacheElement;
    newElement->data = vtkIntArray::New();

    return NULL;
}

//=========================================================================================
bool RCache::ReaderCache::isInCache(double time, RCache::extents xtents)
{
    std::map<double,cacheMap*>::iterator timeElement;
    cacheMap* currentMap=NULL;

    if((timeElement=this->cache.find(time)) != this->cache.end())
    {
        currentMap = timeElement->second;

        if(currentMap->getCacheElementContains(xtents) != NULL)
        {
            return true;
        }
    }

    return false;

}

//=========================================================================================
void RCache::ReaderCache::pruneTimeFromEndTo(double time)
{
}

//=========================================================================================
void RCache::ReaderCache::pruneExtentsFromTime(RCache::extents xtents)
{
}

//=========================================================================================
RCache::extents::extents(int &x1, int &x2, int &x3, int &x4, int &x5, int &x6, bool persists)
{
    this->xtents[0] = x1;
    this->xtents[1] = x2;
    this->xtents[2] = x3;
    this->xtents[3] = x4;
    this->xtents[4] = x5;
    this->xtents[5] = x6;

    this->persists = persists;
}

RCache::extents::extents(const int *xtents, bool persists)
{
    this->xtents[0] = xtents[0];
    this->xtents[1] = xtents[1];
    this->xtents[2] = xtents[2];
    this->xtents[3] = xtents[3];
    this->xtents[4] = xtents[4];
    this->xtents[5] = xtents[5];

    this->persists = persists;
}

RCache::extents::extents()
{
    this->xtents[0] = 0;
    this->xtents[1] = 0;
    this->xtents[2] = 0;
    this->xtents[3] = 0;
    this->xtents[4] = 0;
    this->xtents[5] = 0;
    this->persists = false;
}

RCache::extents::~extents()
{
    //nothing to do here at this time...
}

//=========================================================================================
//defined as extents are exactly equal
bool RCache::extents::operator==(const extents &rhs) const
{
    if(this->xtents[0] == rhs.xtents[0] &&
            this->xtents[1] == rhs.xtents[1] &&
            this->xtents[2] == rhs.xtents[2] &&
            this->xtents[3] == rhs.xtents[3] &&
            this->xtents[4] == rhs.xtents[4] &&
            this->xtents[5] == rhs.xtents[5])
    {
        return true;
    }
    else
    {
        return false;
    }
}

//=========================================================================================
bool RCache::extents::operator !=(const RCache::extents &rhs) const
{
    return !(*this == rhs);
}

//=========================================================================================
//defined as this is completely within (and not equal to) the rhs
bool RCache::extents::operator<(const extents &rhs) const
{
    //this will give us <= for the extents.
    if(((this->xtents[1]-this->xtents[0]) <= (rhs.xtents[1]-rhs.xtents[0])) &&
            ((this->xtents[3]-this->xtents[2]) <= (rhs.xtents[3]-rhs.xtents[2])) &&
            ((this->xtents[5]-this->xtents[4]) <= (rhs.xtents[5]-rhs.xtents[4])))
    {
        //this will give us strictly less than
        if(!(((this->xtents[0]-rhs.xtents[0]) == (this->xtents[1]-rhs.xtents[1])) &&
             ((this->xtents[2]-rhs.xtents[2]) == (this->xtents[3]-rhs.xtents[3])) &&
             ((this->xtents[4]-rhs.xtents[4]) == (this->xtents[5]-rhs.xtents[5]))))
        {
            return true;
        }
    }

    return false;
}

//=========================================================================================
bool RCache::extents::operator <=(const RCache::extents &rhs) const
{
    return ((*this < rhs) || (*this == rhs));
}

//=========================================================================================
bool RCache::extents::operator >(const RCache::extents &rhs) const
{
    return (rhs < *this);
}

//=========================================================================================
bool RCache::extents::operator >=(const RCache::extents &rhs) const
{
    return ((*this > rhs) || (*this == rhs));
}

//=========================================================================================
int RCache::extents::getExtent(int x)
{
    if(x >= 0 && x < 6)
        return this->xtents[x];
    else /*error condition*/
        return -1;
}

//=========================================================================================
int *RCache::extents::getExtents()
{
    int *returnval = new int[6];

    returnval[0] = this->xtents[0];
    returnval[1] = this->xtents[1];
    returnval[2] = this->xtents[2];
    returnval[3] = this->xtents[3];
    returnval[4] = this->xtents[4];
    returnval[5] = this->xtents[5];

    //user MUST free this memory to avoid leaks
    return returnval;
}

//=========================================================================================
void RCache::extents::setPersistance(bool persists)
{
    this->persists = persists;
}

//=========================================================================================
RCache::cacheMap::cacheMap()
{
    //initalize cache size counter
    this->cacheSize = 0;
}

RCache::cacheMap::~cacheMap()
{
    //need to delete all of the vtkAbstractArrays
    this->cleanCache();
}

//=========================================================================================
void RCache::cacheMap::addCacheElement(RCache::extents xtents, vtkAbstractArray *data)
{
    cacheElement* newElement = new cacheElement;
    newElement->data = data;
    newElement->xtents = xtents;

    //we only wnat to add the extent if it is not already in the cache
    if(this->map.find(xtents) == map.end())
    {
        this->map[xtents] = newElement;
    }
}

//=========================================================================================
RCache::cacheElement *RCache::cacheMap::getCacheElement(RCache::extents xtents)
{
    //returns the exact extent if it exists
    if(this->map.find(xtents) != map.end()) return this->map[xtents];
    else return NULL;
}

//=========================================================================================
RCache::cacheElement *RCache::cacheMap::getCacheElementContains(RCache::extents xtents)
{
    //compare the key for each element with requested
    //and determine if it contains the requests extents

    std::map<RCache::extents, RCache::cacheElement*>::iterator iter = this->map.begin();
    std::map<RCache::extents, RCache::cacheElement*>::key_compare isContained = this->map.key_comp();

    //iterate through the map until we find an object that CONTAINS the extents we are looking for
    while(iter != this->map.end())
    {
        //checks to see if the requested extents are contained in the current entry
        if(isContained(xtents, iter->first) || xtents == iter->first)
        {
            //return the vtkAbstractArray pointer for this mapped element
            return iter->second;
        }
    }

    return NULL;
}

//=========================================================================================
void RCache::cacheMap::removeCacheElement(RCache::extents xtents)
{

    //THIS FUNCTION SHOULD BE MOVED TO THE ReaderCache LEVEL

    //finds the key if it exists, and erases the entry
    std::map<RCache::extents, RCache::cacheElement*>::iterator iter;

    if((iter=this->map.find(xtents)) != this->map.end())
    {
        //mark element for garbage collection
        iter->second->data->Delete();
        //remove the entry
        this->map.erase(iter);
    }
}

//=========================================================================================
void RCache::cacheMap::cleanCache()
{
    //this method will remove all items from the cache and mark
    //all cache elements for garbage collection
    std::map<RCache::extents, RCache::cacheElement*>::iterator iter;

    for(iter=this->map.begin(); iter != this->map.end(); ++iter)
    {
        this->map.erase(iter);
    }
}




