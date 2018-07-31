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
    wxWindow* m_defaultPage;

protected:
    clSTCEventsHandler* GetActiveHandler() const;
    void ChangeSelection(clSTCEventsHandler* newHandler);
    clSTCEventsHandler* GetHandler(int index = wxNOT_FOUND) const;
    void DoShowPage(wxWindow* win, bool show, int proportion);

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
     * @brief similar to SetSelection, but no events are fired
     */
    void ChangeSelection(size_t index);

    /**
     * @brief return the current selection
     */
    int GetSelection() const;

    /**
     * @brief return the page associated with a given index
     * @param index
     * @return
     */
    clSTCEventsHandler* GetPage(int index) const { return GetHandler(index); }

    /**
     * @brief set page text
     */
    void SetPageText(int index, const wxString& label);

    /**
     * @brief return page index
     */
    wxString GetPageText(int index) const;

    /**
     * @brief delete all pages in the notebook
     */
    void DeleteAllPages();

    /**
     * @brief When the notebook is empty, display this page
     */
    void SetDefaultPage(wxWindow* page);

    /**
     * @brief hide the notebook and show the default page instead
     */
    void DisplayDefaultPage(bool show = true);
    
    /**
     * @brief set the notebook style
     * @param style
     */
    void SetStyle(size_t style);
};

#endif // CLSTCBOOKCTRL_H
