#include "readerCache.h"

//=========================================================================================
RCache::ReaderCache::ReaderCache()
{
}

//=========================================================================================
RCache::extents::extents(int x1, int x2, int x3, int x4, int x5, int x6)
{
    this->xtents[0] = x1;
    this->xtents[1] = x2;
    this->xtents[2] = x3;
    this->xtents[3] = x4;
    this->xtents[4] = x5;
    this->xtents[5] = x6;
}

RCache::extents::extents(const int xtents[])
{
    this->xtents[0] = xtents[0];
    this->xtents[1] = xtents[1];
    this->xtents[2] = xtents[2];
    this->xtents[3] = xtents[3];
    this->xtents[4] = xtents[4];
    this->xtents[5] = xtents[5];
}

RCache::extents::~extents()
{
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
    if(x > 0 && x < 6)
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
    //we only wnat to add the extent if it is not already in the cache
    if(this->map.find(xtents) == map.end())
    {
        this->map[xtents] = data;
    }
}

//=========================================================================================
vtkAbstractArray *RCache::cacheMap::getCacheElement(RCache::extents xtents)
{
    //returns the exact extent if it exists
    if(this->map.find(xtents) != map.end()) return this->map[xtents];
    else return NULL;
}

//=========================================================================================
vtkAbstractArray *RCache::cacheMap::getCacheElementContains(RCache::extents xtents)
{
    //compare the key for each element with requested
    //and determine if it contains the requests extents

    std::map<RCache::extents, vtkAbstractArray*>::iterator iter = this->map.begin();
    std::map<RCache::extents, vtkAbstractArray*>::key_compare isContained = this->map.key_comp();

    //iterate through the map until we find an object that CONTAINS the extents we are looking for
    while(iter != this->map.end())
    {
        //checks to see if the requested extents are contained in the current entry
        if(isContained(xtents, iter->first))
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

    //finds the key if it exists, and erases the entry
    std::map<RCache::extents, vtkAbstractArray*>::iterator iter;

    if((iter=this->map.find(xtents)) != this->map.end())
    {
        //mark element for garbage collection
        iter->second->Delete();
        //remove the entry
        this->map.erase(iter);
    }
}

//=========================================================================================
void RCache::cacheMap::cleanCache()
{
    //this method will remove all items from the cache and mark
    //all cache elements for garbage collection
    std::map<RCache::extents, vtkAbstractArray*>::iterator iter;

    for(iter=this->map.begin(); iter != this->map.end(); ++iter)
    {
        //mark for garbage collection
        iter->second->Delete();
        //delete the element
        this->map.erase(iter);
    }
}



