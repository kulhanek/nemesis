// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@chemi.muni.cz,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
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

#include <Fragment.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLIterator.hpp>
#include <QPixmap>
#include <QSvgRenderer>
#include <QPainter>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CFragment::CFragment(void)
{
    MasterAtomZ = 6;
    ActiveConnector = -1;
    ConnectorMode = ECM_MANUAL;
}

//---------------------------------------------------------------------------

CFragment::~CFragment(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CFragment::Clear(void)
{
    Name.clear();
    Atoms.FreeVector();
    Bonds.FreeVector();
    MasterAtomZ = 6;
    Connectors.FreeVector();
    ConnectorMode = ECM_MANUAL;
    ActiveConnector = -1;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const QString& CFragment::GetName(void) const
{
    return(Name);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CFragment::SetFirstConnector(void)
{
    if( Connectors.GetLength() != 0 ) {
        ActiveConnector = Connectors[0];
    } else {
        ActiveConnector=-1;
    }
}

//------------------------------------------------------------------------------

void CFragment::SetNextConnector(void)
{
    // FIXME
    for(size_t i=0; i < Connectors.GetLength()-1; i++) {
        if( ActiveConnector == Connectors[i] ) {
            ActiveConnector = Connectors[i+1];
            return;
        }
    }
    SetFirstConnector();;   // no more connectors -> set first
}

//------------------------------------------------------------------------------

bool CFragment::SetBestConnector(EBondOrder order)
{
    if( ConnectorMode == ECM_MANUAL ) {
        if( Connectors.GetLength() == 0) return(false);
        if( ActiveConnector == -1 )  ActiveConnector = Connectors[0];
        int j = GetConnectBond(ActiveConnector);
        if( (j==-1) || (Bonds[j].Order != order) ) return(false);
        return(true);
    }

    // active connector is preffered if suitable
    int j = GetConnectBond(ActiveConnector);
    if( (j!=-1) && (Bonds[j].Order == order) ) return(true);

    for(size_t i=0; i<Connectors.GetLength(); i++) {
        if( (Atoms[GetAtomDataFromIndex(Connectors[i])].Z == 0) ||
                (Atoms[GetAtomDataFromIndex(Connectors[i])].Z == 1) ) {
            j = GetConnectBond(Connectors[i]);
            if( (j!=-1) && (Bonds[j].Order == order)) {
                ActiveConnector = Connectors[i];
                return(true);
            }
        }
    }
    ActiveConnector=-1;
    return(false);
}

//------------------------------------------------------------------------------

void CFragment::SetConnectorMode(EConnectorMode mode)
{
    ConnectorMode = mode;
}

//------------------------------------------------------------------------------

int CFragment::GetNumOfUserConnectors(void)
{
    return(Connectors.GetLength());
}

//------------------------------------------------------------------------------

int CFragment::GetConnectorIndex(void)
{
    for(size_t i=0; i<Connectors.GetLength(); i++) {
        if( Connectors[i] == ActiveConnector ) return(i);
    }
    return(-1);
}

//------------------------------------------------------------------------------

QIcon CFragment::GetConnectorIcon(void)
{
    return(GetConnectorIconByIndex(GetConnectorIndex()));
}

//------------------------------------------------------------------------------

QIcon CFragment::GetConnectorIconByIndex(int index)
{
    if( (index < 0) || (index > (int)Connectors.GetLength()) ) return(QIcon());
    if( GetConnectorMode() == ECM_AUTO ) {
        return(ConnectorIcons[0]);  // use always first icon
    }
    return(ConnectorIcons[index]);
}

//------------------------------------------------------------------------------

int CFragment::GetCurrentConnector(void)
{
    return(ActiveConnector);
}

//------------------------------------------------------------------------------

int CFragment::GetConnector(int index)
{
    if( (index < 0) || (index >= (int)Connectors.GetLength()) ) return(-1);
    return(Connectors[index]);
}

//------------------------------------------------------------------------------

EConnectorMode  CFragment::GetConnectorMode(void)
{
    return(ConnectorMode);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CFragment::GetNumOfAtoms(void) const
{
    return(Atoms.GetLength());
}

//------------------------------------------------------------------------------

const CAtomData&  CFragment::GetAtomData(int index) const
{
    static CAtomData zero;
    if( (index < 0) || (index >= (int)Atoms.GetLength()) ) return(zero);
    return(Atoms[index]);
}

//------------------------------------------------------------------------------

int CFragment::GetNumOfBonds(void) const
{
    return(Bonds.GetLength());
}

//------------------------------------------------------------------------------

const CBondData&  CFragment::GetBondData(int index) const
{
    static CBondData zero;
    if( (index < 0) || (index >= (int)Bonds.GetLength()) ) return(zero);
    return(Bonds[index]);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CFragment::SetMasterAtomZ(int z)
{
    MasterAtomZ = z;
}

//---------------------------------------------------------------------------

int CFragment::GetMasterAtomZ(void) const
{
    return(MasterAtomZ);
}

//---------------------------------------------------------------------------

int CFragment::GetOppositeAtom(int atom_id,int bond_index)
{
    for(size_t i=0; i<Bonds.GetLength(); i++) {
        if( Bonds[i].A1 == atom_id ) {
            bond_index--;
            if( bond_index == 0 ) return(GetAtomDataFromIndex(Bonds[i].A2));
        }
        if( Bonds[i].A2 == atom_id ) {
            bond_index--;
            if( bond_index == 0 ) return(GetAtomDataFromIndex(Bonds[i].A1));
        }
    }
    return(-1);
}

//---------------------------------------------------------------------------

int CFragment::GetConnectBond(int atom_id)
{
    for(size_t i=0; i<Bonds.GetLength(); i++) {
        if( (Bonds[i].A1 == atom_id) || (Bonds[i].A2 == atom_id) )return(i);
    }
    return(-1);
}

//---------------------------------------------------------------------------

int CFragment::GetAtomDataFromIndex(int atom_id)
{
    for(size_t i = 0; i < Atoms.GetLength(); i++) {
        if(Atoms[i].Index == atom_id )return(i);
    }
    return(-1);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFragment::Load(const CSmallString& name)
{
    CXMLDocument   xml_doc;
    CXMLParser     xml_parser;

    xml_parser.SetOutputXMLNode(&xml_doc);

    if( xml_parser.Parse(name) == false ) {
        CSmallString error;
        error << "unable to parse structure " << name;
        ES_ERROR(error);
        return(false);
    }

    CXMLElement* p_el = xml_doc.GetFirstChildElement("structure");
    try{
        Load(p_el);
    } catch( std::exception& e) {
        ES_ERROR_FROM_EXCEPTION("unable to load fragment",e);
        return(false);
    }
    return(true);
}

//---------------------------------------------------------------------------

bool CFragment::Save(const CSmallString& name)
{
    CXMLDocument   xml_doc;

    xml_doc.CreateChildDeclaration();
    xml_doc.CreateChildComment("NEMESIS structure");

    CXMLElement* p_el = xml_doc.CreateChildElement("structure");
    try{
        Save(p_el);
    } catch(std::exception& e){
        ES_ERROR_FROM_EXCEPTION("unable to save fragment",e);
        return(false);
    }

    CXMLPrinter     xml_printer;
    xml_printer.SetPrintedXMLNode(&xml_doc);

    if( xml_printer.Print(name) == false ) {
        CSmallString error;
        error << "unable to save structure " << name;
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CFragment::Load(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_element is NULL");
    }

    // structure main setup
    p_ele->GetAttribute("name",Name);
    p_ele->GetAttribute("connector",ActiveConnector);
    int mode;
    if( p_ele->GetAttribute("mode",mode) == true ) {
        ConnectorMode = (EConnectorMode)mode;
    }

    CXMLElement* p_gele;
    CXMLElement* p_sele;

    // read atoms
    p_gele = p_ele->GetFirstChildElement("atoms");
    if( p_gele == NULL ) {
        LOGIC_ERROR("unable to open atoms element");
    }

    CXMLIterator A(p_gele);
    Atoms.CreateVector(A.GetNumberOfChildElements("atom"));

    int i = 0;
    while( (p_sele = A.GetNextChildElement("atom")) != NULL ) {
        Atoms[i].Load(p_sele);
        i++;
    }

    // read bonds
    p_gele = p_ele->GetFirstChildElement("bonds");
    if( p_gele == NULL ) {
        LOGIC_ERROR("unable to open bonds element");
    }

    CXMLIterator B(p_gele);
    Bonds.CreateVector(B.GetNumberOfChildElements("bond"));

    i = 0;
    while( (p_sele = B.GetNextChildElement("bond")) != NULL ) {
        Bonds[i].Load(p_sele);
        i++;
    }

    // read connectors
    p_gele = p_ele->GetFirstChildElement("connectors");
    if( p_gele == NULL ) {
        LOGIC_ERROR("unable to open connectors element");
    }

    CXMLIterator C(p_gele);
    Connectors.CreateVector(C.GetNumberOfChildElements("connector"));
    ConnectorIcons.CreateVector(C.GetNumberOfChildElements("connector"));

    i = 0;
    while( (p_sele = C.GetNextChildElement("connector")) != NULL ) {
        p_sele->GetAttribute("id",Connectors[i]);
        CXMLElement* p_iele = p_sele->GetFirstChildElement("svg");
        ConnectorIcons[i] = TryToLoadIcon(p_iele);
        i++;
    }

    SetFirstConnector();
}

//---------------------------------------------------------------------------

QIcon CFragment::TryToLoadIcon(CXMLElement* p_ele)
{
    if( p_ele == NULL ) return(QIcon());

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(p_ele);
    xml_printer.SetPrintAsItIs(true);

    unsigned int length = 0;
    unsigned char* p_data = xml_printer.Print(length);

    QByteArray   ba((const char*)p_data,length);
    delete[] p_data;

    QSvgRenderer svgr;
    if( svgr.load(ba) == false ) return(QIcon());
    QSize size = svgr.defaultSize();

    QPixmap     pix(size);
    QPainter    painter;

    pix.fill(Qt::transparent);

    painter.begin(&pix);
    painter.setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform, true);
    svgr.render(&painter);
    painter.end();

    // FIXME QT4
    //QIcon ico(size);
    QIcon ico;
    ico.addPixmap(pix);
    return(ico);
}

//---------------------------------------------------------------------------

void CFragment::Save(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // structure main setup
    p_ele->SetAttribute("name",Name);
    p_ele->SetAttribute("connector",ActiveConnector);
    p_ele->SetAttribute("mode",(int)ConnectorMode);

    CXMLElement* p_gele;
    CXMLElement* p_sele;

    // write atoms
    p_gele = p_ele->CreateChildElement("atoms");

    for(size_t i=0; i < Atoms.GetLength(); i++) {
        p_sele = p_gele->CreateChildElement("atom");
        Atoms[i].Save(p_sele);
    }

    // write bonds
    p_gele = p_ele->CreateChildElement("bonds");
    for(size_t i=0; i<Bonds.GetLength(); i++) {
        p_sele = p_gele->CreateChildElement("bond");
        Bonds[i].Save(p_sele);
    }

    // write connectors
    p_gele = p_ele->CreateChildElement("connectors");
    for(size_t i=0; i<Connectors.GetLength(); i++) {
        p_sele = p_gele->CreateChildElement("connector");
        p_sele->SetAttribute("id",Connectors[i]);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

