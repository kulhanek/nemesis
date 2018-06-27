/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU Affero General Public License version 3 as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/
/* 2012 - modified by kulhanek
 ***************************************************************************/

if (typeof(ui) == 'undefined')
    ui = function () {};

ui.path = '/';
ui.base_url = '';

ui.scale = 40;

ui.zoomValues = [0.5, 1.0, 1.5, 2.0, 2.5, 3.0];
ui.zoomIdx = 1;
ui.zoom = 1.0;

ui.DBLCLICK_INTERVAL = 300;

ui.HISTORY_LENGTH = 8;

ui.DEBUG = false;

ui.render = null;

ui.ctab = new chem.Struct();

ui.client_area = null;
ui.mode_id = null;

ui.undoStack = new Array();
ui.redoStack = new Array();

ui.is_osx = false;
ui.initialized = false;

ui.cursorPos = {pageX: 0, pageY: 0};

ui.MODE = {SIMPLE: 1, ERASE: 2, ATOM: 3, BOND: 4, PATTERN: 5, SGROUP: 6, PASTE: 7, CHARGE: 8, RXN_ARROW: 9, RXN_PLUS: 10, CHAIN: 11};

ui.patterns =
{
    six1: [1, 2, 1, 2, 1, 2],
    six2: [1, 1, 1, 1, 1, 1],
    //sixa: [4, 4, 4, 4, 4, 4],
    five: [1, 1, 1, 1, 1]
};

// kulhanek ui.defaultSelector = 'selector_lasso';
ui.defaultSelector = 'bond_single';

//------------------------------------------------------------------------------
// Init section
//------------------------------------------------------------------------------

ui.init = function ()
{
    if (this.initialized)
    {
        this.Action.fromNewCanvas(new chem.Struct());
        this.render.update();
        this.undoStack.clear();
        this.redoStack.clear();
        this.updateActionButtons();
        this.selectMode(ui.defaultSelector);
        return;
    }

    this.is_osx = (navigator.userAgent.indexOf('Mac OS X') != -1);

    // OS X specific stuff
    if (ui.is_osx)
    {
        $$('.toolButton, .toolButton > img, .sideButton').each(function (button)
        {
            button.title = button.title.replace("Ctrl", "Cmd");
        }, this);
    }

    // Document events
    document.observe('keypress', ui.onKeyPress_Ketcher);
    document.observe('keyup', ui.onKeyUp);
    document.observe(EventMap['mousedown'], ui.onMouseDown_Ketcher);
    document.observe(EventMap['mouseup'], ui.onMouseUp_Ketcher);

    // Button events
    $$('.toolButton').each(ui.initButton);
    $$('.modeButton').each(function (el)
    {
        ui.initButton(el);
        if (el.identify() != 'atom_table')
            el.observe('click', ui.onClick_SideButton);
    });
    $$('.dropdownButton').each(function (el)
    {
        el.observe('click', ui.onClick_DropdownButton);
    });
    $$('.dropdownListItem').each(function (el)
    {
        el.observe(EventMap['mousedown'], ui.onMouseDown_DropdownListItem);
        el.observe('mouseover', function ()
        {
            this.addClassName('highlightedItem');
        });
        el.observe('mouseout', function ()
        {
            this.removeClassName('highlightedItem');
        });
    });

    // menu actions
    $('new').observe('click', ui.onClick_NewFile);
    $('open').observe('click', ui.onClick_OpenFile);
    $('save').observe('click', ui.onClick_SaveFile);
    $('undo').observe('click', ui.onClick_Undo);
    $('redo').observe('click', ui.onClick_Redo);
    $('cut').observe('click', ui.onClick_Cut);
    $('copy').observe('click', ui.onClick_Copy);
    $('paste').observe('click', ui.onClick_Paste);
    $('delete_all').observe('click', ui.onClick_DeleteAll);
    $('show_smiles').observe('click', ui.onClick_ShowSMILES);
    $('zoom_in').observe('click', ui.onClick_ZoomIn);
    $('zoom_out').observe('click', ui.onClick_ZoomOut);
    $('clean_up').observe('click', ui.onClick_CleanUp);
    $('atom_table').observe('click', ui.onClick_ElemTableButton);

    // Client area events
    this.client_area = $('client_area');

    // Dialog events
    $$('.dialogWindow').each(function (el)
    {
        el.observe('keypress', ui.onKeyPress_Dialog);
        el.observe('keyup', ui.onKeyUp);
    });

    // Atom properties dialog events
    $('atom_label').observe('change', ui.onChange_AtomLabel);
    $('atom_charge').observe('change', ui.onChange_AtomCharge);
    $('atom_prop_cancel').observe('click', function ()
    {
        ui.hideDialog('atom_properties');
    });
    $('atom_prop_ok').observe('click', function ()
    {
        ui.applyAtomProperties();
    });
    $('bond_prop_cancel').observe('click', function ()
    {
        ui.hideDialog('bond_properties');
    });
    $('bond_prop_ok').observe('click', function ()
    {
        ui.applyBondProperties();
    });


    // Label input events
    $('input_label').observe('blur', function ()
    {
        this.hide();
    });
    $('input_label').observe('keypress', ui.onKeyPress_InputLabel);
    $('input_label').observe('keyup', ui.onKeyUp);

    // Element table
    $('elem_table_cancel').observe('click', function ()
    {
        ui.elem_table_obj.restore();
        ui.hideDialog('elem_table');
    });
    $('elem_table_ok').observe('click', function (event)
    {
        ui.hideDialog('elem_table');
        ui.onClick_SideButton.apply($('atom_table'), [event]);
    });

    // Save dialog events
    $('file_format').observe('change', ui.onChange_FileFormat);
    $('save_ok').observe('click', function ()
    {
        ui.hideDialog('save_file');
    });

    // resize events
    window.onresize = ui.onResize_Window;
    ui.onResize_Ketcher();

    ui.path = document.location.pathname.substring(0, document.location.pathname.lastIndexOf('/') + 1);
    ui.base_url = document.location.href.substring(0, document.location.href.lastIndexOf('/') + 1);

    // Init renderer
    this.render =  new rnd.Render(this.client_area, ui.scale, {atomColoring: true});
    this.editor = new rnd.Editor(this.render);

    // it must be as it is - it is changed later to default action
    this.selectMode('selector_lasso');

    // events
    this.render.onAtomClick = this.onClick_Atom;
    this.render.onAtomDblClick = this.onDblClick_Atom;
    this.render.onAtomMouseDown = this.onMouseDown_Atom;
    this.render.onAtomMouseOver = this.onMouseOver_Atom;
    this.render.onAtomMouseOut = this.onMouseOut_Atom;

    this.render.onBondClick = this.onClick_Bond;
    this.render.onBondDblClick = this.onDblClick_Bond;
    this.render.onBondMouseDown = this.onMouseDown_Bond;
    this.render.onBondMouseOver = this.onMouseOver_Bond;
    this.render.onBondMouseOut = this.onMouseOut_Bond;

    this.render.onCanvasClick = this.onClick_Canvas;
    this.render.onCanvasMouseMove = this.onMouseMove_Canvas;
    this.render.onCanvasMouseDown = this.onMouseDown_Canvas;
    this.render.onCanvasOffsetChanged = this.onOffsetChanged;

    this.render.setMolecule(this.ctab);
    this.render.update();

    // kulhanek
    this.selectMode(ui.defaultSelector);
    this.initialized = true;
    init_nemesis();
};

// -----------------------------------------------------------------------------
// side bar menus
// -----------------------------------------------------------------------------

ui.onClick_SideButton = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;
    ui.selectMode(this.getAttribute('selid') || this.id);
};

// -----------------------------------------------------------------------------

ui.onClick_DropdownButton = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;
    ui.toggleDropdownList(this.id);
};

// -----------------------------------------------------------------------------

ui.onMouseDown_DropdownListItem = function (event)
{
    ui.selectMode(this.id);
    var dropdown_mode_id = this.id.split('_')[0];
    $(dropdown_mode_id + '_dropdown_list').hide();
    if (ui.mode_id == this.id)
    {
        if ($(dropdown_mode_id).getAttribute('src')) {
            $(dropdown_mode_id).setAttribute('src', this.select('img')[0].getAttribute('src'));
        } else {
            ketcher.showMolfileOpts(dropdown_mode_id, ketcher.templates[ui.mode_id], 20, {
                'autoScale':true,
                'autoScaleMargin':4,
                'hideImplicitHydrogen':true,
                'hideTerminalLabels':true,
                'ignoreMouseEvents':true
            });
        }
        $(dropdown_mode_id).title = this.title;
        $(dropdown_mode_id).setAttribute('selid', ui.mode_id);
    }
    if (event)
    {
        util.stopEventPropagation(event);
        return util.preventDefault(event);
    }
};

// -----------------------------------------------------------------------------

ui.initButton = function (el)
{
    el.observe(EventMap['mousedown'], function ()
    {
        if (this.hasClassName('buttonDisabled'))
            return;
        this.addClassName('buttonPressed');
    });
    el.observe(EventMap['mouseup'], function ()
    {
        this.removeClassName('buttonPressed');
    });
    el.observe('mouseover', function ()
    {
        if (this.hasClassName('buttonDisabled'))
            return;
        this.addClassName('buttonHighlight');

        var status = this.getAttribute('title');
        if (status != null)
            window.status = status;
    });
    el.observe('mouseout', function ()
    {
        this.removeClassName('buttonPressed');
        this.removeClassName('buttonHighlight');
        window.status = '';
    });
};

