/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 Ian McInerney <Ian.S.McInerney@ieee.org>
 * Copyright (C) 2019 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CVPCB_CONTROL_H_
#define CVPCB_CONTROL_H_

#include <tool/tool_interactive.h>

#include <cvpcb_mainframe.h>


/**
 * Class CVPCB_CONTROL
 *
 * Handles actions in  main cvpcb window.
 */

class CVPCB_CONTROL : public TOOL_INTERACTIVE
{
public:
    CVPCB_CONTROL();
    ~CVPCB_CONTROL() {}

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason ) override;

    /**
     * Undo the footprint associations most recently done.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int Undo( const TOOL_EVENT& aEvent );

    /**
     * Redo the footprint associations most recently done.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int Redo( const TOOL_EVENT& aEvent );

    /**
     * Associate the selected footprint with the currently selected components.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int Associate( const TOOL_EVENT& aEvent );

    /**
     * Perform automatic footprint association.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int AutoAssociate( const TOOL_EVENT& aEvent );

    /**
     * Delete all associations.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int DeleteAll( const TOOL_EVENT& aEvent );

    /**
     * Move the selected component to the not associated one in the specified direction.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int ToNA( const TOOL_EVENT& aEvent );

    /**
     * Show the dialog to modify the included footprint association files (.equ)
     *
     * @param aEvent is the event generated by the tool framework
     */
    int ShowEquFileTable( const TOOL_EVENT& aEvent );

    /**
     * Save the associations to the schematic.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int SaveAssociations( const TOOL_EVENT& aEvent );

    /**
     * Create or Update the frame showing the current highlighted footprint
     * and (if showed) the 3D display frame.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int ShowFootprintViewer( const TOOL_EVENT& aEvent );

    /**
     * Filter the footprint list by toggling the given filter type.
     * The event parameter corresponds to the filter type (using the FP_FILTER_T from the
     * FOOTPRINTS_LISTBOX class)
     *
     * @param aEvent is the event generated by the tool framework
     */
    int ToggleFootprintFilter( const TOOL_EVENT& aEvent );

    /**
     * Update the menu to reflect the current tool states.
     *
     * @param aEvent is the event generated by the tool framework
     */
    int UpdateMenu( const TOOL_EVENT& aEvent );

    /*
     * Sets up handlers for various events.
     */
    void setTransitions() override;

private:
    CVPCB_MAINFRAME* m_frame;
};

#endif
