#ifndef CLSTCBOOKCTRL_H
#define CLSTCBOOKCTRL_H

#include "codelite_exports.h"
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
class clSTCTabCtrl;
class clSTCEventsHandler;
class WXDLLIMPEXP_SDK clSTCBookCtrl : public wxPanel
{
    wxStyledTextCtrl* m_stc;
    clSTCTabCtrl* m_tabCtrl;

protected:
    clSTCEventsHandler* GetActiveHandler() const;
    void ChangeSelection(clSTCEventsHandler* newHandler);
    clSTCEventsHandler* GetHandler(int index = wxNOT_FOUND) const;

public:
    clSTCBookCtrl(wxWindow* parent);
    virtual ~clSTCBookCtrl();

    wxStyledTextCtrl* GetCtrl() { return m_stc; }

    /**
     * @brief add new file (represented by handler class) to the notebook
     * if selected is 'true', this methods internall calls SetSelection
     * which fires the book changing/changed events
     */
    void AddPage(clSTCEventsHandler* handler, const wxFileName& filename, const wxString& label, bool selected = true,
                 const wxBitmap& bmp = wxNullBitmap);

    /**
     * @brief change the selection. This functions send the events wxEVT_BOOK_PAGE_CHANGING and wxEVT_BOOK_PAGE_CHANGED
     */
    void SetSelection(size_t index);

    /**
     * @brief return the current selection
     */
    int GetSelection() const;
};

#endif // CLSTCBOOKCTRL_H