// -----------------------------------------------------------------------------
// dialog methods
// -----------------------------------------------------------------------------

ui.showDialog = function (name,nocenter)
{
    $('window_cover').style.width = $('ketcher_window').getWidth().toString() + 'px';
    $('window_cover').style.height = $('ketcher_window').getHeight().toString() + 'px';
    $('window_cover').show();
    $(name).show();

    if( ! nocenter ){
        // kulhanek - put dialog into middle of scene
        var cba = $('client_area').getBoundingClientRect();
        var tba = $(name).getBoundingClientRect();
        var dx = cba.left + (cba.right-cba.left - (tba.right-tba.left))/2;
        var dy = cba.top + (cba.bottom-cba.top - (tba.bottom-tba.top))/2;
        $(name).style.left = dx + "px";
        $(name).style.top = dy + "px";
    }
};

// -----------------------------------------------------------------------------

ui.hideDialog = function (name)
{
    $(name).hide();
    $('window_cover').hide();
    $('window_cover').style.width = '0px';
    $('window_cover').style.height = '0px';
};

// -----------------------------------------------------------------------------

ui.onKeyPress_Dialog = function (event)
{
    util.stopEventPropagation(event);
    if (event.keyCode == 27)
    {
        ui.hideDialog(this.id);
        return util.preventDefault(event);
    }
};

// -----------------------------------------------------------------------------

ui.toggleDropdownList = function (name)
{
    var list_id = name + '_list';
    if ($(list_id).visible())
        $(list_id).hide();
    else
    {
        $(list_id).show();
        if ($(list_id).hasClassName('renderFirst'))
        {
            var renderOpts = {
                'autoScale':true,
                'autoScaleMargin':4,
                'hideImplicitHydrogen':true,
                'hideTerminalLabels':true
            };

            $(list_id).select("tr").each(function (item)
            {
                if ($(item.id + '_preview'))
                    ketcher.showMolfileOpts(item.id + '_preview', ketcher.templates[item.id], 20, renderOpts);
            });

            $(list_id).removeClassName('renderFirst');
        }
    }
};

// -----------------------------------------------------------------------------
// events
// -----------------------------------------------------------------------------

ui.onResize_Window = function ()
{
    ui.client_area.style.height = '0px'; // this is neccessary for height shrinking
    document.getElementById('wrapper_win').style.height = window.innerHeight + 'px';
    // FIXME - size hack for Chrome renderer
    document.getElementById('bottom_row').style.height = (window.innerHeight - 11*40) + 'px';
    ui.onResize_Ketcher();
};

// -----------------------------------------------------------------------------

ui.onResize_Ketcher = function ()
{
//    if (Prototype.Browser.IE)
    ui.client_area.style.width = (Element.getWidth(ui.client_area.parentNode) - 2).toString() + 'px';
    ui.client_area.style.height = (Element.getHeight(ui.client_area.parentNode) - 2).toString() + 'px';

    try{
        ui.render.onResize();
        ui.render.update()
        ui.setZoomCentered(null, ui.render.getStructCenter());
    } catch(er){

    }
};

// -----------------------------------------------------------------------------

ui.onMouseDown_Atom = function (event, aid)
{
    ui.hideBlurredControls();

    if (ui.modeType() == ui.MODE.PASTE)
        return false;

    ui.mouse_moved = false;
    ui.drag.atom_id = aid;
    ui.drag.start_pos = {x: event.pageX, y: event.pageY};
    ui.drag.last_pos = {x: event.pageX, y: event.pageY};

    if (ui.selection.atoms.indexOf(aid) == -1)
    {
        if (ui.modeType() == ui.MODE.SIMPLE)
            ui.drag.action = ui.Action.fromAtomPos(aid);
        ui.drag.selection = false;
        ui.updateSelection();
    } else
    {
        if (ui.modeType() == ui.MODE.SIMPLE)
            ui.drag.action = ui.Action.fromSelectedAtomsPos();
        ui.drag.selection = true;
    }
    return true;
};

// -----------------------------------------------------------------------------

ui.onMouseDown_Bond = function (event, bid)
{
    ui.hideBlurredControls();

    if (ui.modeType() == ui.MODE.PASTE)
        return false;

    ui.mouse_moved = false;
    ui.drag.bond_id = bid;
    ui.drag.start_pos = {x: event.pageX, y: event.pageY};
    ui.drag.last_pos = {x: event.pageX, y: event.pageY};

    if (ui.selection.bonds.indexOf(bid) == -1)
    {
        if (ui.modeType() == ui.MODE.SIMPLE)
            ui.drag.action = ui.Action.fromBondPos(bid);
        ui.drag.selection = false;
        ui.updateSelection();
    } else
    {
        if (ui.modeType() == ui.MODE.SIMPLE)
            ui.drag.action = ui.Action.fromSelectedAtomsPos();
        ui.drag.selection = true;
    }
    return true;
};

// -----------------------------------------------------------------------------

ui.onMouseDown_Canvas = function (event)
{
    ui.hideBlurredControls();

    if (ui.modeType() == ui.MODE.PASTE)
    {
        ui.mouse_moved = true; // to avoid further handling of the click
        ui.addUndoAction(ui.Action.fromFragmentAddition(ui.pasted.atoms, ui.pasted.bonds, ui.pasted.sgroups, ui.pasted.rxnArrows, ui.pasted.rxnPluses));
        ui.render.update();
        ui.pasted.atoms.clear();
        ui.pasted.bonds.clear();
        ui.pasted.sgroups.clear();
        ui.pasted.rxnArrows.clear();
        ui.pasted.rxnPluses.clear();
        ui.cancelPaste();
        ui.updateSelection();
        ui.selectMode(ui.defaultSelector);
        return;
    }

    ui.mouse_moved = false;

    var pos = ui.page2obj(event);

    if (pos.x < ui.client_area.clientWidth && pos.y < ui.client_area.clientHeight)
    {
        ui.drag.start_pos = {x: event.pageX, y: event.pageY};
        ui.drag.last_pos = {x: event.pageX, y: event.pageY};
    }

    ui.updateSelection();
};

// -----------------------------------------------------------------------------

