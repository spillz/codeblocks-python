#ifndef __EXECHIDDENMSW__
#define __EXECHIDDENMSW__


#ifdef __WXMSW__

#include <wx/string.h>

class wxProcess;

long wxExecuteHidden(const wxString& cmd, int flags, wxProcess *handler);
long wxExecuteHidden(wxChar **argv, int flags, wxProcess *handler);
#endif /*__WXMSW__*/

#endif /*__EXECHIDDENMSW__*/
