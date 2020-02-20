// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2012 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2008-2009 Petr Kulhanek, kulhanek@enzim.hu,
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

#include <PluginObject.hpp>
#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>
#include <GraphicsObjectList.hpp>
#include <Project.hpp>
#include <GLSelection.hpp>
#include <ElementColorsList.hpp>
#include <Atom.hpp>
#include <StructureList.hpp>
#include <SciLapack.hpp>
#include <SpecAxesObjectHistory.hpp>
#include <CombPhysicalQuantity.hpp>
#include <PhysicalQuantities.hpp>
#include <Structure.hpp>
#include <Residue.hpp>
#include <AtomList.hpp>

#include <NemesisCoreModule.hpp>
#include <SpecAxesObject.hpp>
#include <SpecAxesSetup.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* SpecAxesObjectCB(void* p_data);

CExtUUID        SpecAxesObjectID(
                    "{SPEC_AXES_OBJECT:b89d56a9-c2f7-42d3-85f4-613bd6e97c27}",
                    "Special Axes");

CPluginObject   SpecAxesObject(&NemesisCorePlugin,
                    SpecAxesObjectID,
                    GRAPHICS_OBJECT_CAT,
                    ":/images/NemesisCore/graphics/objects/SpecAxes.svg",
                    SpecAxesObjectCB);

// -----------------------------------------------------------------------------

