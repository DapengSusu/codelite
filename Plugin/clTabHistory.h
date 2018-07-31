#ifndef CLTABHISTORY_H
#define CLTABHISTORY_H

#include "codelite_exports.h"
#include <vector>
#include <wx/sharedptr.h>
#include <algorithm>

class WXDLLIMPEXP_SDK clTabHistory
{
    std::vector<void*> m_history;

public:
    typedef wxSharedPtr<clTabHistory> Ptr_t;

public:
    clTabHistory() {}
    virtual ~clTabHistory() {}

    void Push(void* tabID)
    {
        Pop(tabID);
        m_history.insert(m_history.begin(), tabID);
    }

    void Pop(void* tabID)
    {
        std::vector<void*>::iterator iter =
            std::find_if(m_history.begin(), m_history.end(), [&](void* p) { return p == tabID; });
        if(iter != m_history.end()) {
            m_history.erase(iter);
        }
    }

    void* PrevPage()
    {
        if(m_history.empty()) {
            return nullptr;
        }
        // return the top of the heap
        return m_history[0];
    }

    /**
     * @brief clear the history
     */
    void Clear() { m_history.clear(); }

    /**
     * @brief return the tabbing history
     * @return
     */
    const std::vector<void*>& GetHistory() const { return m_history; }
};

#endif // CLTABHISTORY_H