ui.onMouseMove_Canvas = function (event)
{
    ui.mouse_moved = true;

    var mode = ui.modeType();

    ui.cursorPos.pageX = event.pageX;
    ui.cursorPos.pageY = event.pageY;

    if (mode == ui.MODE.BOND || mode == ui.MODE.ATOM)
    {
        var type = {type: 1, stereo: chem.Struct.BOND.STEREO.NONE};
        var label = {'label':'C'};

        if (mode == ui.MODE.BOND)
            type = ui.bondType();
        else // mode == ui.MODE.ATOM
            label = ui.atomLabel();

        if (ui.drag.atom_id == null)
            if (ui.drag.start_pos == null || mode == ui.MODE.ATOM)
                return;

        if (mode == ui.MODE.BOND && ui.drag.new_atom_id == -1) // Connect existent atom
            return;

        var pos_cursor = ui.page2obj(event);
        var pos = null;

        if (ui.drag.atom_id != null)
            pos = ui.render.atomGetPos(ui.drag.atom_id);
        else
            pos = ui.page2obj({pageX: ui.drag.start_pos.x, pageY: ui.drag.start_pos.y});

        if (util.Vec2.dist(pos, pos_cursor) < 0.01)
        {
            if (ui.drag.new_atom_id != null)
                return;
            pos_cursor.x += 10, pos_cursor.y += 10; // Hack to avoid return
        }

        var v = util.Vec2.diff(pos_cursor, pos);

        var angle = Math.atan2(v.y, v.x);
        var sign = 1;

        if (angle < 0)
            sign = -1;

        angle = Math.abs(angle);

        var floor = Math.floor(angle / (Math.PI / 12)) * (Math.PI / 12);

        if (angle - floor < Math.PI / 24)
            angle = floor;
        else
            angle = floor + (Math.PI / 12);

        angle *= sign;

        v = new util.Vec2(1, 0);
        v = v.rotate(angle);
        v.add_(pos);

        if (ui.drag.new_atom_id == null)
        {
            var action_ret = null;
            var begin = ui.drag.atom_id;

            if (ui.drag.action != null)
            {
                ui.drag.action.perform();

                if (begin != null && Object.isUndefined(ui.ctab.atoms.get(begin)))
                    begin = null;
            }

            if (begin == null)
            {
                begin = label;
                pos = ui.page2obj({pageX: ui.drag.start_pos.x, pageY: ui.drag.start_pos.y});
            } else
                pos = v;

            action_ret = ui.Action.fromBondAddition(type, begin, label, pos, v);

            ui.drag.action = action_ret[0];
            ui.drag.atom_id = action_ret[1];
            ui.drag.new_atom_id = action_ret[2];
        } else {
            ui.render.atomMove(ui.drag.new_atom_id, v);
        }
    } else if (mode == ui.MODE.CHAIN)
    {
        if (ui.drag.start_pos == null)
            return;

        if (ui.drag.new_atom_id == -1) // Connect existent atom
            return;

        var pos_cursor = ui.page2obj(event);
        var pos = null;

        if (ui.drag.atom_id != null)
            pos = ui.render.atomGetPos(ui.drag.atom_id);
        else
            pos = ui.page2obj({pageX: ui.drag.start_pos.x, pageY: ui.drag.start_pos.y});

        if (util.Vec2.dist(pos, pos_cursor) < 0.01)
        {
            if (ui.drag.new_atom_id != null)
                return;
            pos_cursor.x += 10, pos_cursor.y += 10; // Hack to avoid return
        }

        var v = util.Vec2.diff(pos_cursor, pos);

        var angle = Math.atan2(v.y, v.x);
        var sign = 1;

        if (angle < 0)
            sign = -1;

        angle = Math.abs(angle);

        var floor = Math.floor(angle / (Math.PI / 12)) * (Math.PI / 12);

        if (angle - floor < Math.PI / 24)
            angle = floor;
        else
            angle = floor + (Math.PI / 12);

        angle *= sign;

        var step = 1; // hack, steps should be shorter
        var nSect = Math.ceil(v.length() / step);
        v = v.rotate(angle);
        v.add_(pos);

        var action_ret = null;
        var begin = ui.drag.atom_id;

        if (ui.drag.action != null)
        {
            ui.drag.action.perform();

            if (begin != null && Object.isUndefined(ui.ctab.atoms.get(begin)))
                begin = null;
        }

        if (begin == null) {
            begin = label;
            pos = ui.page2obj({pageX: ui.drag.start_pos.x, pageY: ui.drag.start_pos.y});
        }

        action_ret = ui.Action.fromChain(pos, angle, nSect);

        ui.drag.action = action_ret;

    } else if (mode == ui.MODE.PASTE)
    {
        var cur_pos = new util.Vec2(ui.page2obj(event));
        var delta = util.Vec2.diff(cur_pos, ui.pastedAnchorPos);

        ui.render.multipleMoveRel(ui.pasted, delta);
        ui.pastedAnchorPos = new util.Vec2(cur_pos);
    } else
    {
        if (ui.drag.atom_id == null && ui.drag.bond_id == null && ui.drag.rxnArrow_id == null && ui.drag.rxnPlus_id == null)
        {
            if ((mode == ui.MODE.SIMPLE || mode == ui.MODE.ERASE || mode == ui.MODE.SGROUP) && ui.drag.start_pos != null) // rectangle selection
            {
                var start_pos = ui.page2obj({pageX: ui.drag.start_pos.x, pageY:ui.drag.start_pos.y});
                var cur_pos = ui.page2obj(event);
                var p0 = new util.Vec2(Math.min(start_pos.x, cur_pos.x), Math.min(start_pos.y, cur_pos.y));
                var p1 = new util.Vec2(Math.max(start_pos.x, cur_pos.x), Math.max(start_pos.y, cur_pos.y));
                ui.render.drawSelectionRectangle(p0,p1);
                var sel_list = ui.render.getElementsInRectangle(p0,p1);
                ui.updateSelection(sel_list);
            }
            return;
        }

        if (mode == ui.MODE.ERASE)
            return;

        if (mode == ui.MODE.SIMPLE && ui.drag.new_atom_id == -1) // Merging two atoms
            return;

        var delta = ui.page2obj(event).sub(ui.page2obj({pageX: ui.drag.last_pos.x, pageY: ui.drag.last_pos.y}));

        if (ui.drag.atom_id != null || ui.drag.bond_id != null || ui.drag.rxnArrow_id != null || ui.drag.rxnPlus_id != null)
        {
            // TODO to be handled by SelectTool
            if (ui.drag.selection)
                ui.render.multipleMoveRel(ui.selection, delta);
            else if (ui.drag.atom_id != null)
                ui.render.atomMoveRel(ui.drag.atom_id, delta);
            else if (ui.drag.bond_id != null)
            {
                var bond = ui.ctab.bonds.get(ui.drag.bond_id);
                ui.render.multipleMoveRel({'atoms':[bond.begin, bond.end]}, delta);
            } else if (ui.drag.rxnArrow_id != null) {
                ui.render.rxnArrowMoveRel(ui.drag.rxnArrow_id, delta);
            } else if (ui.drag.rxnPlus_id != null) {
                ui.render.rxnPlusMoveRel(ui.drag.rxnPlus_id, delta);
            }
        }

        ui.drag.last_pos = {x: event.pageX, y: event.pageY};
    }
    ui.render.update();
};

// -----------------------------------------------------------------------------

ui.onMouseDown_Ketcher = function (event)
{
    ui.hideBlurredControls();
    //util.stopEventPropagation(event);
};

// -----------------------------------------------------------------------------

ui.onMouseUp_Ketcher = function (event)
{
    if (ui.modeType() == ui.MODE.ERASE)
        if (ui.selected() && ui.isDrag())
            ui.removeSelected();
    if (ui.modeType() == ui.MODE.SGROUP)
        if (ui.selected() && ui.isDrag())
            ui.showSGroupProperties(null);
    ui.endDrag();
    util.stopEventPropagation(event);
};

// -----------------------------------------------------------------------------
// Hightlighting and joining

ui.onMouseOver_Atom = function (event, aid)
{
    if (!ui.isDrag() && ui.modeType() != ui.MODE.PASTE)
        ui.render.atomSetHighlight(aid, true);
    else if (ui.modeType() == ui.MODE.BOND && ui.drag.atom_id != null && ui.drag.atom_id != aid && ui.drag.new_atom_id != aid)
    {
        if (!Object.isUndefined(ui.render.atomGetNeighbors(ui.drag.atom_id).detect(function (nei)
        {
            return (nei.aid == aid);
        }, this)))
            return true;

        var begin = ui.drag.atom_id;
        var pos = null;

        if (ui.drag.action != null)
        {
            ui.drag.action.perform();

            if (Object.isUndefined(ui.ctab.atoms.get(begin)))
            {
                begin = {label: 'C'};
                pos = ui.page2obj({pageX: ui.drag.start_pos.x, pageY: ui.drag.start_pos.y});
            }
        }

        var action_ret = ui.Action.fromBondAddition(ui.bondType(), begin, aid, pos);
        ui.drag.action = action_ret[0];
        ui.drag.atom_id = action_ret[1];

        ui.render.update();
        ui.drag.new_atom_id = -1; // after update() to avoid mousout

        ui.render.atomSetHighlight(aid, true);
    } else if (ui.modeType() == ui.MODE.SIMPLE && ui.drag.atom_id != null && ui.drag.atom_id != aid &&
        ui.drag.new_atom_id == null && !ui.drag.selection)
    {
        if (ui.drag.action == null)
            throw new Error("action is null");

        ui.drag.action = ui.Action.fromAtomMerge(ui.drag.atom_id, aid);
        ui.drag.atom_id = ui.atomMap.indexOf(ui.drag.atom_id);

        ui.render.update();
        ui.drag.new_atom_id = -1; // after update() to avoid mousout
        ui.render.atomSetHighlight(aid, true);
    }
    return true;
};

// -----------------------------------------------------------------------------

ui.onMouseOut_Atom = function (event, aid)
{
    ui.render.atomSetHighlight(aid, false);
    if (ui.modeType() == ui.MODE.BOND && ui.drag.atom_id != null && ui.drag.atom_id != aid && ui.drag.new_atom_id == -1)
        ui.drag.new_atom_id = null;
    else if (ui.modeType() == ui.MODE.SIMPLE && ui.drag.atom_id != null && ui.drag.atom_id != ui.atomMap.indexOf(aid) && ui.drag.new_atom_id == -1)
    {
        ui.drag.action.perform();
        ui.drag.atom_id = ui.atomMap[ui.drag.atom_id];
        ui.drag.new_atom_id = null;
        ui.render.atomMove(ui.drag.atom_id, ui.page2obj({'pageX':ui.drag.start_pos.x,'pageY':ui.drag.start_pos.y}));
        ui.drag.action = ui.Action.fromAtomPos(ui.drag.atom_id);
        ui.drag.last_pos = Object.clone(ui.drag.start_pos);
    }
    return true;
};

// -----------------------------------------------------------------------------

ui.onMouseOver_Bond = function (event, bid)
{
    if (!ui.isDrag() && ui.modeType() != ui.MODE.PASTE)
        ui.render.bondSetHighlight(bid, true);
    return true;
};

// -----------------------------------------------------------------------------

ui.onMouseOut_Bond = function (event, bid)
{
    ui.render.bondSetHighlight(bid, false);
    return true;
};


// -----------------------------------------------------------------------------
// mode methods
// -----------------------------------------------------------------------------

