#include "ColoursAndFontsManager.h"
#include "Notebook.h"
#include "clTabRendererClassic.h"
#include "clTabRendererCurved.h"
#include "clTabRendererSquare.h"
#include "cl_command_event.h"
#include "codelite_events.h"
#include "drawingutils.h"
#include "editor_config.h"
#include "event_notifier.h"
#include "file_logger.h"
#include "globals.h"
#include "imanager.h"
#include "lexer_configuration.h"
#include <algorithm>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/dnd.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/regex.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>
#include <wx/xrc/xh_bmp.h>
#include <wx/xrc/xmlres.h>

extern void Notebook_Init_Bitmaps();

Notebook::Notebook(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style,
                   const wxString& name)
    : wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS | wxTAB_TRAVERSAL, name)
{
    static bool once = false;

    if(!once) {
        // Add PNG and Bitmap handler
        wxImage::AddHandler(new wxPNGHandler);
        wxXmlResource::Get()->AddHandler(new wxBitmapXmlHandler);
        Notebook_Init_Bitmaps();
        once = true;
    }

    m_tabCtrl = new clTabCtrl(this, style);
    m_windows = new WindowStack(this);

    wxBoxSizer* sizer;
    if(IsVerticalTabs()) {
        sizer = new wxBoxSizer(wxHORIZONTAL);
    } else {
        sizer = new wxBoxSizer(wxVERTICAL);
    }

    SetSizer(sizer);

    if(GetStyle() & kNotebook_BottomTabs) {
        sizer->Add(m_windows, 1, wxEXPAND);
        sizer->Add(m_tabCtrl, 0, wxEXPAND);

    } else {
        sizer->Add(m_tabCtrl, 0, wxEXPAND);
        sizer->Add(m_windows, 1, wxEXPAND);
    }
    Layout();
}

Notebook::~Notebook() {}

void Notebook::AddPage(wxWindow* page, const wxString& label, bool selected, const wxBitmap& bmp)
{
    clTabInfo::Ptr_t tab(new clTabInfo(m_tabCtrl, GetStyle(), page, label, bmp));
    tab->SetActive(selected, GetStyle());
    m_tabCtrl->AddPage(tab);
}

void Notebook::DoChangeSelection(wxWindow* page) { m_windows->Select(page); }

bool Notebook::InsertPage(size_t index, wxWindow* page, const wxString& label, bool selected, const wxBitmap& bmp)
{
    clTabInfo::Ptr_t tab(new clTabInfo(m_tabCtrl, GetStyle(), page, label, bmp));
    tab->SetActive(selected, GetStyle());
    return m_tabCtrl->InsertPage(index, tab);
}

void Notebook::SetStyle(size_t style)
{
    m_tabCtrl->SetStyle(style);
    GetSizer()->Detach(m_windows);
    GetSizer()->Detach(m_tabCtrl);
    wxSizer* sz = NULL;

    // Replace the sizer
    if(IsVerticalTabs()) {
        sz = new wxBoxSizer(wxHORIZONTAL);
    } else {
        sz = new wxBoxSizer(wxVERTICAL);
    }

    if((style & kNotebook_BottomTabs) || (style & kNotebook_RightTabs)) {
        sz->Add(m_windows, 1, wxEXPAND);
        sz->Add(m_tabCtrl, 0, wxEXPAND);

    } else {
        sz->Add(m_tabCtrl, 0, wxEXPAND);
        sz->Add(m_windows, 1, wxEXPAND);
    }
    SetSizer(sz);
    Layout();
    m_tabCtrl->Refresh();
}

wxWindow* Notebook::GetCurrentPage() const
{
    if(m_tabCtrl->GetSelection() == wxNOT_FOUND) return NULL;
    return m_tabCtrl->GetPage(m_tabCtrl->GetSelection());
}

int Notebook::FindPage(wxWindow* page) const { return m_tabCtrl->FindPage(page); }

bool Notebook::RemovePage(size_t page, bool notify) { return m_tabCtrl->RemovePage(page, notify, false); }
bool Notebook::DeletePage(size_t page, bool notify) { return m_tabCtrl->RemovePage(page, notify, true); }
bool Notebook::DeleteAllPages() { return m_tabCtrl->DeleteAllPages(); }

void Notebook::EnableStyle(NotebookStyle style, bool enable)
{
    size_t flags = GetStyle();
    if(enable) {
        flags |= style;
    } else {
        flags &= ~style;
    }
    SetStyle(flags);
}

void Notebook::SetTabDirection(wxDirection d)
{
    size_t flags = GetStyle();
    // Clear all direction styles
    flags &= ~kNotebook_BottomTabs;
    flags &= ~kNotebook_LeftTabs;
    flags &= ~kNotebook_RightTabs;
    if(d == wxBOTTOM) {
        flags |= kNotebook_BottomTabs;
    } else if(d == wxRIGHT) {
        flags |= kNotebook_RightTabs;
    } else if(d == wxLEFT) {
        flags |= kNotebook_LeftTabs;
    }
    SetStyle(flags);
}

bool Notebook::MoveActivePage(int newIndex)
{
    return m_tabCtrl->MoveActiveToIndex(newIndex, GetSelection() > newIndex ? eDirection::kLeft : eDirection::kRight);
}
