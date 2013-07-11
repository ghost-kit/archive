#include "readerCache.h"

//=========================================================================================
RCache::ReaderCache::ReaderCache()
{
    this->dirty = false;

}

//=========================================================================================
RCache::ReaderCache::~ReaderCache()
{
    if(this->cache.size() > 0)
    {
        std::cout << __FUNCTION__ << " Reader Cache Destructor... "  << std::flush << std::endl;
        this->cleanCache();
    }
}

//=========================================================================================
void  RCache::ReaderCache::addTimeLevel(double time)
{

    //this simply creates the time level
    this->cache[time].initialize();

}

//=========================================================================================
void RCache::ReaderCache::addCacheElement( double time,  RCache::extents xtents,  vtkAbstractArray *array)
{
    //if the time doesn't exist, we are going to add it anyway, so lets just do it.

    std::cout << __FUNCTION__ << std::endl;

    //create the element if time not available
    if(!this->cache.contains(time))
    {
        std::cout << "Adding Time Level... " << std::endl;
        addTimeLevel(time);
    }

    //add the element
    std::cout << "Reference Count Prior to add: " << array->GetReferenceCount() << std::endl;
    cacheMap* currentMap = &this->cache[time];
    currentMap->addCacheElement(xtents, array);
    std::cout << "Reference Count After Add: " << array->GetReferenceCount() << std::endl;
    std::cout << "Map Size: " << currentMap->getNumberElments() << std::endl;

}

//=========================================================================================
RCache::cacheElement *RCache::ReaderCache::getExtentsFromCache(double time, RCache::extents xtents)
{

    std::map<double,cacheMap>::iterator timeElement;


    //this must be NULL to start with or algorithm won't work
    RCache::cacheElement *currentArray = NULL;


    //this will search the cache and return the element that is needed
    if(this->cache.contains(time))
    {
        //the time segment is actually in the cache
        cacheMap* currentMap = &this->cache[time];

    }


    //    std::cout << "ReturnValue: " << ((currentArray != NULL) ? "GOOD" : "NULL" ) << std::endl;
    //return the array if found... otherwise, return NULL
    return currentArray;

}

//=========================================================================================
void RCache::ReaderCache::cleanCache()
{

    //kill the cache map
    if(this->cache.size() > 0)
    {
        std::cout << __FUNCTION__ << " Killing Entire Map... " <<  std::flush << std::endl;
        this->cache.clear();
    }
}

//=========================================================================================
RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents xtents,  RCache::cacheElement *cacheArray)
{
    //    if(cacheArray->data->GetDataType() == VTK_FLOAT)
    //    {
    //        //cast to float array and call float function
    //        vtkFloatArray* current = vtkFloatArray::SafeDownCast(cacheArray->data);
    //        return RCache::ReaderCache::extractFromArray(xtents, cacheArray->xtents, current);
    //    }
    //    else if(cacheArray->data->GetDataType() == VTK_DOUBLE)
    //    {
    //        //cast to double array and call function
    //        vtkDoubleArray* current = vtkDoubleArray::SafeDownCast(cacheArray->data);
    //        return RCache::ReaderCache::extractFromArray(xtents, cacheArray->xtents, current);
    //    }
    //    else if(cacheArray->data->GetDataType() == VTK_INT)
    //    {
    //        //cast to int array and call function
    //        vtkIntArray* current = vtkIntArray::SafeDownCast(cacheArray->data);
    //        return RCache::ReaderCache::extractFromArray(xtents, cacheArray->xtents, current);
    //    }
    //    else
    //    {
    return NULL;
    //    }
}

RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents newXtetnts, RCache::extents oldXtents,  vtkDoubleArray *cacheArray)
{
    //extract the proper extents and put in new object
    //    RCache::cacheElement *newElement = new cacheElement;
    //    newElement->data = vtkDoubleArray::New();

    abort();

    return NULL;

}

RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents newXtetnts, RCache::extents oldXtents, vtkFloatArray *cacheArray)
{
    //extract the proper extents and put in new object
    //    RCache::cacheElement *newElement = new cacheElement;
    //    newElement->data = vtkFloatArray::New();

    abort();

    return NULL;
}

RCache::cacheElement *RCache::ReaderCache::extractFromArray(RCache::extents newXtetnts, RCache::extents oldXtents,  vtkIntArray *cacheArray)
{
    //extract the proper extents and put in new object
    //    RCache::cacheElement *newElement = new cacheElement;
    //    newElement->data = vtkIntArray::New();

    abort();

    return NULL;
}

