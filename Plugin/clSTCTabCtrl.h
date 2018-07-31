#ifndef CLSTCTABCTRL_H
#define CLSTCTABCTRL_H

#include "codelite_exports.h"
#include "clTabCtrlBase.h"

class WXDLLIMPEXP_SDK clSTCTabCtrl : public clTabCtrlBase
{
public:
    clSTCTabCtrl(wxWindow* notebook, size_t style);
    virtual ~clSTCTabCtrl();
    void AllPagesDeleted();
    void NotifyPageChanged(int oldIndex, int newIndex);
};

#endif // CLSTCTABCTRL_H
