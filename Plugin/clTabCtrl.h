#ifndef CLTABCTRL_H
#define CLTABCTRL_H

// DnD support of tabs
#include "clTabHistory.h"
#include "clTabRenderer.h"
#include "codelite_exports.h"
#include "windowstack.h"
#include <wx/bookctrl.h>
#include <wx/datetime.h>
#include <wx/dnd.h>
#include <wx/gdicmn.h>
#include <wx/menu.h>

enum class eDirection {
    kInvalid = -1,
    kRight = 0,
    kLeft = 1,
    kUp = 2,
    kDown = 3,
};

class clTabCtrl;
class WXDLLIMPEXP_SDK clTabCtrlDropTarget : public wxTextDropTarget
{
    clTabCtrl* m_tabCtrl;

public:
    clTabCtrlDropTarget(clTabCtrl* tabCtrl);
    virtual ~clTabCtrlDropTarget();
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
};

/**
 * @class clTabCtrl
 * @author Eran Ifrah
 * @brief The Window that all the tabs are drawn on
 */
class WXDLLIMPEXP_SDK clTabCtrl : public wxPanel
{
    int m_height;
    int m_vTabsWidth;
    clTabInfo::Vec_t m_tabs;
    friend class clTabCtrlDropTarget;

    size_t m_style;
    clTabColours m_colours;
    clTabInfo::Vec_t m_visibleTabs;
    int m_closeButtonClickedIndex;
    wxMenu* m_contextMenu;
    wxRect m_chevronRect;
    clTabHistory::Ptr_t m_history;
    clTabRenderer::Ptr_t m_art;

    wxDateTime m_dragStartTime;
    wxPoint m_dragStartPos;

    void DoChangeSelection(size_t index);

protected:
    void OnPaint(wxPaintEvent& e);
    void OnEraseBG(wxEraseEvent& e);
    void OnSize(wxSizeEvent& event);
    void OnWindowKeyDown(wxKeyEvent& event);
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

    /**
     * @brief loop over the tabs and set their coordiantes
     */
    void DoUpdateCoordiantes(clTabInfo::Vec_t& tabs);
    /**
     * @brief get list of tabs to draw. This call always returns the active tab as part of the list
     * It also ensures that we draw as much tabs as we can.
     * @param offset reset the 0 based index from m_tabs
     */
    void UpdateVisibleTabs();

    /**
     * @brief calculate and set the tab ctrl size
     */
    void DoSetBestSize();

    clTabInfo::Ptr_t GetTabInfo(size_t index);
    clTabInfo::Ptr_t GetTabInfo(size_t index) const;
    clTabInfo::Ptr_t GetTabInfo(wxWindow* page);
    clTabInfo::Ptr_t GetActiveTabInfo();

    WindowStack* GetStack();

    void DoDeletePage(size_t page) { RemovePage(page, true, true); }
    void DoShowTabList();
    void DoUpdateXCoordFromPage(wxWindow* page, int diff);

    void OnBeginDrag();

public:
    clTabCtrl(wxWindow* notebook, size_t style);
    virtual ~clTabCtrl();
    
    int GetPageIndex(wxWindow* win) const;
    int GetPageIndex(const wxString& label) const;

    /**
     * @brief return the art class used by this tab control
     */
    clTabRenderer::Ptr_t GetArt() { return m_art; }

    /**
     * @brief replace the art used by this tab control
     */
    void SetArt(clTabRenderer::Ptr_t art);

    virtual bool IsVerticalTabs() const;

    void SetColours(const clTabColours& colours) { this->m_colours = colours; }
    const clTabColours& GetColours() const { return m_colours; }

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
     * @brief return true if index is in the tabs vector range
     */
    virtual bool IsIndexValid(size_t index) const;

    void SetStyle(size_t style);
    size_t GetStyle() const { return m_style; }

    /**
     * @brief update the selected tab. This function also fires an event
     */
    virtual int SetSelection(size_t tabIdx);

    /**
     * @brief update the selected tab. This function does not fire an event
     */
    virtual int ChangeSelection(size_t tabIdx);

    /**
     * @brief return the current selection. return wxNOT_FOUND if non is selected
     */
    virtual int GetSelection() const;

    /**
     * @brief Sets the text for the given page.
     */
    virtual bool SetPageText(size_t page, const wxString& text);
    virtual wxString GetPageText(size_t page) const;

    virtual void AddPage(clTabInfo::Ptr_t tab);
    virtual bool InsertPage(size_t index, clTabInfo::Ptr_t tab);

    virtual void SetPageBitmap(size_t index, const wxBitmap& bmp);
    virtual wxBitmap GetPageBitmap(size_t index) const;
    virtual wxWindow* GetPage(size_t index) const;
    virtual void GetAllPages(std::vector<wxWindow*>& pages);
    virtual int FindPage(wxWindow* page) const;
    virtual bool RemovePage(size_t page, bool notify, bool deletePage);
    virtual bool DeleteAllPages();
    virtual void SetMenu(wxMenu* menu);
    virtual bool SetPageToolTip(size_t page, const wxString& tooltip);

    const clTabInfo::Vec_t& GetTabs() const { return m_tabs; }
    clTabHistory::Ptr_t GetHistory() const { return m_history; }
};

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CHANGING, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CHANGED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CLOSING, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CLOSED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_TAB_CONTEXT_MENU, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_PAGE_CLOSE_BUTTON, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_TAB_DCLICKED, wxBookCtrlEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_BOOK_TABAREA_DCLICKED, wxBookCtrlEvent);

#endif // CLTABCTRL_H
