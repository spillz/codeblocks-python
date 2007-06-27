#include "il_globals.h"

bool WildCardListMatch(wxString list, wxString name)
{
    if(list==_T("")) //any empty list matches everything by default
        return true;
    wxString wildlist=list;
    wxString wild=list.BeforeFirst(';');
    while(wildlist!=_T(""))
    {
        if(wild!=_T("") && ::wxMatchWild(wild,name))
            return true;
        wildlist=wildlist.AfterFirst(';');
        wild=wildlist.BeforeFirst(';');
    }
    return false;
}

bool PromptSaveOpenFile(wxString message, wxFileName path)
{
    EditorManager* em = Manager::Get()->GetEditorManager();
    EditorBase *eb=em->IsOpen(path.GetFullPath());
    if(eb)
    {
        if(eb->GetModified())
            switch(cbMessageBox(message,_T("Save File?"),wxYES_NO|wxCANCEL))
            {
                case wxYES:
                    if(!eb->Save())
                        cbMessageBox(_("Save failed - proceeding with unsaved file"));
                case wxNO:
                    eb->Close();
                    return true;
                case wxCANCEL:
                    return false;
            }
    }
    return true;
}
