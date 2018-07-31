#include "clTabCtrlBase.h"
#include "clTabRendererClassic.h"
#include "clTabRendererCurved.h"
#include "clTabRendererSquare.h"
#include <wx/dcbuffer.h>
#include <wx/regex.h>
#include <wx/wupdlock.h>
#include <wx/stc/stc.h>

#if CL_BUILD
#include "editor_config.h"
#include "event_notifier.h"
#include <ColoursAndFontsManager.h>
#include <lexer_configuration.h>
#endif

wxDEFINE_EVENT(wxEVT_BOOK_PAGE_CHANGING, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_BOOK_PAGE_CHANGED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_BOOK_PAGE_CLOSING, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_BOOK_PAGE_CLOSED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_BOOK_PAGE_CLOSE_BUTTON, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_BOOK_TAB_DCLICKED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_BOOK_TABAREA_DCLICKED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_BOOK_TAB_CONTEXT_MENU, wxBookCtrlEvent);

clTabCtrlBase::clTabCtrlBase(wxWindow* notebook, size_t style)
    : wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxWANTS_CHARS | wxTAB_TRAVERSAL)
    , m_style(style)
    , m_closeButtonClickedIndex(wxNOT_FOUND)
    , m_contextMenu(NULL)
    , m_dragStartTime((time_t)-1)
{
    SetBackgroundColour(DrawingUtils::GetPanelBgColour());
    SetBackgroundStyle(wxBG_STYLE_PAINT);
#if CL_BUILD
    if(EditorConfigST::Get()->GetOptions()->GetOptions() & OptionsConfig::Opt_TabStyleMinimal) {
        m_art.reset(new clTabRendererSquare);
    } else if(EditorConfigST::Get()->GetOptions()->GetOptions() & OptionsConfig::Opt_TabStyleTRAPEZOID) {
        m_art.reset(new clTabRendererCurved);
    } else {
        // the default
        m_art.reset(new clTabRendererClassic);
    }
#else
    m_art.reset(new clTabRendererSquare);
#endif
    DoSetBestSize();
    SetDropTarget(new clTabCtrlDropTarget(this));
    Bind(wxEVT_PAINT, &clTabCtrlBase::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &clTabCtrlBase::OnEraseBG, this);
    Bind(wxEVT_SIZE, &clTabCtrlBase::OnSize, this);
    Bind(wxEVT_LEFT_DOWN, &clTabCtrlBase::OnLeftDown, this);
    Bind(wxEVT_RIGHT_UP, &clTabCtrlBase::OnRightUp, this);
    Bind(wxEVT_LEFT_UP, &clTabCtrlBase::OnLeftUp, this);
    Bind(wxEVT_MOTION, &clTabCtrlBase::OnMouseMotion, this);
    Bind(wxEVT_MIDDLE_UP, &clTabCtrlBase::OnMouseMiddleClick, this);
    Bind(wxEVT_MOUSEWHEEL, &clTabCtrlBase::OnMouseScroll, this);
    Bind(wxEVT_CONTEXT_MENU, &clTabCtrlBase::OnContextMenu, this);
    Bind(wxEVT_LEFT_DCLICK, &clTabCtrlBase::OnLeftDClick, this);

    if(m_style & kNotebook_DarkTabs) {
        m_colours.InitDarkColours();
    } else {
        m_colours.InitLightColours();
    }
    SetStyle(m_style);
    // The history object
    m_history.reset(new clTabHistory());
}

clTabCtrlBase::~clTabCtrlBase()
{
    wxDELETE(m_contextMenu);
    Unbind(wxEVT_PAINT, &clTabCtrlBase::OnPaint, this);
    Unbind(wxEVT_ERASE_BACKGROUND, &clTabCtrlBase::OnEraseBG, this);
    Unbind(wxEVT_SIZE, &clTabCtrlBase::OnSize, this);
    Unbind(wxEVT_LEFT_DOWN, &clTabCtrlBase::OnLeftDown, this);
    Unbind(wxEVT_RIGHT_UP, &clTabCtrlBase::OnRightUp, this);
    Unbind(wxEVT_LEFT_UP, &clTabCtrlBase::OnLeftUp, this);
    Unbind(wxEVT_MOTION, &clTabCtrlBase::OnMouseMotion, this);
    Unbind(wxEVT_MIDDLE_UP, &clTabCtrlBase::OnMouseMiddleClick, this);
    Unbind(wxEVT_CONTEXT_MENU, &clTabCtrlBase::OnContextMenu, this);
    Unbind(wxEVT_LEFT_DCLICK, &clTabCtrlBase::OnLeftDClick, this);
    Unbind(wxEVT_MOUSEWHEEL, &clTabCtrlBase::OnMouseScroll, this);
}

void clTabCtrlBase::DoSetBestSize()
{
    wxBitmap bmp(1, 1);
    wxMemoryDC memDC(bmp);
    wxFont font = clTabRenderer::GetTabFont();
    memDC.SetFont(font);

    wxSize sz = memDC.GetTextExtent("Tp");

    m_height = sz.GetHeight() + (4 * GetArt()->ySpacer);
    m_vTabsWidth = sz.GetHeight() + (5 * GetArt()->ySpacer);
    if(IsVerticalTabs()) {
        SetSizeHints(wxSize(m_vTabsWidth, -1));
        SetSize(m_vTabsWidth, -1);
    } else {
        SetSizeHints(wxSize(-1, m_height));
        SetSize(-1, m_height);
    }
}

