#ifndef IL_GLOBALS_H
#define IL_GLOBALS_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h>

bool WildCardListMatch(wxString list, wxString name);

bool PromptSaveOpenFile(wxString message, wxFileName path);

inline void LogMessage(const wxString &msg)
{ Manager::Get()->GetMessageManager()->Log(msg); }


#endif //IL_GLOBALS_H