ui.selectMode = function (mode)
{
    if (mode != null)
    {
        if ($(mode).hasClassName('buttonDisabled'))
            return;

        if ( ui.selected() ) {
            if (mode == 'select_erase') {
                ui.removeSelected();
                return;
            }
            if (mode.startsWith('atom_')) {
                ui.addUndoAction(ui.Action.fromSelectedAtomsAttrs(ui.atomLabel(mode)), true);
                ui.render.update();
                return;
            }

            // kulhanek - clear selection
            ui.updateSelection();

        } else if (mode.startsWith('atom_')) {
            var cAtom = ui.render.findClosestAtom(ui.page2obj(ui.cursorPos));
            if (cAtom) {
                ui.addUndoAction(ui.Action.fromAtomAttrs(cAtom.id, ui.atomLabel(mode)), true);
                ui.render.update();
                return;
            }
        } else if (mode.startsWith('bond_')) {
            var cBond = ui.render.findClosestBond(ui.page2obj(ui.cursorPos));
            if (cBond) {
                ui.addUndoAction(ui.Action.fromBondAttrs(cBond.id, { type: ui.bondType(mode).type, stereo: chem.Struct.BOND.STEREO.NONE }), true);
                ui.render.update();
                return;
            }
        }

        if (ui.mode_id == null) // ui.MODE.PASTE
            ui.cancelPaste();
    }

    if (this.mode_id != null && this.mode_id != mode) {
        var button_id = this.mode_id.split('_')[0];
        var state_button = ($(button_id) && $(button_id).hasClassName('stateButton')) || false;
     
        if (state_button) {
           // kulhanek - why? if (mode && !mode.startsWith(button_id))
                $(button_id).removeClassName('buttonSelected');
        } else
            $(this.mode_id).removeClassName('buttonSelected');
    }

    if (mode == null) {
        this.mode_id = null;
        delete this.render.current_tool;
    } else {
        this.render.current_tool = this.editor.toolFor(mode);
        this.mode_id = mode;

        button_id = this.mode_id.split('_')[0];
        state_button = ($(button_id) && $(button_id).hasClassName('stateButton')) || false;

        if (state_button)
            $(button_id).addClassName('buttonSelected');
        else
            $(this.mode_id).addClassName('buttonSelected');
    }
};

// -----------------------------------------------------------------------------

ui.modeType = function ()
{
    if (ui.mode_id == null)
        return ui.MODE.PASTE;
    if (ui.mode_id == 'select_simple')
        return ui.MODE.SIMPLE;
    if (ui.mode_id == 'select_erase')
        return ui.MODE.ERASE;
    if (ui.mode_id.startsWith('atom_'))
        return ui.MODE.ATOM;
    if (ui.mode_id.startsWith('charge_'))
        return ui.MODE.CHARGE;
    if (ui.mode_id.startsWith('bond_'))
        return ui.MODE.BOND;
    if (ui.mode_id.startsWith('pattern_'))
        return ui.MODE.PATTERN;
    if (ui.mode_id == 'chain')
        return ui.MODE.CHAIN;
};

// -----------------------------------------------------------------------------

ui.bondTypeMap = {
    'single'   : {type: 1, stereo: chem.Struct.BOND.STEREO.NONE},
    'up'       : {type: 1, stereo: chem.Struct.BOND.STEREO.UP},
    'down'     : {type: 1, stereo: chem.Struct.BOND.STEREO.DOWN},
    'double'   : {type: 2, stereo: chem.Struct.BOND.STEREO.NONE},
    'triple'   : {type: 3, stereo: chem.Struct.BOND.STEREO.NONE},
    'aromatic' : {type: 4, stereo: chem.Struct.BOND.STEREO.NONE}
};

// -----------------------------------------------------------------------------

ui.bondType = function (mode)
{
    var type_str;

    if (Object.isUndefined(mode))
        type_str = ui.mode_id.substr(5);
    else
        type_str = mode.substr(5);

    return ui.bondTypeMap[type_str];
};

// -----------------------------------------------------------------------------

ui.atomLabel = function (mode)
{
    var label;

    if (Object.isUndefined(mode))
        label = ui.mode_id.substr(5);
    else
        label = mode.substr(5);

    if (label == 'table')
        return ui.elem_table_obj.getAtomProps();
    if (label == 'any')
        return {'label':'A'};
    else
        return {'label':label.capitalize()};
};

// -----------------------------------------------------------------------------

ui.pattern = function ()
{
    return ui.patterns[ui.mode_id.substr(8)];
};

// -----------------------------------------------------------------------------
// Hot keys
// -----------------------------------------------------------------------------

ui.onKeyPress_Ketcher = function (event)
{
    util.stopEventPropagation(event);

    if ($('window_cover').visible())
        return util.preventDefault(event);

    if (ui.isDrag())
    {
        if (event.keyCode == 27)
        {
            ui.endDrag();
            if (ui.selected())
                ui.updateSelection();
        }
        return util.preventDefault(event);
    }

    //rbalabanov: here we try to handle event using current editor tool
    //BEGIN
    if (ui && ui.render.current_tool) {
        if (ui.render.current_tool.processEvent('OnKeyPress', event)) {
            util.preventDefault(event);
            return;
        }
    }
    //END

    switch (event.keyCode)
    {
    case 46: // Delete
        if (!Prototype.Browser.WebKit && !Prototype.Browser.IE)
            if (ui.selected())
                ui.removeSelected();
        return util.preventDefault(event);
    }
};

// -----------------------------------------------------------------------------

// Button handler specially for Safari and IE
ui.onKeyUp = function (event)
{
    if (!Prototype.Browser.WebKit && !Prototype.Browser.IE)
        return;

    // Esc
    if (Prototype.Browser.WebKit && event.keyCode == 27)
    {
        if (ui.isDrag())
        {
            ui.endDrag();
            if (ui.selected())
                ui.updateSelection();
        } else if (this == document)
        {
            if (!$('window_cover').visible())
            {
                if (ui.modeType() == ui.MODE.PASTE)
                    ui.cancelPaste();
                else if (ui.modeType() == ui.MODE.SIMPLE)
                    ui.updateSelection();
                ui.selectMode(ui.defaultSelector);
            }
        } else if (this.hasClassName('dialogWindow'))
            ui.hideDialog(this.id);
        else
            this.hide();
        util.stopEventPropagation(event);
        return util.preventDefault(event);
    }

    // The rest is for IE
    if (event.keyCode != 46 && Prototype.Browser.WebKit)
        return;

    if (this != document)
        return;

    util.stopEventPropagation(event);

    switch (event.keyCode)
    {
    case 46: // Delete
        if (!ui.isDrag() && ui.selected())
            ui.removeSelected();
        return;
    }
};

// -----------------------------------------------------------------------------

ui.onKeyPress_InputLabel = function (event)
{
    util.stopEventPropagation(event);
    if (event.keyCode == 13)
    {
        this.hide();

        var label = '';
        var charge = 0;
        var value_arr = this.value.toArray();

        if (this.value == '*')
        {
            label = 'A';
        } else if (this.value.match(/^[*][1-9]?[+-]$/i))
        {
            label = 'A';

            if (this.value.length == 2)
                charge = 1;
            else
                charge = parseInt(value_arr[1]);

            if (value_arr[2] == '-')
                charge *= -1;
        } else if (this.value.match(/^[A-Z]{1,2}$/i))
        {
            label = this.value.capitalize();
        } else if (this.value.match(/^[A-Z]{1,2}[0][+-]?$/i))
        {
            if (this.value.match(/^[A-Z]{2}/i))
                label = this.value.substr(0, 2).capitalize();
            else
                label = value_arr[0].capitalize();
        } else if (this.value.match(/^[A-Z]{1,2}[1-9]?[+-]$/i))
        {
            if (this.value.match(/^[A-Z]{2}/i))
                label = this.value.substr(0, 2).capitalize();
            else
                label = value_arr[0].capitalize();

            var match = this.value.match(/[0-9]/i);

            if (match != null)
                charge = parseInt(match[0]);
            else
                charge = 1;

            if (value_arr[this.value.length - 1] == '-')
                charge *= -1;
        }

        if (label == 'A' || label == 'Q' || label == 'X' || label == 'R' || chem.Element.getElementByLabel(label) != null)
        {
            ui.addUndoAction(ui.Action.fromAtomAttrs(this.atom_id, {label: label, charge: charge}), true);
            ui.render.update();
        }
        return util.preventDefault(event);
    }
    if (event.keyCode == 27)
    {
        this.hide();
        return util.preventDefault(event);
    }
};

//------------------------------------------------------------------------------
// molecule methods
//------------------------------------------------------------------------------

ui.updateMolecule = function (mol)
{
    if (typeof(mol) == 'undefined' || mol == null)
        return;

    if (ui.selected())
        ui.updateSelection();

    this.addUndoAction(this.Action.fromNewCanvas(mol));

    ui.showDialog('loading');
    setTimeout(function ()
    {
        try
        {
            ui.render.onResize(); // TODO: this methods should be called in the resize-event handler
            ui.render.update()
            ui.setZoomCentered(null, ui.render.getStructCenter());
        } catch (er)
        {
            alert(er.message);
        } finally
        {
            ui.hideDialog('loading');
        }
    }, 50);
};

//------------------------------------------------------------------------------

ui.parseMolfile = function (molfile)
{
    var lines = molfile.split('\n');

    if (lines.length > 0 && lines[0] == 'Ok.')
        lines.shift();

    try
    {
        return chem.Molfile.parseMolfile(lines);
    } catch (er)
    {
        alert("Error loading molfile.");
        return null;
    }
};

//------------------------------------------------------------------------------

ui.loadMolecule = function (mol_string, force_layout)
{
    // kulhanek
    if( force_layout ){
        var new_string = nemesis.cleanProject(mol_string, function(returnValue) {
                ui.updateMolecule(ui.parseMolfile(returnValue));
                }
            );
    } else {
        ui.updateMolecule(ui.parseMolfile(mol_string));
    }
};

//
// Interactive section
//
ui.mouse_moved = false;
ui.drag =
{
    atom_id:   null,
    bond_id:   null,
    selection: false,
    start_pos: null,
    last_pos:  null,
    new_atom_id: null,
    action:  null
};
ui.selection =
{
    atoms: [],
    bonds: [],
    rxnArrows: [],
    rxnPluses: []
};