// Events
void clTabCtrlBase::OnPaint(wxPaintEvent& e)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    wxRect clientRect(GetClientRect());
    if(clientRect.width <= 3) return;
    if(clientRect.height <= 3) return;

    m_chevronRect = wxRect();
    wxRect rect(GetClientRect());

    if((GetStyle() & kNotebook_ShowFileListButton)) {
        if(IsVerticalTabs()) {
            int width = rect.GetWidth() > CHEVRON_SIZE ? CHEVRON_SIZE : rect.GetWidth();
            int x = (rect.GetWidth() - width) / 2;
            wxPoint topLeft = rect.GetTopLeft();
            topLeft.x = x;
            m_chevronRect = wxRect(topLeft, wxSize(width, CHEVRON_SIZE));
            rect.y = m_chevronRect.GetBottomLeft().y;
            rect.SetHeight(rect.GetHeight() - m_chevronRect.GetHeight());
        } else {
            wxPoint rightPoint = rect.GetRightTop();
            rightPoint.x -= CHEVRON_SIZE;
            m_chevronRect = wxRect(rightPoint, wxSize(CHEVRON_SIZE, rect.GetHeight()));
            rect.SetWidth(rect.GetWidth() - CHEVRON_SIZE);
        }
    }

    if(m_tabs.empty()) {
        // Draw the default bg colour
        dc.SetPen(DrawingUtils::GetPanelBgColour());
        dc.SetBrush(DrawingUtils::GetPanelBgColour());
        dc.DrawRectangle(GetClientRect());
        return;
    }

    // Draw the tab area background colours
    clTabInfo::Ptr_t active_tab = GetActiveTabInfo();
    wxColour tabAreaBgCol = m_colours.tabAreaColour;
    clTabColours activeTabColours = m_colours;

#if CL_BUILD
    if(active_tab && (GetStyle() & kNotebook_EnableColourCustomization)) {
        // the background colour is set according to the active tab colour
        clColourEvent colourEvent(wxEVT_COLOUR_TAB);
        colourEvent.SetPage(active_tab->GetPtrAs<wxWindow>());
        if(EventNotifier::Get()->ProcessEvent(colourEvent)) {
            tabAreaBgCol = colourEvent.GetBgColour();
            activeTabColours.InitFromColours(colourEvent.GetBgColour(), colourEvent.GetFgColour());
        }
    }
#endif

    // Draw background
    dc.SetPen(DrawingUtils::GetPanelBgColour());
    dc.SetBrush(DrawingUtils::GetPanelBgColour());
#ifdef __WXOSX__
    clientRect.Inflate(1, 1);
#endif
    dc.DrawRectangle(clientRect);
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        m_tabs[i]->CalculateOffsets(GetStyle());
    }

    // Sanity
    if(!(rect.GetSize().x > 0 && rect.GetSize().y > 0)) {
        m_visibleTabs.clear();
        return;
    }

#ifdef __WXGTK__
    wxDC& gcdc = dc;
#else
    wxGCDC gcdc(dc);
    PrepareDC(gcdc);
#endif
    if(!IsVerticalTabs()) {
        gcdc.SetClippingRegion(clientRect.x, clientRect.y, clientRect.width - CHEVRON_SIZE, clientRect.height);
        dc.SetClippingRegion(clientRect.x, clientRect.y, clientRect.width - CHEVRON_SIZE, clientRect.height);
    }

    gcdc.SetPen(DrawingUtils::GetPanelBgColour());
    gcdc.SetBrush(DrawingUtils::GetPanelBgColour());
    gcdc.DrawRectangle(rect.GetSize());
    UpdateVisibleTabs();

    int activeTabInex = wxNOT_FOUND;
    for(int i = (m_visibleTabs.size() - 1); i >= 0; --i) {
        clTabInfo::Ptr_t tab = m_visibleTabs[i];
        if(tab->IsActive()) {
            activeTabInex = i;
        }

        // send event per tab to get their colours
        clTabColours* pColours = &m_colours;
#if CL_BUILD
        clColourEvent colourEvent(wxEVT_COLOUR_TAB);
        colourEvent.SetPage(tab->GetPtrAs<wxWindow>());
        clTabColours user_colours;
        if((GetStyle() & kNotebook_EnableColourCustomization) && EventNotifier::Get()->ProcessEvent(colourEvent)) {
            user_colours.InitFromColours(colourEvent.GetBgColour(), colourEvent.GetFgColour());
            pColours = &user_colours;
        }
#endif
        m_art->Draw(this, gcdc, gcdc, *tab.get(), (*pColours), m_style);
    }

    // Redraw the active tab
    if(activeTabInex != wxNOT_FOUND) {
        clTabInfo::Ptr_t activeTab = m_visibleTabs.at(activeTabInex);
        m_art->Draw(this, gcdc, gcdc, *activeTab.get(), activeTabColours, m_style);
    }
    if(!IsVerticalTabs()) {
        gcdc.DestroyClippingRegion();
        dc.DestroyClippingRegion();
    }
    if(activeTabInex != wxNOT_FOUND) {
        clTabInfo::Ptr_t activeTab = m_visibleTabs.at(activeTabInex);
        if(!(GetStyle() & kNotebook_VerticalButtons)) {
            DoDrawBottomBox(activeTab, clientRect, gcdc, activeTabColours);
        }
    }

    if((GetStyle() & kNotebook_ShowFileListButton)) {
        // Draw the chevron
        gcdc.SetPen(m_colours.inactiveTabPenColour);
        m_art->DrawChevron(this, gcdc, m_chevronRect, m_colours);
    }
}

