#ifndef CLSTCEVENTSHANDLER_H
#define CLSTCEVENTSHANDLER_H

#include "clSTCBookCtrl.h"
#include "codelite_exports.h"
#include <wx/event.h>
#include <wx/filename.h>

class wxStyledTextCtrl;
class WXDLLIMPEXP_SDK clSTCEventsHandler : public wxEvtHandler
{
protected:
    // Helper class that keeps track of the editor view state for the current document
    class WXDLLIMPEXP_SDK State
    {
    public:
        int position;
        int firstLineVisible;
        std::vector<std::pair<int, int> > selections;
        State()
            : position(wxNOT_FOUND)
            , firstLineVisible(wxNOT_FOUND)
        {
        }
        void Capture(wxStyledTextCtrl* stc);
        void Restore(wxStyledTextCtrl* stc);
    };

protected:
    clSTCBookCtrl* m_book;
    wxFileName m_fileName;
    void* m_stcDoc;
    bool m_fileLoaded;
    State m_state;
    bool m_eventsBound;

protected:
    /**
     * @brief this method is called whenever the file associated with this handler is set active
     */
    virtual void BindSTCEvents() = 0;

    /**
     * @brief this method is called whenever the file associated with this handler is not active
     */
    virtual void UnbinSTCEvents() = 0;

    /**
     * @brief load the file
     */
    virtual void LoadFile() = 0;

    void CaptureState();
    void RestoreState();

public:
    clSTCEventsHandler(clSTCBookCtrl* book);
    virtual ~clSTCEventsHandler();

    void SetFileName(const wxFileName& filename) { this->m_fileName = filename; }
    const wxFileName& GetFileName() const { return m_fileName; }

    clSTCBookCtrl* GetBook() { return m_book; }
    const clSTCBookCtrl* GetBook() const { return m_book; }

    const wxStyledTextCtrl* GetCtrl() const { return GetBook()->GetCtrl(); }
    wxStyledTextCtrl* GetCtrl() { return GetBook()->GetCtrl(); }

    /**
     * @brief select this document into the view
     */
    void SelectIntoEditor();

    /**
     * @brief Create a new copy, for the same file name. However, it might be attached to a different notebook control
     */
    virtual clSTCEventsHandler* Clone(clSTCBookCtrl* book) = 0;
};

#endif // CLSTCEVENTSHANDLER_H