ui.page2canvas2 = function (pos)
{
    var offset = ui.client_area.cumulativeOffset();

    return new util.Vec2(pos.pageX - offset.left, pos.pageY - offset.top);
};

ui.page2obj = function (pagePos)
{
    return ui.render.view2obj(ui.page2canvas2(pagePos));
};

ui.scrollPos = function ()
{
    return new util.Vec2(ui.client_area.scrollLeft, ui.client_area.scrollTop);
}

//
// Scrolling
//
ui.scrollLeft = null;
ui.scrollTop = null;

ui.showLabelEditor = function(id) {
    var input_el = $('input_label');

    var offset_client = ui.client_area.cumulativeOffset();
    var atom_pos = ui.render.obj2view(ui.render.atomGetPos(id));
    var offset_atom =
    {
        left: offset_client.left + atom_pos.x,
        top: offset_client.top + atom_pos.y
    };

    var offset = 6 * ui.zoom;
    var d = 0; // TODO: fix/Math.ceil(4 * ui.abl() / 100);

    if (offset > 16)
        offset = 16;

    input_el.atom_id = id;
    input_el.value = ui.render.atomGetAttr(id, 'label');
    input_el.style.fontSize = (offset * 2).toString() + 'px';

    input_el.show();

    var offset_parent = Element.cumulativeOffset(input_el.offsetParent);

    input_el.style.left = (offset_atom.left - offset_parent.left - offset - d).toString() + 'px';
    input_el.style.top = (offset_atom.top - offset_parent.top - offset - d).toString() + 'px';

    input_el.activate();
};

//
// Clicking
//
ui.dbl_click = false;

ui.onClick_Atom = function (event, id)
{
    if (ui.mouse_moved)
        return true;

    if (event.altKey)
    {
        ui.showAtomProperties(id);
        return true;
    }

    ui.dbl_click = false;

    setTimeout(function ()
    {
        if (ui.dbl_click)
            return true;

        switch (ui.modeType())
        {
        case ui.MODE.SIMPLE:

            /* // TODO: Add to selection
            if ((event.metaKey && ui.is_osx) || (event.ctrlKey && !ui.is_osx))
            {
                var idx = ui.selection.atoms.indexOf(id);
                if (idx != -1)
                    ui.selection.atoms.splice(idx, 1);
                else
                    ui.selection.atoms = ui.selection.atoms.concat(id);
                ui.updateSelection({'atoms':ui.selection.atoms, 'bonds':ui.selection.bonds});
                break;
            }
            */

            ui.showLabelEditor(id);
            break;

        case ui.MODE.ERASE:
            ui.addUndoAction(ui.Action.fromAtomDeletion(id));
            ui.render.update();
            break;

        case ui.MODE.ATOM:
            ui.addUndoAction(ui.Action.fromAtomAttrs(id, ui.atomLabel()), true);
            ui.render.update();
            break;

        case ui.MODE.CHARGE:
            var plus = (ui.mode_id == 'charge_plus');
            ui.addUndoAction(ui.Action.fromAtomAttrs(id, {charge: ui.render.atomGetAttr(id, 'charge') - 0 + (plus ? 1 : -1)}), true);
            ui.render.update();
            break;

        case ui.MODE.BOND:
            var atom = ui.atomForNewBond(id);
            ui.addUndoAction(ui.Action.fromBondAddition(ui.bondType(), id, atom.atom, atom.pos)[0]);
            ui.render.update();
            break;

        case ui.MODE.PATTERN:
            ui.addUndoAction(ui.Action.fromPatternOnAtom(id, ui.pattern()), true);
            ui.render.update();
            break;

        case ui.MODE.SGROUP:
            ui.updateSelection({'atoms':[id]});
            ui.showSGroupProperties(null);
            break;
        }
    }, ui.DBLCLICK_INTERVAL);
    return true;
};

ui.onClick_RxnArrow = function (event, id)
{
    if (ui.mouse_moved)
        return true;

    ui.dbl_click = false;

    setTimeout(function ()
    {
        if (ui.dbl_click)
            return true;

        switch (ui.modeType())
        {

        case ui.MODE.ERASE:
            ui.addUndoAction(ui.Action.fromArrowDeletion(id));
            ui.render.update();
            break;
        }
    }, ui.DBLCLICK_INTERVAL);
    return true;
};

ui.onClick_RxnPlus = function (event, id)
{
    if (ui.mouse_moved)
        return true;

    ui.dbl_click = false;

    setTimeout(function ()
    {
        if (ui.dbl_click)
            return true;

        switch (ui.modeType())
        {

        case ui.MODE.ERASE:
            ui.addUndoAction(ui.Action.fromPlusDeletion(id));
            ui.render.update();
            break;
        }
    }, ui.DBLCLICK_INTERVAL);
    return true;
};

ui.onDblClick_Atom = function (event, id)
{
    if (event.altKey)
        return true;

    ui.dbl_click = true;

    if (ui.modeType() != ui.MODE.PASTE)
        ui.showAtomProperties(id);
    return true;
};

ui.bondFlipRequired = function (bond, attrs) {
    return attrs.type == chem.Struct.BOND.TYPE.SINGLE &&
    bond.stereo == chem.Struct.BOND.STEREO.NONE &&
    attrs.stereo != chem.Struct.BOND.STEREO.NONE &&
    ui.ctab.atoms.get(bond.begin).neighbors.length <
    ui.ctab.atoms.get(bond.end).neighbors.length;
};

ui.onClick_Bond = function (event, id)
{
    if (ui.mouse_moved)
        return true;

    if (event.altKey)
    {
        ui.showBondProperties(id);
        return true;
    }

    ui.dbl_click = false;

    setTimeout(function ()
    {
        if (ui.dbl_click)
            return true;

        switch (ui.modeType())
        {
        case ui.MODE.SIMPLE:
        case ui.MODE.ATOM:
            /* // TODO: Add to selection
            if ((event.metaKey && ui.is_osx) || (event.ctrlKey && !ui.is_osx))
            {
                var idx = ui.selection.bonds.indexOf(id);
                if (idx != -1)
                    ui.selection.bonds.splice(idx, 1);
                else
                    ui.selection.bonds = ui.selection.bonds.concat(id);
                ui.updateSelection({'atoms':ui.selection.atoms, 'bonds':ui.selection.bonds});
                break;
            }
            */

            var bond_type = ui.render.bondGetAttr(id, 'type');

            if (bond_type >= 4)
                bond_type = 1;
            else
                bond_type = (bond_type % 3) + 1;

            ui.addUndoAction(ui.Action.fromBondAttrs(id, {type: bond_type}));
            ui.render.update();
            break;

        case ui.MODE.ERASE:
            ui.addUndoAction(ui.Action.fromBondDeletion(id));
            ui.render.update();
            break;

        case ui.MODE.BOND:
            var attrs = Object.clone(ui.bondType());
            var bond = ui.ctab.bonds.get(id);

            if (attrs.stereo != chem.Struct.BOND.STEREO.NONE &&
                bond.type == chem.Struct.BOND.TYPE.SINGLE && attrs.type == chem.Struct.BOND.TYPE.SINGLE &&
                bond.stereo == attrs.stereo)
            {
                ui.addUndoAction(ui.Action.fromBondFlipping(id));
            } else
            {
                var flip = ui.bondFlipRequired(bond, attrs);
                if (bond.type == attrs.type)
                {
                    if (bond.type == chem.Struct.BOND.TYPE.SINGLE)
                    {
                        if (bond.stereo == chem.Struct.BOND.STEREO.NONE && bond.stereo == attrs.stereo)
                        {
                            attrs.type = chem.Struct.BOND.TYPE.DOUBLE;
                        }
                    } else if (bond.type == chem.Struct.BOND.TYPE.DOUBLE)
                    {
                        attrs.type = chem.Struct.BOND.TYPE.TRIPLE;
                    } else if (bond.type == chem.Struct.BOND.TYPE.TRIPLE)
                    {
                        attrs.type = chem.Struct.BOND.TYPE.SINGLE;
                    }
                }
                ui.addUndoAction(ui.Action.fromBondAttrs(id, attrs, flip), true);
            }
            ui.render.update();
            break;

        case ui.MODE.PATTERN:
            ui.addUndoAction(ui.Action.fromPatternOnElement(id, ui.pattern(), false), true);
            ui.render.update();
            break;

        case ui.MODE.SGROUP:
            var bond = ui.ctab.bonds.get(id);

            ui.updateSelection({'atoms':[bond.begin, bond.end], 'bonds':[id]});
            ui.showSGroupProperties(null);
            break;
        }
    }, ui.DBLCLICK_INTERVAL);
    return true;
};

ui.onDblClick_Bond = function (event, id)
{
    if (event.altKey)
        return true;

    ui.dbl_click = true;

    if (ui.modeType() != ui.MODE.PASTE)
        ui.showBondProperties(id);
    return true;
};


ui.onClick_SGroup = function (event, sid)
{
    ui.dbl_click = false;

    setTimeout(function ()
    {
        if (ui.dbl_click)
            return true;

        if (ui.modeType() == ui.MODE.ERASE)
        {
            // remove highlighting
            ui.highlightSGroup(sid, false);

            ui.addUndoAction(ui.Action.fromSgroupDeletion(sid));
            ui.render.update();
        }
    }, ui.DBLCLICK_INTERVAL);

    return true;
};

