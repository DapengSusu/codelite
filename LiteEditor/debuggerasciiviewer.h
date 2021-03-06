//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// copyright            : (C) 2013 by Eran Ifrah
// file name            : debuggerasciiviewer.h
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

#ifndef __debuggerasciiviewer__
#define __debuggerasciiviewer__

/**
@file
Subclass of DebuggerAsciiViewerBase, which is generated by wxFormBuilder.
*/

#include "debuggerasciiviewerbase.h"

class IDebugger;

/** Implementing DebuggerAsciiViewerBase */
class DebuggerAsciiViewer : public DebuggerAsciiViewerBase
{
    void OnClearView(wxCommandEvent &e);
    void OnEdit  (wxCommandEvent &e);
    void OnEditUI(wxUpdateUIEvent &e);
    void OnThemeColourChanged(wxCommandEvent &e);
    bool IsFocused();
public:
    /** Constructor */
    DebuggerAsciiViewer( wxWindow* parent     );
    virtual ~DebuggerAsciiViewer();
    void UpdateView    (const wxString &expr, const wxString &value);
};

#endif // __debuggerasciiviewer__
