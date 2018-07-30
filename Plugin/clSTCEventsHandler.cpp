#include "clSTCBookCtrl.h"
#include "clSTCEventsHandler.h"
#include <wx/stc/stc.h>

clSTCEventsHandler::clSTCEventsHandler(clSTCBookCtrl* book)
    : m_book(book)
    , m_fileLoaded(false)
    , m_eventsBound(false)
{
    // Document as refcount of 1 now
    m_stcDoc = m_book->GetCtrl()->CreateDocument();
}

clSTCEventsHandler::~clSTCEventsHandler()
{
    m_book->GetCtrl()->ReleaseDocument(m_stcDoc);
    m_stcDoc = nullptr;
    m_book = nullptr;
}

void clSTCEventsHandler::SelectIntoEditor()
{
    wxASSERT(m_stcDoc);
    if(m_book->GetCtrl()->GetClientData()) {
        // Unbind the STC events from the old handler
        clSTCEventsHandler* oldHandler = reinterpret_cast<clSTCEventsHandler*>(m_book->GetCtrl()->GetClientData());
        if(oldHandler) {
            oldHandler->CaptureState();
            if(oldHandler->m_eventsBound) {
                oldHandler->UnbinSTCEvents();
                oldHandler->m_eventsBound = false;
            } else {
                oldHandler->m_eventsBound = false;
            }
        }
    }
    m_book->GetCtrl()->SetDocPointer(m_stcDoc);
    if(!m_fileLoaded) {
        LoadFile(wxConvUTF8);
        m_fileLoaded = true;
    }

    // Document as refcount of 2 now
    BindSTCEvents();
    m_eventsBound = true;

    // Bind STC events to the new handler
    m_book->GetCtrl()->SetClientData(this);
    RestoreState();
}

void clSTCEventsHandler::LoadFile(const wxMBConv& conv)
{
    wxUnusedVar(conv);
    m_book->GetCtrl()->LoadFile(GetFilename().GetFullPath());
}

void clSTCEventsHandler::CaptureState() { m_state.Capture(m_book->GetCtrl()); }

void clSTCEventsHandler::RestoreState() { m_state.Restore(m_book->GetCtrl()); }

void clSTCEventsHandler::State::Capture(wxStyledTextCtrl* stc)
{
    firstLineVisible = stc->GetFirstVisibleLine();
    position = stc->GetCurrentPos();
    selections.clear();
    for(int i = 0; i < stc->GetSelections(); ++i) {
        int start = stc->GetSelectionNStart(i);
        int end = stc->GetSelectionNEnd(i);
        selections.push_back({ start, end });
    }
}

void clSTCEventsHandler::State::Restore(wxStyledTextCtrl* stc)
{
    if(firstLineVisible != wxNOT_FOUND) {
        stc->SetFirstVisibleLine(firstLineVisible);
    }
    if(position != wxNOT_FOUND) {
        stc->SetCurrentPos(position);
    }

    stc->ClearSelections();
    for(size_t i = 0; i < selections.size(); ++i) {
        if(i == 0) {
            stc->SetSelection(selections[i].first, selections[i].second);
        } else {
            stc->AddSelection(selections[i].first, selections[i].second);
        }
    }
}
