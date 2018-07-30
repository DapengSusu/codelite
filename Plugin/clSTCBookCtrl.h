#ifndef CLSTCBOOKCTRL_H
#define CLSTCBOOKCTRL_H

#include "codelite_exports.h"
#include "wxStringHash.h"
#include <vector>
#include <wx/bookctrl.h>
#include <wx/filename.h>
#include <wx/panel.h>
#include <wx/stc/stc.h>
#include <wx/string.h>

/**
 * @class clSTCBookCtrl
 * @author eran
 * @date 30/07/18
 * @file clSTCBookCtrl.h
 * @brief an optimized book control for wxStyledTextCtrl only. This class uses a single wxStyledTextCtrl
 * to hold multiple files. If you wish to handle the events from wxSTC, you should provide a sink object
 * derived from clSTCEventsHandler
 */
class clSTCEventsHandler;
class WXDLLIMPEXP_SDK clSTCBookCtrl : public wxPanel
{
    wxStyledTextCtrl* m_stc;
    std::unordered_map<wxString, clSTCEventsHandler*> m_handlersMap;
    std::vector<clSTCEventsHandler*> m_handlersVec;

protected:
    clSTCEventsHandler* GetActiveHandler() const;
    void ChangeSelection(clSTCEventsHandler* newHandler);
    clSTCEventsHandler* GetHandler(int index = wxNOT_FOUND);

    // this method fires events
    void SetSelection(clSTCEventsHandler* handler);

public:
    clSTCBookCtrl(wxWindow* parent);
    virtual ~clSTCBookCtrl();

    /**
     * @brief add new file (represented by handler class) to the notebook
     * if selected is 'true', this methods internall calls SetSelection
     * which fires the book changing/changed events
     */
    void AddPage(clSTCEventsHandler* handler, const wxString& label, bool selected, const wxBitmap& bmp);

    /**
     * @brief change the selection. This functions send the events wxEVT_BOOK_PAGE_CHANGING and wxEVT_BOOK_PAGE_CHANGED
     */
    void SetSelection(size_t index);
    
    /**
     * @brief return the current selection
     */
    int GetSelection() const;
};

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_PAGE_CHANGING, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_PAGE_CHANGED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_PAGE_CLOSING, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_PAGE_CLOSED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_TAB_CONTEXT_MENU, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_PAGE_CLOSE_BUTTON, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_TAB_DCLICKED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_STC_BOOK_TABAREA_DCLICKED, wxBookCtrlEvent);

#endif // CLSTCBOOKCTRL_H