ui.onDblClick_SGroup = function (event, sid)
{
    ui.dbl_click = true;

    if (ui.modeType() != ui.MODE.PASTE)
    {
        if (ui.selected())
            ui.updateSelection();
        ui.showSGroupProperties(sid);
    }

    return true;
};

ui.onClick_Canvas = function (event)
{
    if (ui.mouse_moved)
        return;

    var pos = ui.page2obj(event);
    switch (ui.modeType())
    {
    case ui.MODE.ATOM:

        ui.addUndoAction(ui.Action.fromAtomAddition(pos, ui.atomLabel()));
        ui.render.update();
        break;

    case ui.MODE.BOND:
        var bond = ui.bondType();
        var v = new util.Vec2(1.0 / 2, 0);

        if (bond.type == chem.Struct.BOND.TYPE.SINGLE)
            v = v.rotate(-Math.PI / 6);

        ui.addUndoAction(ui.Action.fromBondAddition(bond, {label: 'C'}, {label: 'C'}, {x: pos.x - v.x, y: pos.y - v.y}, {x: pos.x + v.x, y: pos.y + v.y})[0]);
        ui.render.update();
        break;

    case ui.MODE.PATTERN:

        ui.addUndoAction(ui.Action.fromPatternOnCanvas(pos, ui.pattern()));
        ui.render.update();
        break;

    case ui.MODE.RXN_ARROW:

        ui.addUndoAction(ui.Action.fromArrowAddition(pos));
        ui.render.update();
        break;

    case ui.MODE.RXN_PLUS:

        ui.addUndoAction(ui.Action.fromPlusAddition(pos));
        ui.render.update();
        break;

    case ui.MODE.PASTE:
        ui.addUndoAction(ui.Action.fromFragmentAddition(ui.pasted.atoms, ui.pasted.bonds, ui.pasted.sgroups, ui.pasted.rxnArrows, ui.pasted.rxnPluses));
        ui.render.update();
        ui.pasted.atoms.clear();
        ui.pasted.bonds.clear();
        ui.pasted.sgroups.clear();
        ui.pasted.rxnArrows.clear();
        ui.pasted.rxnPluses.clear();
        ui.selectMode(ui.defaultSelector);
        break;
    }
};

// Get new atom id/label and pos for bond being added to existing atom
ui.atomForNewBond = function (id)
{
    var neighbours = new Array();
    var pos = ui.render.atomGetPos(id);

    ui.render.atomGetNeighbors(id).each(function (nei)
    {
        var nei_pos = ui.render.atomGetPos(nei.aid);

        if (util.Vec2.dist(pos, nei_pos) < 0.1)
            return;

        neighbours.push({id: nei.aid, v: util.Vec2.diff(nei_pos, pos)});
    });

    neighbours.sort(function (nei1, nei2)
    {
        return Math.atan2(nei1.v.y, nei1.v.x) - Math.atan2(nei2.v.y, nei2.v.x);
    });

    var i, max_i = 0;
    var angle, max_angle = 0;

    // TODO: impove layout: tree, ...

    for (i = 0; i < neighbours.length; i++)
    {
        angle = util.Vec2.angle(neighbours[i].v, neighbours[(i + 1) % neighbours.length].v);

        if (angle < 0)
            angle += 2 * Math.PI;

        if (angle > max_angle)
            max_i = i, max_angle = angle;
    }

    var v = new util.Vec2(1, 0);

    if (neighbours.length > 0)
    {
        if (neighbours.length == 1)
        {
            max_angle = -(4 * Math.PI / 3);

            // zig-zag
            var nei = ui.render.atomGetNeighbors(id)[0];
            if (ui.render.atomGetDegree(nei.aid) > 1)
            {
                var nei_neighbours = new Array();
                var nei_pos = ui.render.atomGetPos(nei.aid);
                var nei_v = util.Vec2.diff(pos, nei_pos);
                var nei_angle = Math.atan2(nei_v.y, nei_v.x);

                ui.render.atomGetNeighbors(nei.aid).each(function (nei_nei)
                {
                    var nei_nei_pos = ui.render.atomGetPos(nei_nei.aid);

                    if (nei_nei.bid == nei.bid || util.Vec2.dist(nei_pos, nei_nei_pos) < 0.1)
                        return;

                    var v_diff = util.Vec2.diff(nei_nei_pos, nei_pos);
                    var ang = Math.atan2(v_diff.y, v_diff.x) - nei_angle;

                    if (ang < 0)
                        ang += 2 * Math.PI;

                    nei_neighbours.push(ang);
                });
                nei_neighbours.sort(function (nei1, nei2)
                {
                    return nei1 - nei2;
                });

                if (nei_neighbours[0] <= Math.PI * 1.01 && nei_neighbours[nei_neighbours.length-1] <= 1.01 * Math.PI)
                    max_angle *= -1;

            }
        }

        angle = (max_angle / 2) + Math.atan2(neighbours[max_i].v.y, neighbours[max_i].v.x);

        v = v.rotate(angle);
    }

    v.add_(pos);

    var a = ui.render.findClosestAtom(v, 0.1);

    if (a == null)
        a = {label: 'C'};
    else
        a = a.id;

    return {atom: a, pos: v};
};

//
// Canvas size
//
ui.onOffsetChanged = function (newOffset, oldOffset)
{
    if (oldOffset == null)
        return;

    var delta = new util.Vec2(newOffset.x - oldOffset.x, newOffset.y - oldOffset.y);

    ui.client_area.scrollLeft += delta.x;
    ui.client_area.scrollTop += delta.y;
};

//
// Dragging
//
ui.endDrag = function ()
{
    if (ui.drag.action != null)
    {
        ui.addUndoAction(ui.drag.action);

        if (ui.modeType() == ui.MODE.SIMPLE)
            ui.removeDummyAction();
    }

    ui.drag.atom_id = null;
    ui.drag.bond_id = null;
    ui.drag.rxnArrow_id = null;
    ui.drag.rxnPlus_id = null;

    ui.drag.selection = false;

    ui.drag.new_atom_id = null;

    ui.drag.start_pos = null;
    ui.drag.last_pos = null;

    ui.drag.action = null;

    ui.render.drawSelectionRectangle(null);
    ui.updateClipboardButtons();
};

ui.isDrag = function ()
{
    return ui.drag.start_pos != null;
};

ui.updateSelection = function (selection, nodraw)
{
    selection = selection || {};
    for (var map in rnd.ReStruct.maps) {
        if (Object.isUndefined(selection[map]))
            ui.selection[map] = [];
        else
            ui.selection[map] = selection[map];
    }

    ui.selection.bonds = ui.selection.bonds.filter(function (bid)
    {
        var bond = ui.ctab.bonds.get(bid);
        return (ui.selection.atoms.indexOf(bond.begin) != -1 && ui.selection.atoms.indexOf(bond.end) != -1);
    });

    if (!nodraw) {
        ui.render.setSelection(ui.selection);
        ui.render.update();
    }

    ui.updateClipboardButtons();
};

ui.selected = function ()
{
    for (var map in rnd.ReStruct.maps) {
        if (!Object.isUndefined(ui.selection[map]) && ui.selection[map].length > 0) {
            return true;
        }
    }
    return false;
};

ui.selectAll = function ()
{
    var mode = ui.modeType();
    if (mode == ui.MODE.ERASE || mode == ui.MODE.SGROUP)
        ui.selectMode(ui.defaultSelector);

    var selection = {};
    for (var map in rnd.ReStruct.maps) {
        selection[map] = ui.ctab[map].ikeys();
    }

    ui.updateSelection(selection);
    // kulhanek - to fix atom movement
    ui.editor._selectionHelper.setSelection(selection);
};

ui.clearSelection = function ()
{
    var selection = {};
    ui.updateSelection(selection);
    // kulhanek - to fix atom movement
    ui.editor._selectionHelper.setSelection(selection);
};

ui.removeSelected = function ()
{
    ui.addUndoAction(ui.Action.fromFragmentDeletion());
    for (var map in rnd.ReStruct.maps)
        ui.selection[map] = [];
    ui.render.update();
    ui.updateClipboardButtons();
};

ui.hideBlurredControls = function ()
{
    var ret = false;
    [
        'input_label',
        'selector_dropdown_list',
        'bond_dropdown_list',
        'template_dropdown_list'
    ].each(
        function(el) { el = $(el); if (el.visible()) { el.hide(); ret = true; }}
    );
    return ret;
};

// -----------------------------------------------------------------------------
// Atom properties dialog
// -----------------------------------------------------------------------------

ui.showAtomProperties = function (id)
{
    $('atom_properties').atom_id = id;
    $('atom_label').value = ui.render.atomGetAttr(id, 'label');
    ui.onChange_AtomLabel.call($('atom_label'));
    var value = ui.render.atomGetAttr(id, 'charge');
    $('atom_charge').value = (value == 0 ? '' : value);
    $('atom_radical').value = ui.render.atomGetAttr(id, 'radical');

    ui.showDialog('atom_properties');
    $('atom_label').activate();
};

// -----------------------------------------------------------------------------

ui.applyAtomProperties = function ()
{
    ui.hideDialog('atom_properties');

    var id = $('atom_properties').atom_id;

    ui.addUndoAction(ui.Action.fromAtomAttrs(id,
    {
        label: $('atom_label').value,
        charge: $('atom_charge').value == '' ? 0 : parseInt($('atom_charge').value),
        radical: parseInt($('atom_radical').value),
    }), true);

    ui.render.update();
};