void clTabCtrlBase::OnBeginDrag()
{
    m_dragStartTime.Set((time_t)-1); // Reset the saved values
    m_dragStartPos = wxPoint();

    // We simply drag the active tab index
    wxString dragText;
    dragText << "{Class:clTabCtrlBase,TabIndex:" << GetSelection() << "}{}";

    wxTextDataObject dragContent(dragText);
    wxDropSource dragSource(this);
    dragSource.SetData(dragContent);
    wxDragResult result = dragSource.DoDragDrop(true);
    wxUnusedVar(result);
}

void clTabCtrlBase::SetStyle(size_t style)
{
    this->m_style = style;
    if(IsVerticalTabs()) {
        SetSizeHints(wxSize(m_vTabsWidth, -1));
        SetSize(m_vTabsWidth, -1);
    } else {
        SetSizeHints(wxSize(-1, m_height));
        SetSize(-1, m_height);
    }

    if(style & kNotebook_DarkTabs) {
        m_colours.InitDarkColours();
    } else {
        m_colours.InitLightColours();
    }

    for(size_t i = 0; i < m_tabs.size(); ++i) {
        m_tabs[i]->CalculateOffsets(GetStyle());
    }
#if CL_BUILD
    if(m_style & kNotebook_DynamicColours) {
        wxString globalTheme = ColoursAndFontsManager::Get().GetGlobalTheme();
        if(!globalTheme.IsEmpty()) {
            LexerConf::Ptr_t lexer = ColoursAndFontsManager::Get().GetLexer("c++", globalTheme);
            if(lexer && lexer->IsDark()) {
                // Dark theme, update all the colours
                m_colours.activeTabBgColour = lexer->GetProperty(0).GetBgColour();
                m_colours.activeTabInnerPenColour = m_colours.activeTabBgColour;
                m_colours.activeTabPenColour = m_colours.activeTabBgColour.ChangeLightness(110);
                m_colours.activeTabTextColour = *wxWHITE;
            } else if(lexer) {
                // Light theme
                m_colours.activeTabBgColour = lexer->GetProperty(0).GetBgColour();
                m_colours.activeTabInnerPenColour = m_colours.activeTabBgColour;
                m_colours.activeTabTextColour = *wxBLACK;
            }
        }
    }
#endif

    m_visibleTabs.clear();
    Layout();
    Refresh();
}

clTabInfo::Ptr_t clTabCtrlBase::GetActiveTabInfo()
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(m_tabs[i]->IsActive()) {
            return m_tabs[i];
        }
    }
    return clTabInfo::Ptr_t(NULL);
}

void clTabCtrlBase::UpdateVisibleTabs()
{
    // don't update the list if we don't need to
    if(!IsVerticalTabs()) {
        if(IsActiveTabInList(m_visibleTabs) && IsActiveTabVisible(m_visibleTabs)) return;
    }

    // set the physical coords for each tab (we do this for all the tabs)
    DoUpdateCoordiantes(m_tabs);

    // Start shifting right tabs until the active tab is visible
    m_visibleTabs = m_tabs;
    if(!IsVerticalTabs()) {
        while(!IsActiveTabVisible(m_visibleTabs)) {
            if(!ShiftRight(m_visibleTabs)) break;
        }
    } else {
        while(!IsActiveTabVisible(m_visibleTabs)) {
            if(!ShiftBottom(m_visibleTabs)) break;
        }
    }
}

void clTabCtrlBase::DoUpdateCoordiantes(clTabInfo::Vec_t& tabs)
{
    int majorDimension = GetArt()->majorCurveWidth ? 5 : 0;
    if(IsVerticalTabs()) {
        majorDimension = (GetStyle() & kNotebook_ShowFileListButton) ? 20 : 0;
    }

    for(size_t i = 0; i < tabs.size(); ++i) {
        clTabInfo::Ptr_t tab = tabs[i];
        if(IsVerticalTabs()) {
            if(GetStyle() & kNotebook_VerticalButtons) {
                tab->GetRect().SetX(0);
            } else {
                tab->GetRect().SetX(GetStyle() & kNotebook_LeftTabs ? 1 : 0);
            }
            tab->GetRect().SetY(majorDimension);
            tab->GetRect().SetWidth(tab->GetWidth());
            tab->GetRect().SetHeight(tab->GetHeight());
            majorDimension += tab->GetHeight() - GetArt()->verticalOverlapWidth;
        } else {
            tab->GetRect().SetX(majorDimension);
            tab->GetRect().SetY(0);
            tab->GetRect().SetWidth(tab->GetWidth());
            tab->GetRect().SetHeight(tab->GetHeight());
            majorDimension += tab->GetWidth() - GetArt()->overlapWidth;
        }
    }
}

int clTabCtrlBase::GetSelection() const
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        clTabInfo::Ptr_t tab = m_tabs[i];
        if(tab->IsActive()) return i;
    }
    return wxNOT_FOUND;
}

int clTabCtrlBase::GetTabRealIndex(clTabInfo::Ptr_t tab) const
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(tab->Equals(m_tabs[i])) return (int)i;
    }
    return wxNOT_FOUND;
}

