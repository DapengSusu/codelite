//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// Copyright            : (C) 2015 Eran Ifrah
// File name            : Notebook.h
//
// -------------------------------------------------------------------------
// A
//              _____           _      _     _ _
//             /  __ \         | |    | |   (_) |
//             | /  \/ ___   __| | ___| |    _| |_ ___
//             | |    / _ \ / _  |/ _ \ |   | | __/ _ )
//             | \__/\ (_) | (_| |  __/ |___| | ||  __/
//              \____/\___/ \__,_|\___\_____/_|\__\___|
//
//                                                  F i l e
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef NOTEBOOK_H
#define NOTEBOOK_H

#include "cl_defs.h"

#include "clTabCtrl.h"
#include "clTabHistory.h"
#include "clTabRenderer.h"
#include "windowstack.h"
#include <list>
#include <vector>
#include <wx/bookctrl.h>
#include <wx/dcmemory.h>
#include <wx/dnd.h>
#include <wx/dynarray.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sharedptr.h>

class Notebook;
class wxMenu;
class clTabCtrl;

/**
 * @class Notebook
 * @author Eran Ifrah
 * @brief A modern notebook (similar to the ones seen on Sublime Text and Atom editors
 * for wxWidgets. The class implementation uses wxSimplebook as the tab container and a
 * custom drawing tab area (see above the class clTabCtrl)
 */
class WXDLLIMPEXP_SDK Notebook : public wxPanel
{
    WindowStack* m_windows;
    clTabCtrl* m_tabCtrl;
    friend class clTabCtrl;

protected:
    void DoChangeSelection(wxWindow* page);
    bool IsVerticalTabs() const { return m_tabCtrl->IsVerticalTabs(); }

public:
    /**
     * Constructor
     */
    Notebook(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxEmptyString);

    /**
     * @brief update the notebook art class and refresh
     */
    void SetArt(clTabRenderer::Ptr_t art) { m_tabCtrl->SetArt(art); }

    /**
     * @brief set the notebook style. The style bits are kNotebook_* (you can set several
     * styles OR-ed)
     */
    void SetStyle(size_t style);

    /**
     * @brief set the tab direction
     */
    void SetTabDirection(wxDirection d);

    /**
     * @brief return the book style
     */
    size_t GetStyle() const { return m_tabCtrl->GetStyle(); }

    /**
     * @brief enable a specific style in the notebook
     */
    void EnableStyle(NotebookStyle style, bool enable);

    /**
     * destructor
     */
    virtual ~Notebook();

    /**
     * @brief append page to the notebook
     */
    void AddPage(wxWindow* page, const wxString& label, bool selected = false, const wxBitmap& bmp = wxNullBitmap);

    /**
     * @brief insert page at a specified position
     */
    bool InsertPage(size_t index, wxWindow* page, const wxString& label, bool selected = false,
                    const wxBitmap& bmp = wxNullBitmap);

    /**
     * @brief return the currently selected page or null
     */
    wxWindow* GetCurrentPage() const;

    /**
     * @brief Returns the index of the specified tab window or wxNOT_FOUND if not found
     */
    int FindPage(wxWindow* page) const;

    /**
     * @brief Deletes the specified page, without deleting the associated window
     */
    bool RemovePage(size_t page, bool notify = false);

    /**
     * @brief Deletes the specified page and the associated window
     */
    bool DeletePage(size_t page, bool notify = true);

    /**
     * @brief Deletes all pages
     */
    bool DeleteAllPages();

    /**
     * @brief set a new selection. This function fires an event that can be vetoed
     */
    int SetSelection(size_t selection) { return m_tabCtrl->SetSelection(selection); }
    /**
     * @brief set new selection. No events are fired
     */
    int ChangeSelection(size_t selection) { return m_tabCtrl->ChangeSelection(selection); }

    /**
     * @brief return the currently selected page, return wxNOT_FOUND if non found
     */
    int GetSelection() const { return m_tabCtrl->GetSelection(); }

    /**
     * @brief Sets the text for the given page.
     */
    bool SetPageText(size_t page, const wxString& text) { return m_tabCtrl->SetPageText(page, text); }

    /**
     * @brief Returns the string for the given page
     */
    wxString GetPageText(size_t page) const { return m_tabCtrl->GetPageText(page); }

    /**
     * @brief set the image for the given page
     */
    void SetPageBitmap(size_t index, const wxBitmap& bmp) { m_tabCtrl->SetPageBitmap(index, bmp); }

    /**
     * @brief return bitmap for a given page. Return wxNullBitmap if invalid page
     */
    wxBitmap GetPageBitmap(size_t index) const { return m_tabCtrl->GetPageBitmap(index); }

    // Base class members...
    virtual bool SetPageImage(size_t page, int image)
    {
        wxUnusedVar(page);
        wxUnusedVar(image);
        return false;
    }
    virtual int GetPageImage(size_t n) const { return wxNOT_FOUND; }

    /**
     * @brief return the index of a given window in the tab control
     * @param window
     * @return return window index, or wxNOT_FOUND
     */
    int GetPageIndex(wxWindow* window) const { return m_tabCtrl->GetPageIndex(window); }

    /**
     * @brief return the index of a given window by its title
     */
    int GetPageIndex(const wxString& label) const { return m_tabCtrl->GetPageIndex(label); }

    /**
     * @brief Returns the number of pages in the control
     */
    size_t GetPageCount() const { return m_tabCtrl->GetTabs().size(); }

    /**
     * @brief Returns the window at the given page position.
     */
    wxWindow* GetPage(size_t index) const { return m_tabCtrl->GetPage(index); }

    /**
     * @brief return an array of all the windows managed by this notebook
     */
    void GetAllPages(std::vector<wxWindow*>& pages) { m_tabCtrl->GetAllPages(pages); }

    /**
     * @brief return all tabs info
     * @param tabs [output]
     */
    size_t GetAllTabs(clTabInfo::Vec_t& tabs)
    {
        tabs = m_tabCtrl->GetTabs();
        return tabs.size();
    }

    /**
     * @brief set a context menu to be shown whe context menu is requested
     * on a tab label
     */
    void SetMenu(wxMenu* menu) { m_tabCtrl->SetMenu(menu); }

    /**
     * @brief Sets the tool tip displayed when hovering over the tab label of the page
     * @return true if tool tip was updated, false if it failed, e.g. because the page index is invalid.
     */
    bool SetPageToolTip(size_t page, const wxString& tooltip) { return m_tabCtrl->SetPageToolTip(page, tooltip); }

    /**
     * @brief return the tabbing history
     * @return
     */
    clTabHistory::Ptr_t GetHistory() const { return m_tabCtrl->GetHistory(); }

    /**
     * @brief move the active page and place it in the new nexIndex
     */
    bool MoveActivePage(int newIndex);
};

#endif // NOTEBOOK_H
