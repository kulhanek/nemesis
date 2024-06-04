// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2008 Petr Kulhanek, kulhanek@enzim.hu,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
//    Copyright (C) 1998-2004 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

/*! \mainpage Nemesis
 *
 * \section intro_sec Introduction
 *
 * Nemesis is an advanced molecular structure builder. It is able to build a molecule from predefined 3D fragments. Molecules can be visualized in several representations, including monoscopic and stereoscopic. The geometry of the built molecule can be easily measured and manipulated. Nemesis is a modular program, and its functionality can be extended by using various plugins. For example, it supports imports and exports of various chemical formats using the OpenBabel library. This library is also used for geometry optimization by several force fields.
 */

#include <NemesisJScript.hpp>
#include <NemesisStyle.hpp>
#include "NemesisApplication.hpp"
#include <ErrorSystem.hpp>
#include <TerminalStr.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int main(int argc, char* argv[])
{
    // Qt WebEngine seems to be initialized from a plugin.
    // Please set Qt::AA_ShareOpenGLContexts using QCoreApplication::setAttribute before constructing QGuiApplication.
    // so be it ...
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts,true);

    // run application
    CNemesisApplication* p_app = new CNemesisApplication(argc,argv);

    // this style override layout default margins
    QApplication::setStyle(new CNemesisStyle);

    CNemesisJScript* p_jsengine = new CNemesisJScript;
    try {
        int result = 0;
        switch(p_jsengine->Init(argc,argv)){
            case SO_CONTINUE:
                result = 0;
                if(p_jsengine->Run() == false) result = 1;
                break;
            case SO_EXIT:
                return(0);
            case SO_OPTS_ERROR:
                return(1);
            case SO_USER_ERROR:
            default:
                result = 2;
                break;
        }
        p_jsengine->Finalize();
        return(result);
    } catch(std::exception& e) {
        CTerminalStr tout;
        tout.Attach(stderr);
        tout << std::endl;
        tout << "<red><b>>>> UNHANDLED EXCEPTION: " << e.what() << "</b></red>" << std::endl;
        ErrorSystem.PrintErrors(tout);
        tout << std::endl;
        return(255);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
