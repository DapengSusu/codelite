#ifndef CLSTCEVENTSHANDLER_H
#define CLSTCEVENTSHANDLER_H

#include "codelite_exports.h"
#include <wx/event.h>
#include <wx/filename.h>

class wxStyledTextCtrl;
class clSTCBookCtrl;
class WXDLLIMPEXP_SDK clSTCEventsHandler : public wxEvtHandler
{
protected:
    // Helper class that keeps track of the editor view state for the current document
    class State
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

private:
    clSTCBookCtrl* m_book;
    wxFileName m_filename;
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
    void LoadFile(const wxMBConv& conv = wxConvLibc);

    void CaptureState();
    void RestoreState();

public:
    clSTCEventsHandler(clSTCBookCtrl* book);
    virtual ~clSTCEventsHandler();

    void SetFilename(const wxFileName& filename) { this->m_filename = filename; }
    const wxFileName& GetFilename() const { return m_filename; }
    clSTCBookCtrl* GetBook() { return m_book; }

    /**
     * @brief select this document into the view
     */
    void SelectIntoEditor();
};

#endif // CLSTCEVENTSHANDLER_H
