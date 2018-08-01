#ifndef CLTABCTRLBASE_H
#define CLTABCTRLBASE_H

#include "clTabHistory.h"
#include "clTabRenderer.h"
#include "codelite_exports.h"
#include <wx/bookctrl.h>
#include <wx/datetime.h>
#include <wx/dnd.h>
#include <wx/menu.h>
#include <wx/panel.h>

class clTabCtrlBase;
enum class eDirection {
    kInvalid = -1,
    kRight = 0,
    kLeft = 1,
    kUp = 2,
    kDown = 3,
};

class WXDLLIMPEXP_SDK clTabCtrlDropTarget : public wxTextDropTarget
{
    clTabCtrlBase* m_tabCtrl;

public:
    clTabCtrlDropTarget(clTabCtrlBase* tabCtrl);
    virtual ~clTabCtrlDropTarget();
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
};

class WXDLLIMPEXP_SDK clTabCtrlBase : public wxPanel
{
protected:
    size_t m_style;
    int m_closeButtonClickedIndex;
    wxMenu* m_contextMenu;
    wxDateTime m_dragStartTime;
    clTabColours m_colours;
    clTabInfo::Vec_t m_visibleTabs;
    wxRect m_chevronRect;
    clTabHistory::Ptr_t m_history;
    clTabRenderer::Ptr_t m_art;
    wxPoint m_dragStartPos;
    int m_height;
    int m_vTabsWidth;
    clTabInfo::Vec_t m_tabs;

public:
    clTabCtrlBase(wxWindow* notebook, size_t style);
    virtual ~clTabCtrlBase();

protected:
    // Pure function, must be implemented by subclasses
    virtual bool CanChangePage(int index);
    virtual void NotifyPageChanged(int oldIndex, int newIndex);
    virtual bool CanClosePage(int index);
    virtual void NotifyPageClosed();
    virtual void NotifyPageButtonClicked(int index);
    
    // Override these to do something unique in your subclass
    virtual void PageRemoved(clTabInfo::Ptr_t tab, bool deleteIt) {}
    virtual void BeforePageChange(clTabInfo::Ptr_t tab) {}
    virtual void AllPagesDeleted() {}
    
protected:
    void DoDeletePage(size_t page) { RemovePage(page, true, true); }
    bool DoChangeSelection(size_t index);
    void DoSetBestSize();
    void OnPaint(wxPaintEvent& e);
    void OnEraseBG(wxEraseEvent& e);
    void OnSize(wxSizeEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnRightUp(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnLeftDClick(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);
    void OnMouseMiddleClick(wxMouseEvent& event);
    void OnMouseScroll(wxMouseEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void DoDrawBottomBox(clTabInfo::Ptr_t activeTab, const wxRect& clientRect, wxDC& dc, const clTabColours& colours);
    bool ShiftRight(clTabInfo::Vec_t& tabs);
    bool ShiftBottom(clTabInfo::Vec_t& tabs);
    bool IsActiveTabInList(const clTabInfo::Vec_t& tabs) const;
    bool IsActiveTabVisible(const clTabInfo::Vec_t& tabs) const;
    void OnBeginDrag();
    clTabInfo::Ptr_t GetActiveTabInfo();
    void UpdateVisibleTabs();
    /**
     * @brief loop over the tabs and set their coordiantes
     */
    void DoUpdateCoordiantes(clTabInfo::Vec_t& tabs);
    void DoUpdateXCoordFromPage(clTabInfo::Ptr_t tab, int diff);
    clTabInfo::Ptr_t GetTabInfoById(size_t id) const;
    clTabInfo::Ptr_t GetTabInfoByPtr(void* ptr) const;
    int GetTabIndexByPtr(void* p) const;
    void DoShowTabList();
    int GetTabRealIndex(clTabInfo::Ptr_t tab) const;

public:
    clTabInfo::Ptr_t GetTabInfo(size_t index) const;
    bool RemovePage(size_t page, bool notify, bool deletePage);
    
    virtual bool InsertPage(size_t index, clTabInfo::Ptr_t tab);
    void AddPage(clTabInfo::Ptr_t tab) { InsertPage(m_tabs.size(), tab); }
    
    size_t GetPageCount() const { return m_tabs.size(); }
    
    /**
     * @brief update the selected tab. This function does not fire an event
     */
    bool ChangeSelection(size_t tabIdx);

    /**
     * @brief update the selected tab. This function also fires an event
     */
    bool SetSelection(size_t tabIdx);

    /**
     * @brief replace the art used by this tab control
     */
    void SetArt(clTabRenderer::Ptr_t art);
    /**
     * @brief return the art class used by this tab control
     */
    clTabRenderer::Ptr_t GetArt() { return m_art; }
    bool IsVerticalTabs() const { return (m_style & kNotebook_RightTabs) || (m_style & kNotebook_LeftTabs); }

    void SetColours(const clTabColours& colours) { this->m_colours = colours; }
    const clTabColours& GetColours() const { return m_colours; }

    void SetStyle(size_t style);
    size_t GetStyle() const { return m_style; }
    /**
     * @brief return the current selection. return wxNOT_FOUND if non is selected
     */
    int GetSelection() const;

    /**
     * @brief test if pt is on one of the visible tabs return its index
     * @param pt mouse click position
     * @param realPosition [output] the index position in the m_tabs array
     * @param tabHit [output] the index position in the m_visibleTabs array
     * @param leftSide [output] if the point is on the LEFT side of the tab's rect, then return wxALIGN_LEFT, otherwise
     * return wxALIGN_RIGHT. Another possible value is wxALIGN_INVALID
     */
    void TestPoint(const wxPoint& pt, int& realPosition, int& tabHit, eDirection& align);

    /**
     * @brief Move the active tab to a new position
     * @param newIndex the new position. 0-based index in the m_tabs array
     */
    bool MoveActiveToIndex(int newIndex, eDirection direction);

    /**
     * @brief set a tooltip for page
     */
    bool SetPageToolTip(size_t page, const wxString& tooltip);

    /**
     * @brief set context menu for the tab control
     */
    void SetMenu(wxMenu* menu);

    /**
     * @brief return true if index is in the tabs vector range
     */
    bool IsIndexValid(size_t index) const;

    const clTabInfo::Vec_t& GetTabs() const { return m_tabs; }
    clTabHistory::Ptr_t GetHistory() const { return m_history; }

    /**
     * @brief Sets the text for the given page.
     */
    bool SetPageText(size_t page, const wxString& text);
    wxString GetPageText(size_t page) const;

    /**
     * @brief return the bitmap associated with a tab
     */
    wxBitmap GetPageBitmap(size_t index) const;

    /**
     * @brief set a bitmap to page
     */
    bool SetPageBitmap(size_t index, const wxBitmap& bmp);
    
    /**
     * @brief delete all pages
     */
    void DeleteAllPages();
    
    /**
     * @brief return page index by its label
     */
    int GetPageIndex(const wxString& label) const;
    
    /**
     * @brief return the index of a given pointer
     */
    int GetPageIndex(void* p) const { return GetTabIndexByPtr(p); }
};

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CHANGING, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CHANGED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CLOSING, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CLOSED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_TAB_CONTEXT_MENU, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CLOSE_BUTTON, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_TAB_DCLICKED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_TABAREA_DCLICKED, wxBookCtrlEvent);

#endif // CLTABCTRLBASE_H
