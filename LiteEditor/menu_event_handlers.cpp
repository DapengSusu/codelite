//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// copyright            : (C) 2008 by Eran Ifrah
// file name            : menu_event_handlers.cpp
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
#include "cl_command_event.h"
#include "cl_editor.h"
#include "codelite_events.h"
#include "event_notifier.h"
#include "frame.h"
#include "manager.h"
#include "menu_event_handlers.h"

//------------------------------------
// Handle copy events
//------------------------------------
void EditHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    wxUnusedVar(event);
    clEditor* editor = (clEditor*)owner;

    OptionsConfigPtr options = editor->GetOptions();
    if(event.GetId() == wxID_COPY) {
        if(options->GetCopyLineEmptySelection())
            editor->GetCtrl()->CopyAllowLine();
        else
            editor->GetCtrl()->Copy();

    } else if(event.GetId() == wxID_CUT) {
        editor->GetCtrl()->Cut();

    } else if(event.GetId() == wxID_PASTE) {
        editor->GetCtrl()->Paste();

    } else if(event.GetId() == wxID_UNDO) {
        if(editor->GetCommandsProcessor().CanUndo()) {
            editor->GetCtrl()->Undo();
            editor->GetCommandsProcessor().CloseSciUndoAction();
            editor->GetCommandsProcessor().DecrementCurrentCommand();
        }

    } else if(event.GetId() == wxID_REDO) {
        if(editor->GetCommandsProcessor().CanRedo()) {
            editor->GetCtrl()->Redo();
            editor->GetCommandsProcessor().CloseSciUndoAction(); // Is this necessary? At least it does no harm
            editor->GetCommandsProcessor().IncrementCurrentCommand();
        }

    } else if(event.GetId() == XRCID("label_current_state")) {
        wxString label =
            wxGetTextFromUser("What would you like to call the current state?", "Label current state", "", nullptr);
        if(!label.empty()) { editor->GetCommandsProcessor().SetUserLabel(label); }

    } else if(event.GetId() == wxID_SELECTALL) {
        editor->GetCtrl()->SelectAll();

    } else if(event.GetId() == wxID_DUPLICATE) {
        editor->GetCtrl()->SelectionDuplicate();
    } else if(event.GetId() == XRCID("delete_line_end")) {
        editor->GetCtrl()->DelLineRight();

    } else if(event.GetId() == XRCID("delete_line_start")) {
        editor->GetCtrl()->DelLineLeft();

    } else if(event.GetId() == XRCID("delete_line")) {
        editor->GetCtrl()->LineDelete();

    } else if(event.GetId() == XRCID("copy_line")) {
        editor->GetCtrl()->LineCopy();

    } else if(event.GetId() == XRCID("cut_line")) {
        editor->GetCtrl()->LineCut();

    } else if(event.GetId() == XRCID("trim_trailing")) {
        editor->TrimText(true, false);

    } else if(event.GetId() == XRCID("to_lower")) {
        editor->ChangeCase(true);

    } else if(event.GetId() == XRCID("to_upper")) {
        editor->ChangeCase(false);

    } else if(event.GetId() == XRCID("transpose_lines")) {
        editor->GetCtrl()->LineTranspose();

    } else if(event.GetId() == wxID_DELETE) {
        editor->GetCtrl()->DeleteBack();

    } else if(event.GetId() == XRCID("move_line_down")) {

        int curline = editor->GetCurrentLine();
        int lastline = editor->GetCtrl()->LineFromPosition(editor->GetLength() - 1);

        if(editor->GetSelection().empty() ||
           (editor->LineFromPos(editor->GetSelectionStart() == editor->LineFromPos(editor->GetSelectionEnd())))) {
            // No selection (or only a trivial 1-line one)
            if(curline != lastline) {
                editor->GetCtrl()->LineDown();
                editor->GetCtrl()->LineTranspose();
            }

        } else {
            editor->GetCtrl()->MoveSelectedLinesDown(); // There is a selection, so we can move it direct
        }

    } else if(event.GetId() == XRCID("move_line_up")) {

        if(editor->GetSelection().empty() ||
           (editor->LineFromPos(editor->GetSelectionStart() == editor->LineFromPos(editor->GetSelectionEnd())))) {
            // No selection (or only a trivial 1-line one)
            editor->GetCtrl()->LineTranspose();
            editor->GetCtrl()->LineUp();

        } else {
            editor->GetCtrl()->MoveSelectedLinesUp(); // There is a selection, so we can move it direct
        }

    } else if(event.GetId() == XRCID("center_line_roll")) {
        int here = editor->GetCurrentLine();
        int top = editor->GetCtrl()->GetFirstVisibleLine();
        int count = editor->GetCtrl()->LinesOnScreen();
        int center = top + (count / 2);
        if(here < center) {
            for(int lnIterator = 0; lnIterator < center - here; lnIterator++)
                editor->GetCtrl()->LineScrollUp(); // roll up until we get to center
        } else if(here > center) {
            for(int lnIterator = 0; lnIterator < here - center; lnIterator++)
                editor->GetCtrl()->LineScrollDown(); // roll down until we get to center
        }

    } else if(event.GetId() == XRCID("convert_indent_to_spaces")) {
        editor->ConvertIndentToSpaces();
    } else if(event.GetId() == XRCID("convert_indent_to_tabs")) {
        editor->ConvertIndentToTabs();
    }
}

void EditHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    clEditor* editor = owner;

    if(event.GetId() == wxID_COPY || event.GetId() == XRCID("trim_trailing") || event.GetId() == XRCID("to_lower") ||
       event.GetId() == XRCID("to_upper")) {
        event.Enable(editor);

    } else if(event.GetId() == wxID_CUT) {
        event.Enable(editor);

    } else if(event.GetId() == wxID_PASTE) {
#ifdef __WXGTK__
        event.Enable(editor);
#else
        event.Enable(editor && editor->GetCtrl()->CanPaste());
#endif
    } else if(event.GetId() == wxID_UNDO) {
        event.Enable(editor && editor->GetCtrl()->CanUndo());
    } else if(event.GetId() == wxID_REDO) {
        event.Enable(editor && editor->GetCtrl()->CanRedo());
    } else if(event.GetId() == wxID_SELECTALL) {
        event.Enable(editor && editor->GetLength() > 0);
    } else if(event.GetId() == wxID_DUPLICATE || event.GetId() == wxID_DELETE) {
        event.Enable(true);
    } else {
        event.Enable(false);
    }
    event.Skip(false);
}

//------------------------------------
// brace matching
//------------------------------------
void BraceMatchHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    clEditor* editor = owner;
    if(!editor) { return; }

    if(event.GetId() == XRCID("select_to_brace")) {
        editor->MatchBraceAndSelect(true);
    } else {
        editor->MatchBraceAndSelect(false);
    }
}

void BraceMatchHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    clEditor* editor = owner;
    event.Enable(editor && editor->GetLength() > 0);
}

//------------------------------------
// Find / Replace
//------------------------------------
void FindReplaceHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    clEditor* editor = owner;
    if(editor) {
        if(event.GetId() == wxID_FIND) {
            clMainFrame::Get()->GetMainBook()->ShowQuickBar(editor->GetFirstSelection());

        // } else if(event.GetId() == wxID_REPLACE) {
        //     editor->DoFindAndReplace(true);

        } else if(event.GetId() == XRCID("ID_QUICK_ADD_NEXT")) {
            editor->QuickAddNext();

        } else if(event.GetId() == XRCID("ID_QUICK_FIND_ALL")) {
            editor->QuickFindAll();
        }
    }
}

void FindReplaceHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    wxUnusedVar(owner);
    wxUnusedVar(event);
}

//----------------------------------
// goto linenumber
//----------------------------------

void GotoHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    wxUnusedVar(event);
    clEditor* editor = owner;
    if(!editor) { return; }

    wxString msg;
    msg.Printf(_("Go to line number (1 - %i):"), editor->GetCtrl()->GetLineCount());

    while(1) {
        wxTextEntryDialog dlg(NULL, msg, _("Go To Line"));
        dlg.SetTextValidator(wxFILTER_NUMERIC);

        if(dlg.ShowModal() == wxID_OK) {
            wxString val = dlg.GetValue();
            long line;
            if(!val.ToLong(&line)) {
                wxString err;
                err.Printf(_("'%s' is not a valid line number"), val.GetData());
                wxMessageBox(err, _("Go To Line"), wxOK | wxICON_INFORMATION);
                continue;
            }

            if(line > editor->GetCtrl()->GetLineCount()) {
                wxString err;
                err.Printf(_("Please insert a line number in the range of (1 - %i)"),
                           editor->GetCtrl()->GetLineCount());
                wxMessageBox(err, _("Go To Line"), wxOK | wxICON_INFORMATION);
                continue;
            }

            if(line > 0) {
                editor->CenterLine(line - 1);
                break;
            } else {
                editor->GetCtrl()->GotoLine(0);
                break;
            }
        } else {
            // wxID_CANCEL
            return;
        }
    }
    editor->SetActive();
}

void GotoHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    wxUnusedVar(event);
    wxUnusedVar(owner);
}

//------------------------------------
// Bookmarks
//------------------------------------
void BookmarkHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    clEditor* editor = owner;
    if(!editor) { return; }

    if(event.GetId() == XRCID("toggle_bookmark")) {
        editor->ToggleMarker();
    } else if(event.GetId() == XRCID("next_bookmark")) {
        editor->FindNextMarker();
    } else if(event.GetId() == XRCID("previous_bookmark")) {
        editor->FindPrevMarker();
    } else if(event.GetId() == XRCID("removeall_current_bookmarks")) {
        editor->DelAllMarkers(0); //  0 == only the currently-active type
    } else if(event.GetId() == XRCID("removeall_bookmarks")) {
        editor->DelAllMarkers(-1); // -1 == all types
    }
}

void BookmarkHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    wxUnusedVar(owner);
    wxUnusedVar(event);
}

//------------------------------------
// Go to definition
//------------------------------------
void GotoDefinitionHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    clEditor* editor = owner;
    if(!editor) { return; }
    if(event.GetId() == XRCID("goto_definition")) { editor->GotoDefinition(); }
}

void GotoDefinitionHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    clEditor* editor = owner;
    if(event.GetId() == XRCID("goto_previous_definition")) {
        event.Enable(editor && editor->CanGotoPreviousDefintion());
    } else {
        event.Enable(editor != NULL);
    }
}

//----------------------------------------------------
// Word wrap handler
//----------------------------------------------------

void WordWrapHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    clEditor* editor = owner;
    if(!editor) {
        event.Enable(false);
        return;
    }

    event.Enable(true);
    event.Check(editor->GetCtrl()->GetWrapMode() != wxSTC_WRAP_NONE);
}

void WordWrapHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    clEditor* editor = owner;
    if(!editor) { return; }

    editor->GetCtrl()->SetWrapMode(event.IsChecked() ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
}
//----------------------------------------------------
// Fold handler
//----------------------------------------------------

void FoldHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    wxUnusedVar(event);
    clEditor* editor = owner;
    if(!editor) { return; }

    if(event.GetId() == XRCID("toggle_fold"))
        editor->ToggleCurrentFold();
    else if(event.GetId() == XRCID("fold_all_in_selection"))
        editor->ToggleAllFoldsInSelection();
    else if(event.GetId() == XRCID("fold_topmost_in_selection"))
        editor->ToggleTopmostFoldsInSelection();
    else
        editor->FoldAll();
}

void FoldHandler::ProcessUpdateUIEvent(clEditor* owner,
                                       wxUpdateUIEvent& event) // Used for ToggleAllFoldsInSelection() etc
{
    clEditor* editor = owner;
    if(!editor) {
        event.Enable(false);
        return;
    }

    bool hasSelection = !editor->GetSelection().empty();
    if(hasSelection) {
        // Even if there is a selection, check it's not a trivial amount i.e. spans at least 2 lines
        hasSelection =
            editor->LineFromPos(editor->GetSelectionStart()) != editor->LineFromPos(editor->GetSelectionEnd());
    }
    event.Enable(hasSelection);
}

void DebuggerMenuHandler::ProcessCommandEvent(clEditor* owner, wxCommandEvent& event)
{
    clEditor* editor = owner;
    if(!editor) { return; }

    if(event.GetId() == XRCID("add_breakpoint")) { editor->AddBreakpoint(); }
    if(event.GetId() == XRCID("insert_breakpoint")) { editor->ToggleBreakpoint(); }
    if((event.GetId() == XRCID("insert_temp_breakpoint")) || (event.GetId() == XRCID("insert_disabled_breakpoint")) ||
       (event.GetId() == XRCID("insert_cond_breakpoint"))) {
        editor->AddOtherBreakpointType(event);
    }
    if(event.GetId() == XRCID("delete_breakpoint")) { editor->DelBreakpoint(); }

    if(event.GetId() == XRCID("toggle_breakpoint_enabled_status")) { editor->ToggleBreakpointEnablement(); }

    if(event.GetId() == XRCID("ignore_breakpoint")) { editor->OnIgnoreBreakpoint(); }

    if(event.GetId() == XRCID("edit_breakpoint")) { editor->OnEditBreakpoint(); }

    if(event.GetId() == XRCID("disable_all_breakpoints")) {
        clDebugEvent event(wxEVT_DBG_UI_DISABLE_ALL_BREAKPOINTS);
        EventNotifier::Get()->ProcessEvent(event);
        ManagerST::Get()->GetBreakpointsMgr()->SetAllBreakpointsEnabledState(false);
    }

    if(event.GetId() == XRCID("enable_all_breakpoints")) {
        clDebugEvent event(wxEVT_DBG_UI_ENABLE_ALL_BREAKPOINTS);
        EventNotifier::Get()->ProcessEvent(event);
        ManagerST::Get()->GetBreakpointsMgr()->SetAllBreakpointsEnabledState(true);
    }

    if(event.GetId() == XRCID("delete_all_breakpoints")) {

        // First let the plugins do this thing
        clDebugEvent event(wxEVT_DBG_UI_DELETE_ALL_BREAKPOINTS);
        EventNotifier::Get()->AddPendingEvent(event);

        // Now clear the manager
        ManagerST::Get()->GetBreakpointsMgr()->DelAllBreakpoints();
    }
}

void DebuggerMenuHandler::ProcessUpdateUIEvent(clEditor* owner, wxUpdateUIEvent& event)
{
    wxUnusedVar(owner);
    wxUnusedVar(event);
}