void clTabCtrlBase::TestPoint(const wxPoint& pt, int& realPosition, int& tabHit, eDirection& align)
{
    realPosition = wxNOT_FOUND;
    tabHit = wxNOT_FOUND;
    align = eDirection::kInvalid;

    if(m_visibleTabs.empty()) return;
    for(size_t i = 0; i < m_visibleTabs.size(); ++i) {
        clTabInfo::Ptr_t tab = m_visibleTabs[i];
        wxRect r(tab->GetRect());
        if(r.Contains(pt)) {
            if(IsVerticalTabs()) {
                if(pt.y > ((r.GetHeight() / 2) + r.GetY())) {
                    // the point is on the RIGHT side
                    align = eDirection::kUp;
                } else {
                    align = eDirection::kDown;
                }
            } else {
                if(pt.x > ((r.GetWidth() / 2) + r.GetX())) {
                    // the point is on the RIGHT side
                    align = eDirection::kRight;
                } else {
                    align = eDirection::kLeft;
                }
            }
            tabHit = i;
            realPosition = GetTabRealIndex(tab);
            return;
        }
    }
}

bool clTabCtrlBase::MoveActiveToIndex(int newIndex, eDirection direction)
{
    int activeTabInex = GetSelection();

    // Sanity
    if(newIndex == wxNOT_FOUND) return false;
    if(activeTabInex == wxNOT_FOUND) return false;
    if((newIndex < 0) || (newIndex >= (int)m_tabs.size())) return false;

    bool movingTabRight;
    if(direction == eDirection::kInvalid) {
        movingTabRight = (newIndex > activeTabInex);
    } else if((direction == eDirection::kRight) || (direction == eDirection::kUp)) {
        movingTabRight = true;
    } else {
        movingTabRight = false;
    }

    clTabInfo::Ptr_t movingTab = GetActiveTabInfo();
    clTabInfo::Ptr_t insertBeforeTab = m_tabs[newIndex];

    if(!movingTab) return false;

    // Step 1:
    // Remove the tab from both the active and from the visible tabs array
    clTabInfo::Vec_t::iterator iter = std::find_if(m_visibleTabs.begin(), m_visibleTabs.end(), [&](clTabInfo::Ptr_t t) {
        if(t->Equals(movingTab)) {
            return true;
        }
        return false;
    });

    if(iter != m_visibleTabs.end()) {
        m_visibleTabs.erase(iter);
    }
    iter = std::find_if(m_tabs.begin(), m_tabs.end(), [&](clTabInfo::Ptr_t t) {
        if(t->Equals(movingTab)) {
            return true;
        }
        return false;
    });

    if(iter != m_tabs.end()) {
        m_tabs.erase(iter);
    }

    // Step 2:
    // Insert 'tab' in its new position (in both arrays)
    iter = std::find_if(m_tabs.begin(), m_tabs.end(), [&](clTabInfo::Ptr_t t) {
        if(t->Equals(insertBeforeTab)) {
            return true;
        }
        return false;
    });

    if(movingTabRight) {
        ++iter;
        // inser the new tab _after_
        if(iter != m_tabs.end()) {
            m_tabs.insert(iter, movingTab);
        } else {
            m_tabs.push_back(movingTab);
        }

        iter = std::find_if(m_visibleTabs.begin(), m_visibleTabs.end(), [&](clTabInfo::Ptr_t t) {
            if(t->Equals(insertBeforeTab)) {
                return true;
            }
            return false;
        });
        ++iter;
        if(iter != m_visibleTabs.end()) {
            m_visibleTabs.insert(iter, movingTab);
        } else {
            m_visibleTabs.push_back(movingTab);
        }
    } else {
        if(iter != m_tabs.end()) {
            m_tabs.insert(iter, movingTab);
        }

        iter = std::find_if(m_visibleTabs.begin(), m_visibleTabs.end(), [&](clTabInfo::Ptr_t t) {
            if(t->Equals(insertBeforeTab)) {
                return true;
            }
            return false;
        });
        if(iter != m_visibleTabs.end()) {
            m_visibleTabs.insert(iter, movingTab);
        }
    }
    // Step 3:
    // Update the visible tabs coordinates
    DoUpdateCoordiantes(m_visibleTabs);

    // And finally: Refresh
    Refresh();
    return true;
}

clTabInfo::Ptr_t clTabCtrlBase::GetTabInfo(size_t index) const
{
    if(!IsIndexValid(index)) return clTabInfo::Ptr_t(NULL);
    return m_tabs[index];
}

bool clTabCtrlBase::SetPageToolTip(size_t page, const wxString& tooltip)
{
    clTabInfo::Ptr_t tab = GetTabInfo(page);
    if(tab) {
        tab->SetTooltip(tooltip);
        return true;
    }
    return false;
}

void clTabCtrlBase::SetMenu(wxMenu* menu)
{
    wxDELETE(m_contextMenu);
    m_contextMenu = menu;
}

bool clTabCtrlBase::IsIndexValid(size_t index) const { return (index < m_tabs.size()); }

bool clTabCtrlBase::SetPageText(size_t page, const wxString& text)
{
    clTabInfo::Ptr_t tab = GetTabInfo(page);
    if(!tab) return false;

    int oldWidth = tab->GetWidth();
    tab->SetLabel(text, GetStyle());
    int newWidth = tab->GetWidth();
    int diff = (newWidth - oldWidth);

    // Update the width of the tabs from the current tab by "diff"
    DoUpdateXCoordFromPage(tab, diff);

    // Redraw the tab control
    Refresh();
    return true;
}

void clTabCtrlBase::DoUpdateXCoordFromPage(clTabInfo::Ptr_t tab, int diff)
{
    // Update the coordinates starting from the current tab
    bool foundActiveTab = false;
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(!foundActiveTab && tab->Equals(m_tabs[i])) {
            foundActiveTab = true;
        } else if(foundActiveTab) {
            m_tabs[i]->GetRect().SetX(m_tabs[i]->GetRect().GetX() + diff);
        }
    }
}

