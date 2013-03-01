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

    std::map<double,cacheMap*>::iterator timeElement;
    cacheMap* currentMap=NULL;

    if((timeElement=this->cache.find(time)) != this->cache.end())
    {
        std::cout << "Found Time" << std::endl;
        currentMap = timeElement->second;

        //disabled until we figure out how to make it work
        //if(currentMap->getCacheElementContains(xtents) == NULL)
        if(currentMap->getCacheElement(xtents) == NULL)
        {
            //the cache element is not in the map, so lets add it
           RCache::cacheElement *currentElement = currentMap->addCacheElement(xtents, array);

            //keep track of it so we can kill it when needed

           std::cout << "Element: " << ((currentElement == NULL) ? "GOOD" : "NULL") << std::endl;

           if(currentElement != NULL)
           {
               std::cerr << "Adding Element to the Stack" << std::endl;

               this->cacheStack[time]->push(currentElement);
//               std::cout << "Added Elemnt to the Cache Stack" << std::endl;


           }
           else
           {
               //lets move the element with our xtents to the top of the stack
               this->promoteElement(time, xtents);

               std::cout << "Promoting Element to top of stack..." << std::endl;

           }
//            std::cout << "Added Element to Cache for time " << time  << std::endl;

        }
        else
        {
            //we are not adding this array, so mark it for deletion
//            array->Delete();

//            std::cout << "ARRAY ALREADY IN CACHE" << std::endl;
        }
    }
    else
    {

        std::cout << "Creating new cache map" << std::endl;
        //add the time to the cache table
        this->cache[time] = new cacheMap;
        currentMap = this->cache[time];

        std::cout << "Creating new cacheStack" << std::endl;
        //set up the cacheStack element
        this->cacheStack[time] = new QStack<RCache::cacheElement*>;

        std::cout << "adding element to cache Map" << std::endl;
        //add the new element to the new time
        RCache::cacheElement *currentElement = currentMap->addCacheElement(xtents, array);

        std::cout << "Adding element to cacheStack" << std::endl;

        if(currentElement != NULL)
        {
            std::cerr << "Adding Element to the Stack" << std::endl;

            this->cacheStack[time]->push(currentElement);
//               std::cout << "Added Elemnt to the Cache Stack" << std::endl;


        }
        else
        {
            //lets move the element with our xtents to the top of the stack
            this->promoteElement(time, xtents);

            std::cout << "Promoting Element to top of stack..." << std::endl;

        }
//        std::cout << "Added Element to Cache for time " << time  << std::endl;
    }

}

//=========================================================================================
RCache::cacheElement *RCache::ReaderCache::getExtentsFromCache(double time, RCache::extents xtents)
{

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
//        if((currentArray=currentMap->getCacheElement(xtents)) == NULL)
//        {
//            std::cout << "Found Element for xtents "
//                      << xtents.getExtent(0) << " " << xtents.getExtent(1) << " "
//                      << xtents.getExtent(2) << " " << xtents.getExtent(3) << " "
//                      << xtents.getExtent(4) << " " << xtents.getExtent(5) << std::endl;

//            //if exact extents don't exist, check if subextent exist
//            //disabled until we know how to make it work
////            if((currentArray=currentMap->getCacheElementContains(xtents)) != NULL)
////            {

////                //extract the correct extents from the superset
////                currentArray = ReaderCache::extractFromArray(xtents, currentArray);
////            }
//        }

    }


//    std::cout << "ReturnValue: " << ((currentArray != NULL) ? "GOOD" : "NULL" ) << std::endl;
    //return the array if found... otherwise, return NULL
    return currentArray;

}

