#ifndef __EXECHIDDENMSW__
#define __EXECHIDDENMSW__

#include <wx/wx.h>

#ifdef __WXMSW__
long wxExecuteHidden(const wxString& cmd, int flags, wxProcess *handler);
long wxExecuteHidden(wxChar **argv, int flags, wxProcess *handler);
#endif /*__WXMSW__*/

#endif /*__EXECHIDDENMSW__*/