bool clTabCtrlBase::ShiftRight(clTabInfo::Vec_t& tabs)
{
    // Move the first tab from the list and adjust the remainder
    // of the tabs x coordiate
    if(!tabs.empty()) {
        clTabInfo::Ptr_t t = tabs.at(0);
        int width = t->GetWidth();
        tabs.erase(tabs.begin() + 0);

        for(size_t i = 0; i < tabs.size(); ++i) {
            clTabInfo::Ptr_t t = tabs.at(i);
            t->GetRect().SetX(t->GetRect().x - width + GetArt()->overlapWidth);
        }
        return true;
    }
    return false;
}

bool clTabCtrlBase::IsActiveTabInList(const clTabInfo::Vec_t& tabs) const
{
    for(size_t i = 0; i < tabs.size(); ++i) {
        if(tabs.at(i)->IsActive()) return true;
    }
    return false;
}

bool clTabCtrlBase::IsActiveTabVisible(const clTabInfo::Vec_t& tabs) const
{
    wxRect clientRect(GetClientRect());
    if((GetStyle() & kNotebook_ShowFileListButton) && !IsVerticalTabs()) {
        clientRect.SetWidth(clientRect.GetWidth() - CHEVRON_SIZE);
    } else if((GetStyle() & kNotebook_ShowFileListButton) && IsVerticalTabs()) {
        // Vertical tabs
        clientRect.SetHeight(clientRect.GetHeight() - CHEVRON_SIZE);
    }

    for(size_t i = 0; i < tabs.size(); ++i) {
        clTabInfo::Ptr_t t = tabs.at(i);
        if(IsVerticalTabs()) {
            if(t->IsActive() && clientRect.Intersects(t->GetRect())) {
                return true;
            }
        } else {
            wxRect tabRect = t->GetRect();
            tabRect.SetWidth(tabRect.GetWidth() *
                             0.5); // The tab does not need to be fully shown, but at least 50% of it
            if(t->IsActive() && clientRect.Contains(tabRect)) {
                return true;
            }
        }
    }
    return false;
}
void clTabCtrlBase::OnEraseBG(wxEraseEvent& e) { wxUnusedVar(e); }

void clTabCtrlBase::OnSize(wxSizeEvent& event)
{
    event.Skip();
    m_visibleTabs.clear();
    Refresh();
}

void clTabCtrlBase::OnLeftDown(wxMouseEvent& event)
{
    event.Skip();
    m_closeButtonClickedIndex = wxNOT_FOUND;

    if((GetStyle() & kNotebook_ShowFileListButton) && m_chevronRect.Contains(event.GetPosition())) {
        // we will handle this later in the "Mouse Up" event
        return;
    }

    int tabHit, realPos;
    eDirection align;
    TestPoint(event.GetPosition(), realPos, tabHit, align);
    if(tabHit == wxNOT_FOUND) return;

    // Did we hit the active tab?
    bool clickWasOnActiveTab = (GetSelection() == realPos);

    // If the click was not on the active tab, set the clicked
    // tab as the new selection and leave this function
    if(!clickWasOnActiveTab) {
        SetSelection(realPos);
    }

    // If we clicked on the active and we have a close button - handle it here
    if((GetStyle() & kNotebook_CloseButtonOnActiveTab) && clickWasOnActiveTab) {
        // we clicked on the selected index
        clTabInfo::Ptr_t t = m_visibleTabs.at(tabHit);
        wxRect xRect(t->GetRect().x + t->GetBmpCloseX(), t->GetRect().y + t->GetBmpCloseY(), 16, 16);
        if(xRect.Contains(event.GetPosition())) {
            m_closeButtonClickedIndex = tabHit;
            return;
        }
    }

    // We clicked on a tab, so prepare to start DnD operation
    if((m_style & kNotebook_AllowDnD)) {
        wxCHECK_RET(!m_dragStartTime.IsValid(), "A leftdown event when Tab DnD was already starting/started");
        m_dragStartTime = wxDateTime::UNow();
        m_dragStartPos = wxPoint(event.GetX(), event.GetY());
    }
}

bool clTabCtrlBase::DoChangeSelection(size_t index)
{
    // sanity
    if(index >= m_tabs.size()) return false;

    int oldSelection = GetSelection();
    /// Do nothing if the tab is already selected
    if(oldSelection == (int)index) {
        ChangeSelection(index);
        return true;
    }

    if(!CanChangePage(oldSelection)) return false;

    // Do the selection change
    ChangeSelection(index);

    // Keep this page
    m_history->Push(GetTabInfo(index)->GetPagePtr());
    NotifyPageChanged(oldSelection, GetSelection());
    return true;
}

bool clTabCtrlBase::SetSelection(size_t tabIdx)
{
    return DoChangeSelection(tabIdx);
}

bool clTabCtrlBase::ChangeSelection(size_t tabIdx)
{
    wxWindowUpdateLocker locker(this);
    //int oldSelection = GetSelection();
    if(!IsIndexValid(tabIdx)) return false;

    for(size_t i = 0; i < m_tabs.size(); ++i) {
        clTabInfo::Ptr_t tab = m_tabs.at(i);
        tab->SetActive((i == tabIdx), GetStyle());
    }
    Refresh();
    return true;
}

bool clTabCtrlBase::InsertPage(size_t index, clTabInfo::Ptr_t tab)
{
    int oldSelection = GetSelection();
    if(index > m_tabs.size()) return false;
    m_tabs.insert(m_tabs.begin() + index, tab);
    bool sendPageChangedEvent = (oldSelection == wxNOT_FOUND) || tab->IsActive();

    BeforePageChange(tab);
    if(sendPageChangedEvent) {
        int tabIndex = index;
        ChangeSelection(tabIndex);
        NotifyPageChanged(oldSelection, GetSelection());
    }

    m_history->Push(tab->GetPagePtr());
    Refresh();
    return true;
}

