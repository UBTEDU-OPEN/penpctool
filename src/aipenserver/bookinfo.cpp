#include "bookinfo.h"

BookInfo::BookInfo()
{
}


int BookInfo::getWordDetail(int pageId,int zIndex, WordDetail& wordDetail){

    if(pageMap.contains(pageId)){
        auto indexMap = pageMap[pageId];
        if(indexMap.contains(zIndex)){
            wordDetail =  indexMap[zIndex];
            return wordDetail.contentCode;
        }
    }
    return 0;
}
