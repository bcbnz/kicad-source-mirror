
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 KiCad Developers, see CHANGELOG.TXT for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include <fctsys.h>
#include <macros.h>
#include <gr_basic.h>
#include <pgm_base.h>
#include <project.h>
#include <wx/stdpaths.h>
#include <kicad_string.h>
#include <config_params.h>
#include <wildcards_and_files_ext.h>



PROJECT::PROJECT()
{
    memset( m_elems, 0, sizeof(m_elems) );
}

PROJECT::~PROJECT()
{
    /* @todo
        careful here, this may work, but the virtual destructor may not
        be in the same link image as PROJECT.  Won't enable this until
        we're more stable and destructor is assuredly in same image, i.e.
        libki.so
    for( unsigned i = 0;  i<DIM(m_elems);  ++i )
        delete m_elems[i];
    */
}


void PROJECT::SetProjectFullName( const wxString& aFullPathAndName )
{
    m_project_name = aFullPathAndName;

    wxASSERT( m_project_name.IsAbsolute() );
#if 0
    wxASSERT( m_project_name.GetExt() == wxT( ".pro" ) )
#else
    m_project_name.SetExt( wxT( ".pro" ) );
#endif

    // until multiple projects are in play, set an environment variable for the
    // the project pointer.
    {
        wxString path = m_project_name.GetPath();

        // wxLogDebug( wxT( "Setting env %s to '%s'." ),  PROJECT_VAR_NAME, GetChars( path ) );

        wxSetEnv( PROJECT_VAR_NAME, path );
    }
}


const wxString PROJECT::GetProjectFullName() const
{
    return m_project_name.GetFullPath();
}


RETAINED_PATH& PROJECT::RPath( RETPATH_T aIndex )
{
    unsigned ndx = unsigned( aIndex );

    if( ndx < DIM( m_rpaths ) )
    {
        return m_rpaths[ndx];
    }
    else
    {
        static RETAINED_PATH no_cookie_for_you;

        wxASSERT( 0 );      // bad index

        return no_cookie_for_you;
    }
}


PROJECT::_ELEM* PROJECT::Elem( ELEM_T aIndex, _ELEM* aElem )
{
    unsigned ndx = unsigned( aIndex );

    if( ndx < DIM( m_elems ) )
    {
        if( aElem )
            m_elems[ndx] = aElem;

        return m_elems[ndx];
    }
    return NULL;
}


// non-member so it can be moved easily, and kept REALLY private.
// Do NOT Clear() in here.
static void add_search_paths( SEARCH_STACK* aDst, wxConfigBase* aCfg, int aIndex )
{
    for( int i=1;  true;  ++i )
    {
        wxString key   = wxString::Format( wxT( "LibraryPath%d" ), i );
        wxString upath = aCfg->Read( key, wxEmptyString );

        if( !upath )
            break;

        aDst->AddPaths( upath, aIndex );
    }
}


// non-member so it can be moved easily, and kept REALLY private.
// Do NOT Clear() in here.
static void add_search_paths( SEARCH_STACK* aDst, const SEARCH_STACK& aSrc, int aIndex )
{
    for( unsigned i=0; i<aSrc.GetCount();  ++i )
        aDst->AddPaths( aSrc[i], aIndex );
}


/*
bool PROJECT::MaybeLoadProjectSettings( const std::vector<wxString>& aFileSet )
{
    // @todo
    return true;
}
*/


