#include "clSTCEventsHandler.h"
#include <wx/stc/stc.h>

clSTCEventsHandler::clSTCEventsHandler(wxStyledTextCtrl* stc)
    : m_stc(stc)
{
    // Document as refcount of 1 now
    m_stcDoc = m_stc->CreateDocument();
}

clSTCEventsHandler::~clSTCEventsHandler() 
{
    m_stc->ReleaseDocument(m_stcDoc);
    m_stcDoc = nullptr;
}

void clSTCEventsHandler::SelectIntoEditor()
{
    m_stc->SetDocPointer(m_stcDoc);
    // Document as refcount of 2 now
    BindSTCEvents();
    if(m_stc->GetClientData()) {
        // Unbind the STC events from the old handler
        clSTCEventsHandler* oldHandler = reinterpret_cast<clSTCEventsHandler*>(m_stc->GetClientData());
        if(oldHandler) { oldHandler->UnbinSTCEvents(); }
    }
    // Bind STC events to the new handler
    m_stc->SetClientData(this);
}
