#ifndef FILE_VIEW_TREE_H
#define FILE_VIEW_TREE_H

#include "wx/treectrl.h" 
#include "project.h"
#include "pluginmanager.h"
#include "imanager.h"
#include "map"

class wxMenu;

struct FileViewItem {
	wxString virtualDir;
	wxString fullpath;
	wxString displayName;
};

/**
 * Class FilewViewTreeItemData, a user defined class which stores a node private information
 *
 * \date 12-04-2007
 * \author Eran 
 *
 */
class FilewViewTreeItemData : public wxTreeItemData
{
	ProjectItem m_item;
public:
	FilewViewTreeItemData(const ProjectItem &item) : m_item(item) { }
	const ProjectItem &GetData() const { return m_item; }
};

class FileViewTree : public wxTreeCtrl
{
	DECLARE_DYNAMIC_CLASS()
	wxMenu *m_folderMenu;
	wxMenu *m_projectMenu;
	wxMenu *m_fileMenu;
	wxMenu *m_workspaceMenu;

	std::map<void*, bool> m_itemsToSort;
	wxTreeItemId m_draggedItem;

public:
	/**
	 * Default cosntructor.
	 */
	FileViewTree();

	/**
	 * Parameterized constructor.
	 * \param parent Tree parent window
	 * \param id Window id
	 * \param pos Window position
	 * \param size Window size
	 * \param style Window style
	 */
	FileViewTree(wxWindow *parent, const wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	
	/**
	 * Destructor .
	 */
	virtual ~FileViewTree(void);	

	/**
	 * Create tree, usually called after constructing FileViewTree with default constructor.
	 * \param parent Tree parent window
	 * \param id Window id
	 * \param pos Window position
	 * \param size Window size
	 * \param style Window style
	 */
	virtual void Create(wxWindow *parent, const wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);

	// Build the actual tree from the workspace
	void BuildTree();
	
	
	/**
	 * \brief return the current selected item information
	 */
	TreeItemInfo GetSelectedItemInfo();
	
	/**
	 * Make sure that fileName under project is visible
	 * \param &project 
	 * \param &fileName 
	 */
	void ExpandToPath(const wxString &project, const wxFileName &fileName);
	
	bool AddFilesToVirtualFodler(wxTreeItemId &item, wxArrayString &paths);
	
protected:
	virtual void OnPopupMenu(wxTreeEvent &event);
	virtual void OnItemActivated(wxTreeEvent &event);
	virtual void OnMouseDblClick(wxMouseEvent &event);
	virtual void OnRemoveProject(wxCommandEvent &event);
	virtual void OnSetActive(wxCommandEvent &event);
	virtual void OnNewItem(wxCommandEvent &event);
	virtual void OnAddExistingItem(wxCommandEvent &event);
	virtual void OnNewVirtualFolder(wxCommandEvent &event);
	virtual void OnProjectProperties(wxCommandEvent &event);
	virtual void OnSortItem(wxCommandEvent &event);
	virtual void OnRemoveVirtualFolder(wxCommandEvent &event);
	virtual void OnRemoveItem(wxCommandEvent &event);
	virtual void OnExportMakefile(wxCommandEvent &event);
	virtual void OnSaveAsTemplate(wxCommandEvent &event);
	virtual void OnBuildOrder(wxCommandEvent &event);
	virtual void OnClean(wxCommandEvent &event);
	virtual void OnBuild(wxCommandEvent &event);
	virtual void OnRunPremakeStep(wxCommandEvent &event);
	virtual void OnBuildProjectOnly(wxCommandEvent &event);
	virtual void OnCleanProjectOnly(wxCommandEvent &event);
	virtual void OnStopBuild(wxCommandEvent &event);
	virtual void OnRetagProject(wxCommandEvent &event);
	virtual void OnRetagWorkspace(wxCommandEvent &event);
	virtual void OnBuildInProgress(wxUpdateUIEvent &event);
	virtual void OnItemBeginDrag(wxTreeEvent &event);
	virtual void OnItemEndDrag(wxTreeEvent &event);
	virtual void OnImportDirectory(wxCommandEvent &e);
	virtual void OnCompileItem(wxCommandEvent &e);
	virtual void SortTree();
	virtual void SortItem(wxTreeItemId &item);
	
	// Tree sorting
	virtual int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2);
	int OnCompareItems(const FilewViewTreeItemData *a, const FilewViewTreeItemData *b);
	
	void PopupContextMenu(wxMenu *menu, MenuType type, const wxString &projectName = wxEmptyString);

private:
	// Build project node
	void BuildProjectNode(const wxString &projectName);
	int GetIconIndex(const ProjectItem &item);
	void ConnectEvents();
	wxString GetItemPath(wxTreeItemId &item);

	void DoRemoveProject(const wxString &name);
	void DoSetProjectActive(wxTreeItemId &item);
	void DoAddVirtualFolder(wxTreeItemId &parent);
	void DoRemoveVirtualFolder(wxTreeItemId &parent);
	void DoRemoveItem(wxTreeItemId &item);
	void DoItemActivated(wxTreeItemId &item, wxEvent &event);
	void DoAddItem(ProjectPtr proj, const FileViewItem &item);
	
	wxTreeItemId GetSingleSelection();
	size_t GetMultiSelection(wxArrayTreeItemIds &arr);
	wxTreeItemId FindItemByPath(wxTreeItemId &parent, const wxString &projectPath, const wxString &fileName);
	DECLARE_EVENT_TABLE();
};

#endif // FILE_VIEW_TREE_H