//=========================================================================================
bool RCache::ReaderCache::isInCache(double time, RCache::extents xtents)
{
    std::map<double,cacheMap>::iterator timeElement;
    cacheMap* currentMap=NULL;

    if(this->cache.contains(time))
    {
        currentMap = &this->cache[time];
        if(currentMap->hasCacheElement(xtents))
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
void RCache::ReaderCache::promoteElement(double time, RCache::extents Xtents)
{
    QStack<RCache::cacheElement*>*  currentStack;
    RCache::cacheElement* currentElement;

    QStack<RCache::cacheElement*>::Iterator itr;

    int count = 0;

    //get the current stack for the current time
    //  then promote the entry
    //    if(this->cacheStack.contains(time))
    //    {
    //        currentStack = this->cacheStack[time];

    //        for(itr=currentStack->begin(); itr != currentStack->end(); ++itr)
    //        {
    //            currentElement = dynamic_cast<RCache::cacheElement*>(*itr);
    //            if(currentElement->xtents == Xtents)
    //            {
    //                //remove and then insert at the top
    //                if(itr != currentStack->begin())
    //                {
    //                    //remove elemnt
    //                    currentStack->remove(count);
    //                    //insert at the top
    //                    currentStack->push(currentElement);
    //                }
    //                //don't go beyond the point we are looking for
    //                break;
    //            }

    //            //for some reason, we don't have a remove() method for iterators
    //          count++;
    //        }
    //    }
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
    this->initd = false;
}

RCache::cacheMap::~cacheMap()
{
    //need to delete all of the vtkAbstractArrays
    std::cout << __FUNCTION__ << std::endl;
    this->clearCacheMap();
}

//=========================================================================================
void RCache::cacheMap::addCacheElement(RCache::extents xtents, vtkAbstractArray *data)
{  
    //check to see if element is already in the stack
    QList<RCache::cacheElement>::Iterator iter;

    //temp pointer for promoting item on stack
    RCache::cacheElement* tempEl = NULL;

    //index
    int index = 0;

    for(index = 0; index < this->cacheStack.size(); index++)
    {
        if(this->cacheStack[index].xtents == xtents)
        {
            std::cout << "Found Existing Element ... promoting" << std::endl;

            //delete from current position and place on top of stack
            //this->cacheStack.move(index, 0);

            return;
        }
    }


    std::cout << "ADDING NEW ELEMENT" << std::endl;
    RCache::cacheElement newElement;
    newElement.xtents = xtents;
    newElement.data = data;

    //add to the stack
    this->cacheStack.prepend(newElement);

}

//=========================================================================================
RCache::cacheElement *RCache::cacheMap::getCacheElement(RCache::extents xtents)
{

    QList<RCache::cacheElement>::Iterator iter;
    RCache::cacheElement* tempEl = NULL;

    for(iter = this->cacheStack.begin(); iter != this->cacheStack.end(); ++iter)
    {
        tempEl = &*iter;
        if(tempEl->xtents == xtents)
        {
            std::cout << "Element Found ... Returning" << std::endl;
            return tempEl;
        }
    }
    std::cout << "Element NOT found" << std::endl;

    return NULL;
}

//=========================================================================================
bool RCache::cacheMap::hasCacheElement(RCache::extents xtents)
{
    QList<RCache::cacheElement>::Iterator iter;
    RCache::cacheElement* tempEl = NULL;

    for(iter = this->cacheStack.begin(); iter != this->cacheStack.end(); ++iter)
    {
        tempEl = &*iter;
        if(tempEl->xtents == xtents)
        {
            //the element exists
            return true;
        }
    }

    //the element does not exist
    return false;
}

//=========================================================================================
RCache::cacheElement *RCache::cacheMap::getCacheElementContains(RCache::extents xtents)
{

    abort();

    return NULL;
}

//=========================================================================================
void RCache::cacheMap::removeCacheElement(RCache::extents xtents)
{

    QList<RCache::cacheElement>::Iterator iter;
    RCache::cacheElement* tempEl = NULL;

    for(iter = this->cacheStack.begin(); iter != this->cacheStack.end(); ++iter)
    {
        tempEl = &*iter;
        if(tempEl->xtents == xtents)
        {
            //found it... so remove it...
            this->cacheStack.erase(iter);

            //our loop is no longer valid, so kill it
            break;
        }
    }


}

//=========================================================================================
void RCache::cacheMap::clearCacheMap()
{
    // since we are using vtkSmartPointer and we don't have to do much for
    // cleanup, except the following:
    std::cout <<  __FUNCTION__ << " Clearing Cache... " << std::flush << std::endl;
    this->cacheStack.clear();

}

int RCache::cacheMap::getNumberElments()
{
    return this->cacheStack.size();
}