wxConfigBase* PROJECT::configCreate( const SEARCH_STACK& aSList, const wxString& aFileName,
            const wxString& aGroupName, bool aForceUseLocalConfig )
{
    wxConfigBase*   cfg = 0;
    wxFileName      fn = aFileName;

    fn.SetExt( ProjectFileExtension );

    // is there an edge transition, a change in m_project_filename?
    if( m_project_name != fn )
    {
        m_sch_search.Clear();

        SetProjectFullName( fn.GetFullPath() );

        // to the empty list, add project dir as first
        m_sch_search.AddPaths( fn.GetPath() );

        // append all paths from aSList
        add_search_paths( &m_sch_search, aSList, -1 );

        // addLibrarySearchPaths( SEARCH_STACK* aSP, wxConfigBase* aCfg )
        // This is undocumented, but somebody wanted to store !schematic!
        // library search paths in the .kicad_common file?
        add_search_paths( &m_sch_search, Pgm().CommonSettings(), -1 );

#if 1 && defined(DEBUG)
        m_sch_search.Show( __func__ );
#endif
    }

    // Init local config filename
    if( aForceUseLocalConfig || fn.FileExists() )
    {
        wxString cur_pro_fn = fn.GetFullPath();

        cfg = new wxFileConfig( wxEmptyString, wxEmptyString, cur_pro_fn, wxEmptyString );

        cfg->DontCreateOnDemand();

        if( aForceUseLocalConfig )
        {
            SetProjectFullName( cur_pro_fn );
            return cfg;
        }

        /* Check the application version against the version saved in the
         * project file.
         *
         * TODO: Push the version test up the stack so that when one of the
         *       KiCad application version changes, the other applications
         *       settings do not get updated.  Practically, this can go away.
         *       It isn't used anywhere as far as I know (WLS).
         */

        cfg->SetPath( aGroupName );

        int def_version = 0;
        int version = cfg->Read( wxT( "version" ), def_version );

        if( version > 0 )
        {
            cfg->SetPath( wxCONFIG_PATH_SEPARATOR );
            SetProjectFullName( cur_pro_fn );
            return cfg;
        }
        else    // Version incorrect
        {
            delete cfg;
            cfg = 0;
        }
    }

    // Search for the template kicad.pro file by using caller's SEARCH_STACK.

    wxString kicad_pro_template = aSList.FindValidPath( wxT( "kicad.pro" ) );

    if( !kicad_pro_template )
    {
        wxLogDebug( wxT( "Template file <kicad.pro> not found." ) );

        fn = wxFileName( wxStandardPaths::Get().GetDocumentsDir(),
                         wxT( "kicad" ), ProjectFileExtension );
    }
    else
    {
        fn = kicad_pro_template;
    }

    cfg = new wxFileConfig( wxEmptyString, wxEmptyString, wxEmptyString, fn.GetFullPath() );
    cfg->DontCreateOnDemand();

    SetProjectFullName( fn.GetFullPath() );
    return cfg;
}


void PROJECT::ConfigSave( const SEARCH_STACK& aSList, const wxString&  aFileName,
        const wxString& aGroupName, const PARAM_CFG_ARRAY& aParams )
{
    std::auto_ptr<wxConfigBase> cfg( configCreate( aSList, aFileName, aGroupName, FORCE_LOCAL_CONFIG ) );

    cfg->SetPath( wxCONFIG_PATH_SEPARATOR );

    cfg->Write( wxT( "update" ), DateAndTime() );

    // @todo: pass in aLastClient wxString:
    cfg->Write( wxT( "last_client" ), Pgm().App().GetAppName() );

    // Save parameters
    cfg->DeleteGroup( aGroupName );     // Erase all data
    cfg->Flush();

    cfg->SetPath( aGroupName );
    cfg->Write( wxT( "version" ), CONFIG_VERSION );

    cfg->SetPath( wxCONFIG_PATH_SEPARATOR );

    wxConfigSaveParams( cfg.get(), aParams, aGroupName );

    cfg->SetPath( UNIX_STRING_DIR_SEP );

    // cfg is deleted here by std::auto_ptr, that saves the *.pro file to disk
}


bool PROJECT::ConfigLoad( const SEARCH_STACK& aSList, const wxString& aFileName,
        const wxString&  aGroupName, const PARAM_CFG_ARRAY& aParams,
        bool doLoadOnlyIfNew )
{
    std::auto_ptr<wxConfigBase> cfg( configCreate( aSList, aFileName, aGroupName, false ) );

    cfg->SetPath( wxCONFIG_PATH_SEPARATOR );

    wxString timestamp = cfg->Read( wxT( "update" ) );

    if( doLoadOnlyIfNew && timestamp.size() &&
        timestamp == m_pro_date_and_time )
    {
        return false;
    }

    m_pro_date_and_time = timestamp;

    wxConfigLoadParams( cfg.get(), aParams, aGroupName );

    return true;
}