QObject* SpecAxesObjectCB(void* p_data)
{
    return(new CSpecAxesObject(static_cast<CGraphicsObjectList*>(p_data)));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSpecAxesObject::CSpecAxesObject(CGraphicsObjectList* p_gl)
    : CGraphicsObject(&SpecAxesObject,p_gl)
{
    Tensor.CreateMatrix(3,3);
    EigenValues.CreateVector(3);

    RegisterAllowedObjectType(AtomID);
    RegisterAllowedObjectType(ResidueID);
    RegisterAllowedObjectType(StructureID);

    SetFlag<ESpecAxesObjectFlag>(ESAOF_SHOW_A,true);
    SetFlag<ESpecAxesObjectFlag>(ESAOF_SHOW_B,true);
    SetFlag<ESpecAxesObjectFlag>(ESAOF_SHOW_C,true);

    SetFlag<ESpecAxesObjectFlag>(ESAOF_MASS_WEIGHTED,true);
    SetFlag<ESpecAxesObjectFlag>(ESAOF_APPLY_MAGNITUDES,true);

    Mode = ESAOM_PMT; // default mode

    // unit for tensor of inertia eigenvalues
    PQ_RGT = new CCombPhysicalQuantity(this,"distance square",PQ_DISTANCE,PQ_DISTANCE,ECPQO_MULT);
    PQ_PMI = new CCombPhysicalQuantity(this,"pmi",PQ_MASS,PQ_RGT,ECPQO_MULT);

    connect(GetProject()->GetStructures(),SIGNAL(OnGeometryChangeTick()),
            this,SLOT(UpdateData(void)));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSpecAxesObject::SetModeWH(ESpecAxesObjectMode mode)
{
    if( Mode == mode ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,"set axes mode");
    if( p_history == NULL ) return(false);

    SetMode(mode,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

void CSpecAxesObject::SetMode(ESpecAxesObjectMode mode,CHistoryNode* p_history)
{
    if( Mode == mode ) return;

    if( p_history ){
        CHistoryItem* p_hitem = new CSpecAxesObjectChangeModeHI(this,mode);
        p_history->Register(p_hitem);
    }

    Mode = mode;
    UpdateData();
}

//------------------------------------------------------------------------------

ESpecAxesObjectMode CSpecAxesObject::GetMode(void)
{
    return(Mode);
}

//------------------------------------------------------------------------------

CPhysicalQuantity* CSpecAxesObject::GetVectorComponentPQ(void)
{
    return(PQ_UNITY);
}

//------------------------------------------------------------------------------

CPhysicalQuantity* CSpecAxesObject::GetEigenValuePQ(void)
{
    switch(Mode){
        case ESAOM_PMT:
            return(PQ_PMI);
        case ESAOM_RGT:
            return(PQ_RGT);
    }
    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CPoint& CSpecAxesObject::GetCOM(void)
{
    return(COM);
}

//------------------------------------------------------------------------------

const CPoint CSpecAxesObject::GetEigenVectorA(void)
{
    return(CPoint(Tensor[0][0],Tensor[1][0],Tensor[2][0]));
}

//------------------------------------------------------------------------------

const CPoint CSpecAxesObject::GetEigenVectorB(void)
{
    return(CPoint(Tensor[0][1],Tensor[1][1],Tensor[2][1]));
}

//------------------------------------------------------------------------------

const CPoint CSpecAxesObject::GetEigenVectorC(void)
{
    return(CPoint(Tensor[0][2],Tensor[1][2],Tensor[2][2]));
}

//------------------------------------------------------------------------------

double CSpecAxesObject::GetEigenValueA(void)
{
    return(EigenValues[0]);
}

//------------------------------------------------------------------------------

double CSpecAxesObject::GetEigenValueB(void)
{
    return(EigenValues[1]);
}

//------------------------------------------------------------------------------

double CSpecAxesObject::GetEigenValueC(void)
{
    return(EigenValues[2]);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSpecAxesObject::SetFlags(const CProObjectFlags& flags,CHistoryNode* p_history)
{
    if( GetFlags() == flags ) return;
    CGraphicsObject::SetFlags(flags,p_history);
    UpdateData();
}

//------------------------------------------------------------------------------

void CSpecAxesObject::ObjectListChanged(void)
{
    UpdateData();
}

//------------------------------------------------------------------------------

void CSpecAxesObject::ObjectListObjectAdded(CProObject* p_obj)
{
    CStructure* p_str = dynamic_cast<CStructure*>(p_obj);
    if( p_str != NULL ){
        connect(p_str->GetAtoms(),SIGNAL(OnAtomListChanged()),
                this,SLOT(UpdateData()));
    }
    CResidue* p_res = dynamic_cast<CResidue*>(p_obj);
    if( p_res != NULL ){
        connect(p_res,SIGNAL(OnAtomListChanged()),
                this,SLOT(UpdateData()));
    }
}

//------------------------------------------------------------------------------

void CSpecAxesObject::ObjectListObjectRemoved(CProObject* p_obj)
{
    p_obj->disconnect(this);
}

//------------------------------------------------------------------------------

bool CSpecAxesObject::AreDataValid(void)
{
    return( TotMass >= 2 );
}

//------------------------------------------------------------------------------

void CSpecAxesObject::UpdateData(void)
{
    // calculate center of mass
    COM.SetZero();
    TotMass = 0;

    foreach(CProObject* p_obj, Objects){
        CStructure* p_str = dynamic_cast<CStructure*>(p_obj);
        if( p_str != NULL ){
            AddCOMStructure(p_str);
        }
        CResidue* p_res = dynamic_cast<CResidue*>(p_obj);
        if( p_res != NULL ){
            AddCOMResidue(p_res);
        }
        CAtom* p_atom = dynamic_cast<CAtom*>(p_obj);
        if( p_atom != NULL ){
            AddCOMAtom(p_atom);
        }
    }

    if( TotMass > 0 ){
        COM /= TotMass;
    }

    Tensor.SetZero();
    TotMass = 0;

    foreach(CProObject* p_obj, Objects){
        CStructure* p_str = dynamic_cast<CStructure*>(p_obj);
        if( p_str != NULL ){
            AddAxesStructure(p_str);
        }
        CResidue* p_res = dynamic_cast<CResidue*>(p_obj);
        if( p_res != NULL ){
            AddAxesResidue(p_res);
        }
        CAtom* p_atom = dynamic_cast<CAtom*>(p_obj);
        if( p_atom != NULL ){
            AddAxesAtom(p_atom);
        }
    }

    switch(Mode){
        case ESAOM_PMT:
            // nothing to do
            break;
        case ESAOM_RGT:
            Tensor[0][0] /= TotMass;
            Tensor[0][1] /= TotMass;
            Tensor[0][2] /= TotMass;
            Tensor[1][0] /= TotMass;
            Tensor[1][1] /= TotMass;
            Tensor[1][2] /= TotMass;
            Tensor[2][0] /= TotMass;
            Tensor[2][1] /= TotMass;
            Tensor[2][2] /= TotMass;
            break;
    }

//    cout << Tensor[0][0] << " " << Tensor[0][1] << " " << Tensor[0][2] << endl;
//    cout << Tensor[1][0] << " " << Tensor[1][1] << " " << Tensor[1][2] << endl;
//    cout << Tensor[2][0] << " " << Tensor[2][1] << " " << Tensor[2][2] << endl;

    CSciLapack::syev('V','U',Tensor,EigenValues);

//    cout << Tensor[0][0] << " " << Tensor[0][1] << " " << Tensor[0][2] << endl;
//    cout << Tensor[1][0] << " " << Tensor[1][1] << " " << Tensor[1][2] << endl;
//    cout << Tensor[2][0] << " " << Tensor[2][1] << " " << Tensor[2][2] << endl;

    // make vectors righ-hand ortogonal
        CPoint cp = CrossDot(GetEigenVectorB(),GetEigenVectorC());
        double an = Angle(GetEigenVectorA(),cp);
        if( an > M_PI/2.0 ){
            // swap axes
            Tensor[0][0] = -Tensor[0][0];
            Tensor[1][0] = -Tensor[1][0];
            Tensor[2][0] = -Tensor[2][0];
        }

    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CSpecAxesObject::AddCOMStructure(CStructure* p_str)
{
    if( p_str == NULL ) return;
    foreach(QObject* p_obj,p_str->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_obj);
        AddCOMAtom(p_atom);
    }
}

//------------------------------------------------------------------------------

void CSpecAxesObject::AddCOMResidue(CResidue* p_res)
{
    if( p_res == NULL ) return;
    foreach(CAtom* p_atom,p_res->GetAtoms()){
        AddCOMAtom(p_atom);
    }
}

//------------------------------------------------------------------------------

void CSpecAxesObject::AddCOMAtom(CAtom* p_atom)
{
    if( p_atom == NULL ) return;
    double mass = 1.0;
    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_MASS_WEIGHTED) ){
        mass = p_atom->GetMass();
    }
    TotMass += mass;
    CPoint pos = p_atom->GetPos();
    COM += pos*mass;
}

//------------------------------------------------------------------------------

void CSpecAxesObject::AddAxesStructure(CStructure* p_str)
{
    if( p_str == NULL ) return;
    foreach(QObject* p_obj,p_str->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_obj);
        AddAxesAtom(p_atom);
    }
}

//------------------------------------------------------------------------------

void CSpecAxesObject::AddAxesResidue(CResidue* p_res)
{
    if( p_res == NULL ) return;
    foreach(CAtom* p_atom,p_res->GetAtoms()){
        AddAxesAtom(p_atom);
    }
}

//------------------------------------------------------------------------------

void CSpecAxesObject::AddAxesAtom(CAtom* p_atom)
{
    if( p_atom == NULL ) return;
    double mass = 1.0;
    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_MASS_WEIGHTED) ){
        mass = p_atom->GetMass();
    }
    TotMass += mass;
    CPoint pos = p_atom->GetPos();

    switch(Mode){
        case ESAOM_PMT:
            Tensor[0][0] += mass*((pos.y-COM.y)*(pos.y-COM.y) + (pos.z-COM.z)*(pos.z-COM.z));
            Tensor[1][1] += mass*((pos.x-COM.x)*(pos.x-COM.x) + (pos.z-COM.z)*(pos.z-COM.z));
            Tensor[2][2] += mass*((pos.x-COM.x)*(pos.x-COM.x) + (pos.y-COM.y)*(pos.y-COM.y));
            Tensor[0][1] -= mass*(pos.x-COM.x)*(pos.y-COM.y);
            Tensor[1][0] -= mass*(pos.y-COM.y)*(pos.x-COM.x);
            Tensor[0][2] -= mass*(pos.x-COM.x)*(pos.z-COM.z);
            Tensor[2][0] -= mass*(pos.z-COM.z)*(pos.x-COM.x);
            Tensor[1][2] -= mass*(pos.y-COM.y)*(pos.z-COM.z);
            Tensor[2][1] -= mass*(pos.z-COM.z)*(pos.y-COM.y);
            break;
        case ESAOM_RGT:
            Tensor[0][0] += mass*(pos.x-COM.x)*(pos.x-COM.x);
            Tensor[0][1] += mass*(pos.x-COM.x)*(pos.y-COM.y);
            Tensor[1][0] += mass*(pos.y-COM.y)*(pos.x-COM.x);
            Tensor[0][2] += mass*(pos.x-COM.x)*(pos.z-COM.z);
            Tensor[2][0] += mass*(pos.z-COM.z)*(pos.x-COM.x);
            Tensor[1][1] += mass*(pos.y-COM.y)*(pos.y-COM.y);
            Tensor[1][2] += mass*(pos.y-COM.y)*(pos.z-COM.z);
            Tensor[2][1] += mass*(pos.z-COM.z)*(pos.y-COM.y);
            Tensor[2][2] += mass*(pos.z-COM.z)*(pos.z-COM.z);
            break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSpecAxesObject::SetupChanged(void)
{
    Setup = GetSetup<CSpecAxesSetup>();
    if( Setup == NULL ){
        ES_ERROR("setup is not available");
        return;
    }

    Cone.SetTessellationQuality(Setup->ConeTesselation);
    Cylinder.SetTessellationQuality(Setup->CylinderTesselation);

    // call core update
    CGraphicsObject::SetupChanged();
}

//------------------------------------------------------------------------------

void CSpecAxesObject::Draw(void)
{
    if( AreDataValid() == false ) return;

    if( IsFlagSet(EPOF_VISIBLE) == false ) return;

    Setup = GetSetup<CSpecAxesSetup>();
    if( Setup == NULL ){
        ES_ERROR("setup is not available");
        return;
    }

    GLLoadObject(this);

    SetInitialColorScheme();

    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_SHOW_A) ) {
        DrawAxis(COM,GetEigenVectorA(),GetEigenValueA(),Setup->MagnificationX,
                 &Setup->PXColor,&Setup->ConeXColor);
    }
    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_SHOW_B) ) {
        DrawAxis(COM,GetEigenVectorB(),GetEigenValueB(),Setup->MagnificationY,
                 &Setup->PYColor,&Setup->ConeYColor);
    }
    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_SHOW_C) ) {
        DrawAxis(COM,GetEigenVectorC(),GetEigenValueC(),Setup->MagnificationZ,
                 &Setup->PZColor,&Setup->ConeZColor);
    }
}