wxString clTabCtrlBase::GetPageText(size_t page) const
{
    clTabInfo::Ptr_t tab = GetTabInfo(page);
    if(tab) return tab->GetLabel();
    return "";
}

wxBitmap clTabCtrlBase::GetPageBitmap(size_t index) const
{
    clTabInfo::Ptr_t tab = GetTabInfo(index);
    if(tab) return tab->GetBitmap();
    return wxNullBitmap;
}

void clTabCtrlBase::SetPageBitmap(size_t index, const wxBitmap& bmp)
{
    clTabInfo::Ptr_t tab = GetTabInfo(index);
    if(!tab) return;

    // Set the new bitmap and calc the difference
    int oldWidth = tab->GetWidth();
    tab->SetBitmap(bmp, GetStyle());
    int newWidth = tab->GetWidth();
    int diff = (newWidth - oldWidth);

    // Update the width of the tabs from the current tab by "diff"
    DoUpdateXCoordFromPage(tab, diff);

    // Redraw the tab control
    Refresh();
}

void clTabCtrlBase::OnLeftUp(wxMouseEvent& event)
{
    event.Skip();

    m_dragStartTime.Set((time_t)-1); // Not considering D'n'D so reset any saved values
    m_dragStartPos = wxPoint();

    // First check if the chevron was clicked. We do this because the chevron could overlap the buttons drawing
    // area
    if((GetStyle() & kNotebook_ShowFileListButton) && m_chevronRect.Contains(event.GetPosition())) {
        // Show the drop down list
        CallAfter(&clTabCtrlBase::DoShowTabList);

    } else {
        int tabHit, realPos;
        eDirection align;
        TestPoint(event.GetPosition(), realPos, tabHit, align);
        if(tabHit != wxNOT_FOUND) {
            if((GetStyle() & kNotebook_CloseButtonOnActiveTab) && m_visibleTabs.at(tabHit)->IsActive()) {
                // we clicked on the selected index
                clTabInfo::Ptr_t t = m_visibleTabs.at(tabHit);
                wxRect xRect(t->GetRect().x + t->GetBmpCloseX(), t->GetRect().y + t->GetBmpCloseY(), 16, 16);
                xRect.Inflate(2); // don't be picky if we did not click exactly on the 16x16 bitmap...

                if(m_closeButtonClickedIndex == tabHit && xRect.Contains(event.GetPosition())) {
                    if(GetStyle() & kNotebook_CloseButtonOnActiveTabFireEvent) {
                        // let the user process this
                        NotifyPageButtonClicked(realPos);
                    } else {
                        CallAfter(&clTabCtrlBase::DoDeletePage, realPos);
                    }
                }
            }
        }
    }
}

void clTabCtrlBase::DoShowTabList()
{
    if(m_tabs.empty()) return;

    const int curselection = GetSelection();
    wxMenu menu;
    const int firstTabPageID = 13457;
    int pageMenuID = firstTabPageID;

    // Optionally make a sorted view of tabs.
    std::vector<size_t> sortedIndexes(m_tabs.size());
    {
        // std is C++11 at the moment, so no generalized capture.
        size_t index = 0;
        std::generate(sortedIndexes.begin(), sortedIndexes.end(), [&index]() { return index++; });
    }
#if CL_BUILD
    if(EditorConfigST::Get()->GetOptions()->IsSortTabsDropdownAlphabetically()) {
        std::sort(sortedIndexes.begin(), sortedIndexes.end(),
                  [this](size_t i1, size_t i2) { return m_tabs[i1]->m_label.CmpNoCase(m_tabs[i2]->m_label) < 0; });
    }
#endif

    for(auto sortedIndex : sortedIndexes) {
        clTabInfo::Ptr_t tab = m_tabs.at(sortedIndex);
        wxMenuItem* item = new wxMenuItem(&menu, pageMenuID, tab->GetLabel(), "", wxITEM_CHECK);
        menu.Append(item);
        item->Check(tab->IsActive());
        pageMenuID++;
    }

    int selection = GetPopupMenuSelectionFromUser(menu, m_chevronRect.GetBottomLeft());
    if(selection != wxID_NONE) {
        selection -= firstTabPageID;
        if(selection < (int)sortedIndexes.size()) {
            const int newSelection = sortedIndexes[selection];

            // don't change the selection unless the selection is really changing
            if(curselection != newSelection) {
                SetSelection(newSelection);
            }
        }
    }
}

clTabInfo::Ptr_t clTabCtrlBase::GetTabInfoById(size_t id) const
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(m_tabs[i]->GetId() == id) {
            return m_tabs[i];
        }
    }
    return clTabInfo::Ptr_t(nullptr);
}

