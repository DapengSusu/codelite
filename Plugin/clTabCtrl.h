#ifndef CLTABCTRL_H
#define CLTABCTRL_H

// DnD support of tabs
#include "clTabCtrlBase.h"
#include "clTabHistory.h"
#include "clTabRenderer.h"
#include "codelite_exports.h"
#include "windowstack.h"
#include <wx/bookctrl.h>
#include <wx/datetime.h>
#include <wx/dnd.h>
#include <wx/gdicmn.h>
#include <wx/menu.h>

class clTabCtrl;
/**
 * @class clTabCtrl
 * @author Eran Ifrah
 * @brief The Window that all the tabs are drawn on
 */
class WXDLLIMPEXP_SDK clTabCtrl : public clTabCtrlBase
{
    WindowStack* GetStack();

protected:
    /**
     * @brief a page has been changed, notify
     */
    void NotifyPageChanged(int oldIndex, int newIndex);
    /**
     * @brief A page is about to change (this action can not be vetoed). You execute your own code here
     * (e.g. remove the page from the view)
     */
    void BeforePageChange(clTabInfo::Ptr_t tab);

    void PageRemoved(clTabInfo::Ptr_t tab, bool deleteIt);

    /**
     * @brief all pages were removed, you can something about it here...
     */
    void AllPagesDeleted();
    
public:
    clTabCtrl(wxWindow* notebook, size_t style);
    virtual ~clTabCtrl();

    int GetPageByWin(wxWindow* win) const;
    
    wxWindow* GetPage(size_t index) const;
    void GetAllPages(std::vector<wxWindow*>& pages);
    int FindPage(wxWindow* page) const;
};

#endif // CLTABCTRL_H
