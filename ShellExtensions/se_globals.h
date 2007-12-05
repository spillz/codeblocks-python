#ifndef IL_GLOBALS_H
#define IL_GLOBALS_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h>

wxString GetParentDir(const wxString &path);

bool DirIsChildOf(const wxString &path, const wxString &child);

bool WildCardListMatch(wxString list, wxString name);

bool PromptSaveOpenFile(wxString message, wxFileName path);

inline void LogMessage(const wxString &msg)
{ Manager::Get()->GetLogManager()->Log(msg); }

inline int MessageBox(wxWindow *parent, const wxString& message, const wxString& caption = wxEmptyString, int style = wxOK, int x = -1, int y = -1) { return cbMessageBox(message, caption, style, parent, x, y);}

#endif //IL_GLOBALS_H