bool clTabCtrlBase::RemovePage(size_t page, bool notify, bool deletePage)
{
    if(!IsIndexValid(page)) return false;
    bool deletingSelection = ((int)page == GetSelection());

    if(notify && !CanClosePage(page)) return false;

    // Remove the tab from the "all-tabs" list
    clTabInfo::Ptr_t tab = m_tabs.at(page);
    m_tabs.erase(m_tabs.begin() + page);

    // Remove this page from the history
    m_history->Pop(tab->GetPagePtr());

    // Remove the tabs from the visible tabs list
    clTabInfo::Vec_t::iterator iter = std::find_if(m_visibleTabs.begin(), m_visibleTabs.end(), [&](clTabInfo::Ptr_t t) {
        if(t->Equals(tab)) {
            return true;
        }
        return false;
    });

    if(iter != m_visibleTabs.end()) {
        iter = m_visibleTabs.erase(iter);

        for(; iter != m_visibleTabs.end(); ++iter) {
            // update the remainding tabs coordinates
            if(IsVerticalTabs()) {
                (*iter)->GetRect().SetY((*iter)->GetRect().GetY() - tab->GetHeight() + GetArt()->verticalOverlapWidth);
            } else {
                (*iter)->GetRect().SetX((*iter)->GetRect().GetX() - tab->GetWidth() + GetArt()->overlapWidth);
            }
        }
    }

    // Choose a new selection, but only if we are deleting the active tab
    void* nextSelection = nullptr;
    if(deletingSelection) {
        while(!m_history->GetHistory().empty() && (nextSelection == nullptr)) {
            nextSelection = m_history->PrevPage();
            if(!GetTabInfoByPtr(nextSelection)) {
                // The history contains a tab that no longer exists
                m_history->Pop(nextSelection);
                nextSelection = nullptr;
            }
        }
        // It is OK to end up with a null next selection, we will handle it later
    }

    // Let the subclass process the page removal
    PageRemoved(tab, deletePage);

    // Notify
    if(notify) {
        NotifyPageClosed();
    }

    // Choose the next page
    if(deletingSelection) {
        // Always make sure we have something to select...
        if((nextSelection == nullptr) && !m_tabs.empty()) {
            nextSelection = m_tabs[0]->GetPagePtr();
        }

        int nextSel = GetTabIndexByPtr(nextSelection);
        if(nextSel != wxNOT_FOUND) {
            ChangeSelection(nextSel);
            if(notify) {
                NotifyPageChanged(wxNOT_FOUND, GetSelection());
            }
        } else {
            Refresh();
        }
    } else {
        Refresh();
    }
    return true;
}

void clTabCtrlBase::OnMouseMotion(wxMouseEvent& event)
{
    event.Skip();
    int realPos, tabHit;
    wxString curtip = GetToolTipText();
    eDirection align;
    TestPoint(event.GetPosition(), realPos, tabHit, align);
    if(tabHit == wxNOT_FOUND || realPos == wxNOT_FOUND) {
        if(!curtip.IsEmpty()) {
            SetToolTip("");
        }
    } else {
        wxString pagetip = m_tabs.at(realPos)->GetTooltip();
        if(pagetip != curtip) {
            SetToolTip(pagetip);
        }
    }

    if(m_dragStartTime.IsValid()) { // If we're tugging on the tab, consider starting D'n'D
        wxTimeSpan diff = wxDateTime::UNow() - m_dragStartTime;
        if(diff.GetMilliseconds() > 100 && // We need to check both x and y distances as tabs may be vertical
           ((abs(m_dragStartPos.x - event.GetX()) > 10) || (abs(m_dragStartPos.y - event.GetY()) > 10))) {
            OnBeginDrag(); // Sufficient time and distance since the LeftDown for a believable D'n'D start
        }
    }
}

void clTabCtrlBase::OnMouseMiddleClick(wxMouseEvent& event)
{
    event.Skip();
    if(GetStyle() & kNotebook_MouseMiddleClickClosesTab) {
        int realPos, tabHit;
        eDirection align;
        TestPoint(event.GetPosition(), realPos, tabHit, align);
        if(realPos != wxNOT_FOUND) {
            CallAfter(&clTabCtrlBase::DoDeletePage, realPos);
        }
    } else if(GetStyle() & kNotebook_MouseMiddleClickFireEvent) {
        int realPos, tabHit;
        eDirection align;
        TestPoint(event.GetPosition(), realPos, tabHit, align);
        if(realPos != wxNOT_FOUND) {
            // Just fire an event
            NotifyPageButtonClicked(realPos);
        }
    }
}

void clTabCtrlBase::DeleteAllPages()
{
    AllPagesDeleted();
    m_tabs.clear();
    m_visibleTabs.clear();
    m_history->Clear();
    Refresh();
}

int clTabCtrlBase::GetPageIndex(const wxString& label) const
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(m_tabs[i]->GetLabel() == label) return (int)i;
    }
    return wxNOT_FOUND;
}

void clTabCtrlBase::OnContextMenu(wxContextMenuEvent& event)
{
    event.Skip();

    wxPoint pt = ::wxGetMousePosition();
    pt = ScreenToClient(pt);
    int realPos, tabHit;
    eDirection align;
    TestPoint(pt, realPos, tabHit, align);

    if((realPos != wxNOT_FOUND)) {
        // Show context menu for active tabs only
        if(m_contextMenu && (realPos == GetSelection())) {
            PopupMenu(m_contextMenu);
        } else {
            // fire an event for the selected tab
            wxBookCtrlEvent menuEvent(wxEVT_BOOK_TAB_CONTEXT_MENU);
            menuEvent.SetEventObject(GetParent());
            menuEvent.SetSelection(realPos);
            GetParent()->GetEventHandler()->ProcessEvent(menuEvent);
        }
    }
}

void clTabCtrlBase::OnLeftDClick(wxMouseEvent& event)
{
    event.Skip();
    int realPos, tabHit;
    eDirection align;
    TestPoint(event.GetPosition(), realPos, tabHit, align);
    if(tabHit == wxNOT_FOUND) {
        // Fire background d-clicked event
        wxBookCtrlEvent e(wxEVT_BOOK_TABAREA_DCLICKED);
        e.SetEventObject(GetParent());
        GetParent()->GetEventHandler()->AddPendingEvent(e);
    } else {
        // a tab was hit
        wxBookCtrlEvent e(wxEVT_BOOK_TAB_DCLICKED);
        e.SetEventObject(GetParent());
        e.SetSelection(realPos);
        GetParent()->GetEventHandler()->AddPendingEvent(e);
    }
}

