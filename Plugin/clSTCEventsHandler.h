#ifndef CLSTCEVENTSHANDLER_H
#define CLSTCEVENTSHANDLER_H

#include "codelite_exports.h"
#include <wx/event.h>
#include <wx/filename.h>

class wxStyledTextCtrl;
class WXDLLIMPEXP_SDK clSTCEventsHandler : public wxEvtHandler
{
    wxStyledTextCtrl* m_stc;
    wxFileName m_filename;
    void* m_stcDoc;

protected:
    void SetDocument(void* doc);
    
    /**
     * @brief this method is called whenever the file associated with this handler is set active
     */
    virtual void BindSTCEvents() = 0;

    /**
     * @brief this method is called whenever the file associated with this handler is not active
     */
    virtual void UnbinSTCEvents() = 0;
    
    wxStyledTextCtrl* GetCtrl() { return m_stc; }
    
public:
    clSTCEventsHandler(wxStyledTextCtrl* stc);
    virtual ~clSTCEventsHandler();

    void SetFilename(const wxFileName& filename) { this->m_filename = filename; }
    const wxFileName& GetFilename() const { return m_filename; }
    
    /**
     * @brief select this document into the view
     */
    void SelectIntoEditor();
};

#endif // CLSTCEVENTSHANDLER_H