//=========================================================================================
void RCache::ReaderCache::cleanCache()
{
    QMap<double, QStack<RCache::cacheElement*>* >::Iterator iter;
    QStack<RCache::cacheElement*> *currentStack;
    RCache::cacheElement* currentElement;

    //kill the cache map
    this->cache.clear();

    std::cout << "Starring Cache Cleanup... " << std::endl;

    for(iter = this->cacheStack.begin(); iter != this->cacheStack.end(); ++iter)
    {
        currentStack = iter.value();

        std::cout << "Working on time " << iter.key() << std::endl;
        std::cout << "Number of Elements to Delete: " << currentStack->size() << std::endl;

        for(int x = 0; x < currentStack->size(); x++)
        {
            currentElement = currentStack->pop();

            currentElement->data->Delete();
            delete [] currentElement;

            currentElement = NULL;
        }

//        currentStack->clear();
    }




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

        //this is deactivated until we figure out how to make it work
//        if(currentMap->getCacheElementContains(xtents) != NULL)
//        {
//            return true;
//        }

        if(currentMap->getCacheElement(xtents) != NULL)
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
    if(this->cacheStack.contains(time))
    {
        currentStack = this->cacheStack[time];

        for(itr=currentStack->begin(); itr != currentStack->end(); ++itr)
        {
            currentElement = dynamic_cast<RCache::cacheElement*>(*itr);
            if(currentElement->xtents == Xtents)
            {
                //remove and then insert at the top
                if(itr != currentStack->begin())
                {
                    //remove elemnt
                    currentStack->remove(count);
                    //insert at the top
                    currentStack->push(currentElement);
                }
                //don't go beyond the point we are looking for
                break;
            }

            //for some reason, we don't have a remove() method for iterators
          count++;
        }
    }
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
    this->clearCacheMap();
}

//=========================================================================================
RCache::cacheElement* RCache::cacheMap::addCacheElement(RCache::extents xtents, vtkAbstractArray *data)
{
    cacheElement* newElement = new cacheElement;
    newElement->data = data;
    newElement->xtents = xtents;

    //increase referene count
    data->SetReferenceCount(data->GetReferenceCount() + 1);


    std::map<RCache::extents, RCache::cacheElement*>::iterator itr;

    for(itr = this->map.begin(); itr != this->map.end(); ++itr)
    {
        if(itr->first == xtents)
        {
            std::cout << "Found existing Element" << std::endl;
            return NULL;
        }
    }

    std::cout << "Adding Element to Cache Map" << std::endl;
    this->map[xtents] = newElement;
    return newElement;

}

//=========================================================================================
RCache::cacheElement *RCache::cacheMap::getCacheElement(RCache::extents xtents)
{

    std::map<RCache::extents, RCache::cacheElement*>::iterator itr;

    for(itr = this->map.begin(); itr != this->map.end(); ++itr)
    {
        if(itr->first == xtents)
        {
            std::cout << "Element Found..." << std::endl;
            return itr->second;
        }
    }
    std::cout << "Element NOT found" << std::endl;

    return NULL;
}

//=========================================================================================
RCache::cacheElement *RCache::cacheMap::getCacheElementContains(RCache::extents xtents)
{
    //compare the key for each element with requested
    //and determine if it contains the requests extents

//    std::map<RCache::extents, RCache::cacheElement*>::iterator iter = this->map.begin();

//    //iterate through the map until we find an object that CONTAINS the extents we are looking for
//    while(iter != this->map.end())
//    {
//        //checks to see if the requested extents are contained in the current entry
//        if(isContained(xtents, iter->first) || xtents == iter->first)
//        {
//            //return the vtkAbstractArray pointer for this mapped element
//            return iter->second;
//        }
//    }

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
        delete [] iter->second;
        //remove the entry
        this->map.erase(iter);
    }
}

//=========================================================================================
void RCache::cacheMap::clearCacheMap()
{
    //this clears the cache map, but DOES NOT free any of the cache elements

    std::map<RCache::extents, RCache::cacheElement*>::iterator iter;

    for(iter=this->map.begin(); iter != this->map.end(); ++iter)
    {
        this->map.erase(iter);
    }
}