bool clTabCtrlBase::ShiftBottom(clTabInfo::Vec_t& tabs)
{
    // Move the first tab from the list and adjust the remainder
    // of the tabs y coordiate
    if(!tabs.empty()) {
        clTabInfo::Ptr_t t = tabs.at(0);
        int height = t->GetHeight();
        tabs.erase(tabs.begin() + 0);

        for(size_t i = 0; i < tabs.size(); ++i) {
            clTabInfo::Ptr_t t = tabs.at(i);
            t->GetRect().SetY(t->GetRect().y - height + GetArt()->verticalOverlapWidth);
        }
        return true;
    }
    return false;
}

void clTabCtrlBase::OnRightUp(wxMouseEvent& event) { event.Skip(); }

void clTabCtrlBase::SetArt(clTabRenderer::Ptr_t art)
{
    m_art = art;
    if((m_style & kNotebook_DarkTabs)) {
        m_colours.InitDarkColours();
    } else {
        m_colours.InitLightColours();
    }
    DoSetBestSize();
    Refresh();
}

void clTabCtrlBase::OnMouseScroll(wxMouseEvent& event)
{
    event.Skip();
    if(GetStyle() & kNotebook_MouseScrollSwitchTabs) {
        size_t curSelection = GetSelection();
        if(event.GetWheelRotation() > 0) {
            if(curSelection > 0) {
                SetSelection(curSelection - 1);
            }
        } else {
            if(curSelection < GetTabs().size()) {
                SetSelection(curSelection + 1);
            }
        }
    }
}

void clTabCtrlBase::DoDrawBottomBox(clTabInfo::Ptr_t activeTab, const wxRect& clientRect, wxDC& dc,
                                    const clTabColours& colours)
{
    GetArt()->DrawBottomRect(this, activeTab, clientRect, dc, colours, GetStyle());
}

bool clTabCtrlBase::CanClosePage(int index)
{
    wxBookCtrlEvent event(wxEVT_BOOK_PAGE_CLOSING);
    event.SetEventObject(GetParent());
    event.SetSelection(index);
    GetParent()->GetEventHandler()->ProcessEvent(event);
    return event.IsAllowed();
}

void clTabCtrlBase::NotifyPageClosed()
{
    wxBookCtrlEvent event(wxEVT_BOOK_PAGE_CLOSED);
    event.SetEventObject(GetParent());
    event.SetSelection(GetSelection());
    GetParent()->GetEventHandler()->ProcessEvent(event);
}

bool clTabCtrlBase::CanChangePage(int index)
{
    wxBookCtrlEvent event(wxEVT_BOOK_PAGE_CHANGING);
    event.SetEventObject(GetParent());
    event.SetSelection(index);
    event.SetOldSelection(wxNOT_FOUND);
    GetParent()->GetEventHandler()->ProcessEvent(event);
    return event.IsAllowed();
}

void clTabCtrlBase::NotifyPageChanged(int oldIndex, int newIndex)
{
    wxBookCtrlEvent event(wxEVT_BOOK_PAGE_CHANGED);
    event.SetEventObject(GetParent());
    event.SetSelection(newIndex);
    event.SetOldSelection(oldIndex);
    GetParent()->GetEventHandler()->AddPendingEvent(event);
}

void clTabCtrlBase::NotifyPageButtonClicked(int index)
{
    wxBookCtrlEvent e(wxEVT_BOOK_PAGE_CLOSE_BUTTON);
    e.SetEventObject(GetParent());
    e.SetSelection(index);
    GetParent()->GetEventHandler()->AddPendingEvent(e);
}

clTabInfo::Ptr_t clTabCtrlBase::GetTabInfoByPtr(void* ptr) const
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(m_tabs[i]->GetPagePtr() == ptr) return m_tabs[i];
    }
    return clTabInfo::Ptr_t(nullptr);
}

int clTabCtrlBase::GetTabIndexByPtr(void* p) const
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(m_tabs[i]->GetPagePtr() == p) return (int)i;
    }
    return wxNOT_FOUND;
}

// ---------------------------------------------------------------------------
// DnD
// ---------------------------------------------------------------------------

clTabCtrlDropTarget::clTabCtrlDropTarget(clTabCtrlBase* tabCtrl)
    : m_tabCtrl(tabCtrl)
{
}

clTabCtrlDropTarget::~clTabCtrlDropTarget() {}

bool clTabCtrlDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    // Extract the content dragged using regular expression
    static wxRegEx re("\\{Class:clTabCtrlBase,TabIndex:([0-9]+)\\}\\{.*?\\}", wxRE_ADVANCED);
    if(!re.Matches(data)) return false;

    wxString tabIndex = re.GetMatch(data, 1);
    long nTabIndex = wxNOT_FOUND;
    tabIndex.ToCLong(&nTabIndex);
    // Sanity
    if(nTabIndex == wxNOT_FOUND) return false;

    // Test the drop tab index
    int realPos, tabHit;
    eDirection align;
    m_tabCtrl->TestPoint(wxPoint(x, y), realPos, tabHit, align);

    // if the tab being dragged and the one we drop it on are the same
    // return false
    if(nTabIndex == realPos) return false;
    m_tabCtrl->MoveActiveToIndex(realPos, align);
    return true;
}