// -----------------------------------------------------------------------------

ui.onChange_AtomLabel = function ()
{
    this.value = this.value.strip().capitalize();

    var element = chem.Element.getElementByLabel(this.value);

    if (element == null && this.value != 'A' && this.value != '*' && this.value != 'Q' && this.value != 'X' && this.value != 'R')
    {
        this.value = ui.render.atomGetAttr($('atom_properties').atom_id, 'label');

        if (this.value != 'A' && this.value != '*')
            element = chem.Element.getElementByLabel(this.value);
    }

    if (this.value == 'A' || this.value == '*')
        $('atom_number').value = "any";
    else if (!element)
        $('atom_number').value = "";
    else
        $('atom_number').value = element.toString();
};

// -----------------------------------------------------------------------------

ui.onChange_AtomCharge = function ()
{
    if (this.value.strip() == '' || this.value == '0')
        this.value = '';
    else if (!this.value.match(/^[+-]?[1-9][0-9]{0,1}$/))
        this.value = ui.render.atomGetAttr($('atom_properties').atom_id, 'charge');
};

// -----------------------------------------------------------------------------
// Bond properties dialog
// -----------------------------------------------------------------------------

ui.showBondProperties = function (id)
{
    $('bond_properties').bond_id = id;

    var type = ui.render.bondGetAttr(id, 'type');
    var stereo = ui.render.bondGetAttr(id, 'stereo');

    for (var bond in ui.bondTypeMap)
    {
        if (ui.bondTypeMap[bond].type == type && ui.bondTypeMap[bond].stereo == stereo)
            break;
    }

    $('bond_type').value = bond;

    ui.showDialog('bond_properties');
    $('bond_type').activate();
};

// -----------------------------------------------------------------------------

ui.applyBondProperties = function ()
{
    ui.hideDialog('bond_properties');

    var id = $('bond_properties').bond_id;
    var bond = Object.clone(ui.bondTypeMap[$('bond_type').value]);
    
    ui.addUndoAction(ui.Action.fromBondAttrs(id, bond), true);

    ui.render.update();
};

// -----------------------------------------------------------------------------
// Element table
// -----------------------------------------------------------------------------

ui.onClick_ElemTableButton = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;
    if (ui.modeType() == ui.MODE.PASTE)
    {
        ui.cancelPaste();
        ui.selectMode(ui.defaultSelector);
    }
    ui.showElemTable();
};

// -----------------------------------------------------------------------------

ui.showElemTable = function ()
{
    if ($('elem_table').visible())
        return;

    ui.showDialog('elem_table',true);
    if (typeof(ui.elem_table_obj) == 'undefined') {
        ui.elem_table_obj = new rnd.ElementTable('elem_table_area', {
            'fillColor':'#DADADA',
            'fillColorSelected':'#FFFFFF',
            'frameColor':'#E8E8E8',
            'fontSize':23,
            'buttonHalfSize':18
        }, true);
        ui.elem_table_area = ui.elem_table_obj.renderTable();
    }
    ui.elem_table_obj.store();
    $('elem_table_ok').focus();

    // kulhanek - put dialog into middle of scene
    var cba = $('client_area').getBoundingClientRect();
    var tba = $('elem_table').getBoundingClientRect();
    var dx = cba.left + (cba.right-cba.left - (tba.right-tba.left))/2;
    var dy = cba.top + (cba.bottom-cba.top - (tba.bottom-tba.top))/2;
    $('elem_table').style.left = dx + "px";
    $('elem_table').style.top = dy + "px";
};

//------------------------------------------------------------------------------
// save smiles dialog
//------------------------------------------------------------------------------

ui.onChange_FileFormat = function (event, update)
{
    var output = $('output_mol');
    var el = $('file_format');

    if (update)
    {
        var saver = new chem.MolfileSaver();
        output.molfile = saver.saveMolecule(ui.ctab, true);

        try
        {
            saver = new chem.SmilesSaver();
            output.smiles = saver.saveMolecule(ui.ctab, true);
        } catch (er)
        {
            output.smiles = er.message;
        }
    }

    if (el.value == 'mol')
    {
        output.value = output.molfile;
        output.style.wordWrap = 'normal';
    } else // if (el.value == 'smi')
    {
        output.value = output.smiles;
        output.style.wordWrap = 'break-word';
    }

    $('mol_data').value = el.value + '\n' + output.value;
    output.activate();
};

//------------------------------------------------------------------------------
// Clipboard actions
//------------------------------------------------------------------------------

ui.clipboard = null;
ui.pasted = {atoms: [], bonds: [], sgroups: [], rxnArrows: [], rxnPluses: []}; // ids
ui.pastedAnchorPos = null;

//------------------------------------------------------------------------------

ui.isClipboardEmpty = function ()
{
    return ui.clipboard == null;
};

//------------------------------------------------------------------------------

ui.updateClipboardButtons = function ()
{
    if (ui.isClipboardEmpty())
        $('paste').addClassName('buttonDisabled');
    else
        $('paste').removeClassName('buttonDisabled');

    if (ui.selected())
    {
        $('copy').removeClassName('buttonDisabled');
        $('cut').removeClassName('buttonDisabled');
    } else
    {
        $('copy').addClassName('buttonDisabled');
        $('cut').addClassName('buttonDisabled');
    }

    // kulhanek
    nemesis.updateMenu("changed");
};

//------------------------------------------------------------------------------

ui.copy = function ()
{
    ui.clipboard =
    {
        atoms: new Array(),
        bonds: new Array(),
        sgroups: new Array(),
        rxnArrows: new Array(),
        rxnPluses: new Array()
    };

    var mapping = {};

    ui.selection.atoms.each(function (id)
    {
        var new_atom = new chem.Struct.Atom(ui.ctab.atoms.get(id));
        new_atom.pos = ui.render.atomGetPos(id);

        if (new_atom.sgroup != -1)
            new_atom.sgroup = -1;

        mapping[id] = ui.clipboard.atoms.push(new chem.Struct.Atom(new_atom)) - 1;
    });

    ui.selection.bonds.each(function (id)
    {
        var new_bond = new chem.Struct.Bond(ui.ctab.bonds.get(id));
        new_bond.begin = mapping[new_bond.begin];
        new_bond.end = mapping[new_bond.end];
        ui.clipboard.bonds.push(new chem.Struct.Bond(new_bond));
    });

    var sgroup_counts = new Hash();

    // determine selected sgroups
    ui.selection.atoms.each(function (id)
    {
        var sg = ui.render.atomGetSGroups(id);

        sg.each(function (sid)
        {
            var n = sgroup_counts.get(sid);
            if (Object.isUndefined(n))
                n = 1;
            else
                n++;
            sgroup_counts.set(sid, n);
        }, this);
    }, this);

    sgroup_counts.each(function (sg)
    {
        var sid = parseInt(sg.key);

        if (sg.value == ui.render.sGroupGetAtoms(sid).length)
        {
            var new_sgroup =
            {
                type: ui.render.sGroupGetType(sid),
                mul: ui.render.sGroupGetAttr(sid, 'mul'),
                connectivity: ui.render.sGroupGetAttr(sid, 'connectivity'),
                name: ui.render.sGroupGetAttr(sid, 'name'),
                subscript: ui.render.sGroupGetAttr(sid, 'subscript'),
                fieldName: ui.render.sGroupGetAttr(sid, 'fieldName'),
                fieldValue: ui.render.sGroupGetAttr(sid, 'fieldValue'),
                atoms: ui.render.sGroupGetAtoms(sid).clone()
            };

            for (var i = 0; i < new_sgroup.atoms.length; i++)
            {
                new_sgroup.atoms[i] = mapping[new_sgroup.atoms[i]];
            }

            ui.clipboard.sgroups.push(new_sgroup);
        }
    });

    ui.selection.rxnArrows.each(function (id)
    {
        var arrow = new chem.Struct.RxnArrow(ui.ctab.rxnArrows.get(id));
        arrow.pos = ui.render.rxnArrowGetPos(id);
        ui.clipboard.rxnArrows.push(arrow);
    });

    ui.selection.rxnPluses.each(function (id)
    {
        var plus = new chem.Struct.RxnPlus(ui.ctab.rxnPluses.get(id));
        plus.pos = ui.render.rxnPlusGetPos(id);
        ui.clipboard.rxnPluses.push(plus);
    });

};

//------------------------------------------------------------------------------