//---------------------------------------------------------------------------

void CSpecAxesObject::SetInitialColorScheme(void)
{
    // draw individual objects
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);

    // set global color enviroment
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,ColorsList.GlobalMaterial.Ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,ColorsList.GlobalMaterial.Diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,ColorsList.GlobalMaterial.Emission);

    float shininess[1];
    shininess[0] = ColorsList.GlobalMaterial.Shininess;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS ,shininess);

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,ColorsList.GlobalMaterial.Specular);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,ColorsList.MaterialMode);
}

//---------------------------------------------------------------------------

void CSpecAxesObject::DrawAxis(const CPoint& origin,const CPoint& vect,float value,float pmag,
                               CColor* ppart,CColor* conec)
{
    if( (value == 0) || Size(vect) == 0.0 ) return;  

    CSimplePoint<float>     pd;
    float                   rotAngle;
    float                   cylH;

    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_APPLY_MAGNITUDES) ){
        if( Mode == ESAOM_PMT ){
            if( TotMass > 0 ){
                pd = vect*pmag*sqrt(value/TotMass);
            } else {
                pd = vect;
            }
        }

        if( Mode == ESAOM_RGT ){
            pd = vect*pmag*sqrt(value);
        }
    } else {
        pd = vect;
    }

    cylH = Size(pd);

    glPushMatrix();

    if (cylH == 0) {
        rotAngle = 0;
    } else {
        rotAngle = acos(pd.z/cylH)*180.0/3.14;
    }
    glTranslatef (origin.x,origin.y,origin.z);
    if( (pd.y != 0) || (pd.x != 0) ) {
        glRotatef(rotAngle,-pd.y,pd.x, 0);
    } else {
        if( rotAngle > 90.0 ) glScalef(1,1,-1);
    }

    glColor4fv(*ppart);
    Cylinder.Draw(Setup->AxisRadius,cylH);
    glTranslatef(0,0,cylH);
    glColor4fv(*conec);
    Cone.Draw(Setup->ConeRadius,0,Setup->ConeHeight);

    glPopMatrix();
}

//------------------------------------------------------------------------------

void CSpecAxesObject::GetObjectMetrics(CObjMetrics& metrics)
{
    if( IsFlagSet(EPOF_VISIBLE) == false ) return;

    metrics.CompareWith(COM);

    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_SHOW_A)  ){
        metrics.CompareWith(GetEigenVectorA()*Setup->MagnificationX);
    }

    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_SHOW_B)  ){
        metrics.CompareWith(GetEigenVectorB()*Setup->MagnificationY);
    }

    if( IsFlagSet<ESpecAxesObjectFlag>(ESAOF_SHOW_C)  ){
        metrics.CompareWith(GetEigenVectorC()*Setup->MagnificationZ);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSpecAxesObject::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load object info --------------------------
    CGraphicsObject::LoadData(p_ele);

    // specific data -----------------------------
    p_ele->GetAttribute<ESpecAxesObjectMode>("mode",Mode);
}

//------------------------------------------------------------------------------

void CSpecAxesObject::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // core data ---------------------------------
    CGraphicsObject::SaveData(p_ele);

    // specific data -----------------------------
    p_ele->SetAttribute("mode",Mode);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


