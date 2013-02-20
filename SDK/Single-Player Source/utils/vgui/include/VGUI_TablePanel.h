//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_TABLEPANEL_H
#define VGUI_TABLEPANEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Dar.h>

namespace vgui
{

class HeaderPanel;

class VGUIAPI TablePanel : public Panel
{
friend class FooVGuiTablePanelHandler;
private:
	vgui::Dar<int> _columnDar;
	bool           _gridVisible[2];
	int            _gridWide;
	int            _gridTall;
	int            _selectedCell[2];
	int            _mouseOverCell[2];
	int            _editableCell[2];
	Panel*         _fakeInputPanel;
	bool           _columnSelectionEnabled;
	bool           _rowSelectionEnabled;
	bool           _cellSelectionEnabled;
	Panel*         _editableCellPanel;
	int            _virtualSize[2];
	bool           _cellEditingEnabled;
public:
	TablePanel(int x,int y,int wide,int tall,int columnCount);
public:
	virtual void   setCellEditingEnabled(bool state);
	virtual void   setColumnCount(int columnCount);
	virtual void   setGridVisible(bool horizontal,bool vertical);
	virtual void   setGridSize(int gridWide,int gridTall);
	virtual int    getColumnCount();
	virtual void   setColumnExtents(int column,int x0,int x1);
	virtual void   setSelectedCell(int column,int row);
	virtual void   getSelectedCell(int& column,int& row);
	virtual void   setHeaderPanel(HeaderPanel* header);
	virtual void   setColumnSelectionEnabled(bool state);
	virtual void   setRowSelectionEnabled(bool state);
	virtual void   setCellSectionEnabled(bool state);
	virtual void   setEditableCell(int column,int row);
	virtual void   stopCellEditing();
	virtual void   getVirtualSize(int& wide,int& tall);
	virtual int    getRowCount()=0;
	virtual int    getCellTall(int row)=0;
	virtual Panel* getCellRenderer(int column,int row,bool columnSelected,bool rowSelected,bool cellSelected)=0;
	virtual Panel* startCellEditing(int column,int row)=0;
protected:
	virtual void   paint();
	virtual Panel* isWithinTraverse(int x,int y);
private:
	virtual void privateMousePressed(MouseCode code,Panel* panel);
	virtual void privateMouseDoublePressed(MouseCode code,Panel* panel);
	virtual void privateKeyTyped(KeyCode code,Panel* panel);
};

}

#endif