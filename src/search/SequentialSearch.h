#pragma once

#include "DataStructures/LinkedList.h"

namespace search {

// The predicate selects the field/rule; returned pointers borrow the stored objects.
template <typename T, typename Range, typename Predicate>
void sequentialSearch(const Range& records, Predicate matches,
                      LinkedList<const T*>& result)
{
    LinkedList<const T*> found;
    for (const T& record : records)
        if (matches(record)) found.pushBack(&record);
    result.swap(found);
}

}