ui.paste = function ()
{
    var mapping = {};
    var id;

    for (id = 0; id < ui.clipboard.atoms.length; id++)
    {
        var atom = ui.clipboard.atoms[id];
        mapping[id] = ui.render.atomAdd(atom.pos, atom);
        ui.pasted.atoms.push(mapping[id]);
    }

    for (id = 0; id < ui.clipboard.bonds.length; id++)
    {
        var bond = ui.clipboard.bonds[id];
        ui.pasted.bonds.push(ui.render.bondAdd(mapping[bond.begin], mapping[bond.end], bond));
    }

    ui.clipboard.sgroups.each(function (sgroup)
    {
        var sid = ui.render.sGroupCreate(sgroup.type);

        ui.render.sGroupSetAttr(sid, 'mul', sgroup.mul);
        ui.render.sGroupSetAttr(sid, 'connectivity', sgroup.connectivity);
        ui.render.sGroupSetAttr(sid, 'name', sgroup.name);
        ui.render.sGroupSetAttr(sid, 'subscript', sgroup.subscript);
        ui.render.sGroupSetAttr(sid, 'fieldName', sgroup.fieldName);
        ui.render.sGroupSetAttr(sid, 'fieldValue', sgroup.fieldValue);

        sgroup.atoms.each(function(id)
        {
            ui.render.atomClearSGroups(mapping[id]);
            ui.render.atomAddToSGroup(mapping[id], sid);
        }, this);

        ui.pasted.sgroups.push(sid);
    }, this);

    for (id = 0; id < ui.clipboard.rxnArrows.length; id++) {
        var arrow = ui.clipboard.rxnArrows[id];
        ui.pasted.rxnArrows.push(ui.render.rxnArrowAdd(arrow.pos, arrow));
    }

    for (id = 0; id < ui.clipboard.rxnPluses.length; id++) {
        var plus = ui.clipboard.rxnPluses[id];
        ui.pasted.rxnPluses.push(ui.render.rxnPlusAdd(plus.pos, plus));
    }

    ui.pastedAnchorPos = null;
    if (ui.pasted.atoms.length) {
        ui.pastedAnchorPos = ui.render.atomGetPos(ui.pasted.atoms[0]);
    } else if (ui.pasted.rxnArrows.length) {
        ui.pastedAnchorPos = ui.render.rxnArrowGetPos(ui.pasted.rxnArrows[0]);
    } else if (ui.pasted.rxnPluses.length) {
        ui.pastedAnchorPos = ui.render.rxnPlusGetPos(ui.pasted.rxnPluses[0]);
    }

    ui.selectMode(null);
    ui.render.update();
};

//------------------------------------------------------------------------------

ui.cancelPaste = function ()
{
    ui.pasted.sgroups.each(function (id)
    {
        ui.render.sGroupDelete(id);
    });

    ui.pasted.atoms.each(function (id)
    {
        ui.render.atomRemove(id);
    });

    ui.pasted.atoms.clear();
    ui.pasted.bonds.clear();
    ui.pasted.sgroups.clear();

    if (ui.render != null)
        ui.render.update();

    ui.clipboard = null;
};

//------------------------------------------------------------------------------
// zoom methods
//------------------------------------------------------------------------------

ui.setZoomRegular = function (zoom) {
    //mr: prevdent unbounded zooming
    //begin
    if (zoom < 0.1 || zoom > 10)
        return;
    //end
    ui.zoom = zoom;
    ui.render.setZoom(ui.zoom);
    // when scaling the canvas down it may happen that the scaled canvas is smaller than the view window
    // don't forget to call setScrollOffset after zooming (or use extendCanvas directly)
}

//------------------------------------------------------------------------------

// get the size of the view window in pixels
ui.getViewSz = function () {
    return new util.Vec2(ui.render.viewSz);
}

//------------------------------------------------------------------------------

// c is a point in scaled coordinates, which will be positioned in the center of the view area after zooming
ui.setZoomCentered = function (zoom, c) {
    if (!c)
        throw new Error("Center point not specified");
    if (zoom) {
        ui.setZoomRegular(zoom);
    }
    ui.setScrollOffset(0, 0);
    var sp = ui.render.obj2view(c).sub(ui.render.viewSz.scaled(0.5));
    ui.setScrollOffset(sp.x, sp.y);
}

//------------------------------------------------------------------------------

// set the reference point for the "static point" zoom (in object coordinates)
ui.setZoomStaticPointInit = function (s) {
    ui.zspObj = new util.Vec2(s);
}

//------------------------------------------------------------------------------

// vp is the point where the reference point should now be (in view coordinates)
ui.setZoomStaticPoint = function (zoom, vp) {
    ui.setZoomRegular(zoom);
    ui.setScrollOffset(0, 0);
    var avp = ui.render.obj2view(ui.zspObj);
    var so = avp.sub(vp);
    ui.setScrollOffset(so.x, so.y);
}

//------------------------------------------------------------------------------

ui.setScrollOffset = function (x, y) {
    var cx = ui.client_area.clientWidth;
    var cy = ui.client_area.clientHeight;
    var d = ui.render.extendCanvas(x, y, cx + x, cy + y);
    ui.client_area.scrollLeft = x;
    ui.client_area.scrollTop = y;
    ui.scrollLeft = ui.client_area.scrollLeft; // TODO: store drag position in scaled systems
    ui.scrollTop = ui.client_area.scrollTop;
}

//------------------------------------------------------------------------------

ui.setScrollOffsetRel = function (dx, dy) {
    ui.setScrollOffset(ui.client_area.scrollLeft + dx, ui.client_area.scrollTop + dy);
}

//------------------------------------------------------------------------------
// save methods
//------------------------------------------------------------------------------

ui.saveAsSVG = function ()
{
    var canvas_ = document.getElementById("client_area").childNodes[0];
    var text = (new XMLSerializer()).serializeToString(canvas_);
    return(text);
};

//------------------------------------------------------------------------------

ui.saveAsSMILES = function ()
{
    var smiles = '';
    try {
        saver = new chem.SmilesSaver();
        smiles = saver.saveMolecule(ui.ctab, true);
    } catch (er)  {
        smiles = er.message;
    }
    return(smiles);
}

//------------------------------------------------------------------------------
// menu actions
//------------------------------------------------------------------------------

ui.onClick_OpenFile = function ()
{
   if (this.hasClassName('buttonDisabled'))
        return;

    // kulhanek - connect to Nemesis environment
    nemesis.openProject();
};

//------------------------------------------------------------------------------

ui.onClick_SaveFile = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    // kulhanek - connect to Nemesis environment
    nemesis.saveProject();
};

//------------------------------------------------------------------------------

ui.onClick_Undo = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    ui.undo();
};

//------------------------------------------------------------------------------

ui.onClick_Redo = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    ui.redo();
};

//------------------------------------------------------------------------------

ui.onClick_Cut = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    ui.copy();
    ui.removeSelected();
};

//------------------------------------------------------------------------------

ui.onClick_Copy = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    ui.copy();
    ui.updateSelection();
};

//------------------------------------------------------------------------------

ui.onClick_Paste = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    ui.paste();
};

//------------------------------------------------------------------------------

ui.onClick_DeleteAll = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    if (ui.modeType() == ui.MODE.PASTE)
        ui.cancelPaste();

    ui.selectMode(ui.defaultSelector);

    if (!ui.ctab.isBlank())
    {
        ui.addUndoAction(ui.Action.fromNewCanvas(new chem.Struct()));
        ui.render.update();
    }
};

//------------------------------------------------------------------------------

ui.onClick_ZoomIn = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    if (ui.modeType() == ui.MODE.PASTE)
        ui.cancelPaste();

    ui.zoomIdx++;

    if (ui.zoomIdx >= ui.zoomValues.length - 1){
        this.addClassName('buttonDisabled');
    }
    $('zoom_out').removeClassName('buttonDisabled');
    if (ui.zoomIdx < 0 || ui.zoomIdx >= ui.zoomValues.length)
        throw new Error ("Zoom index out of range");
    ui.setZoomCentered(ui.zoomValues[ui.zoomIdx], ui.render.view2obj(ui.render.viewSz.scaled(0.5)));
    // ui.render.update();

    // kulhanek
    try{
        ui.render.onResize();
        ui.render.update()
        ui.setZoomCentered(null, ui.render.getStructCenter());
    } catch(er){

    }

    // kulhanek
    nemesis.updateMenu("changed");
};

//------------------------------------------------------------------------------

ui.onClick_ZoomOut = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    if (ui.modeType() == ui.MODE.PASTE)
        ui.cancelPaste();

    ui.zoomIdx--;

    if (ui.zoomIdx <= 0) {
        this.addClassName('buttonDisabled');
    }

    $('zoom_in').removeClassName('buttonDisabled');
    if (ui.zoomIdx < 0 || ui.zoomIdx >= ui.zoomValues.length)
        throw new Error ("Zoom index out of range");
    ui.setZoomCentered(ui.zoomValues[ui.zoomIdx], ui.render.view2obj(ui.render.viewSz.scaled(0.5)));
   // ui.render.update();

    // kulhanek
    try{
        ui.render.onResize();
        ui.render.update()
        ui.setZoomCentered(null, ui.render.getStructCenter());
    } catch(er){

    }

    // kulhanek
    nemesis.updateMenu("changed");
};

//------------------------------------------------------------------------------

ui.onClick_ShowSMILES = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    ui.showDialog('save_file');
    ui.onChange_FileFormat(null, true);
}

//------------------------------------------------------------------------------

ui.onClick_CleanUp = function ()
{
    if (this.hasClassName('buttonDisabled'))
        return;

    if (ui.modeType() == ui.MODE.PASTE)
    {
        ui.cancelPaste();
        ui.selectMode(ui.defaultSelector);
    }

    var ms = new chem.MolfileSaver();

    try
    {
        ui.loadMolecule(ms.saveMolecule(ui.ctab), true);
    } catch (er)
    {
        alert("Molfile: " + er.message);
    }

    // kulhanek
    try{
        ui.render.onResize();
        ui.render.update()
        ui.setZoomCentered(null, ui.render.getStructCenter());
    } catch(er){

    }
};

//------------------------------------------------------------------------------


