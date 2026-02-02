// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <TorsionProperty.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <HistoryNode.hpp>
#include <PropertyList.hpp>
#include <PropertyAtomList.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <GeoMeasurement.hpp>
#include <GLSelection.hpp>
#include <Atom.hpp>
#include <GeoPropertySetup.hpp>
#include <GraphicsUtil.hpp>
#include <ElementColorsList.hpp>
#include <math.h>

#if defined _WIN32 || defined __CYGWIN__
#undef DrawText
#endif

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* TorsionPropertyCB(void* p_data);

CExtUUID        TorsionPropertyID(
                    "{TORSION_PROPERTY:49fb346b-b34b-4845-90e5-35d72f60fbae}",
                    "Torsion");

CPluginObject   TorsionPropertyObject(&NemesisCorePlugin,
                    TorsionPropertyID,PROPERTY_CAT,
                    ":/images/NemesisCore/properties/Torsion.svg",
                    TorsionPropertyCB);

// -----------------------------------------------------------------------------

QObject* TorsionPropertyCB(void* p_data)
{
    return(new CTorsionProperty(static_cast<CPropertyList*>(p_data)));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTorsionProperty::CTorsionProperty(CPropertyList *p_bl)
    : CGeoProperty(&TorsionPropertyObject,p_bl)
{  
    PropUnit = PQ_ANGLE;

    PointA = new CPropertyAtomList(this);
    connect(PointA,SIGNAL(OnPropertyAtomListChanged(void)),
            this,SLOT(PropertyAtomListChanged(void)));

    PointB = new CPropertyAtomList(this);
    connect(PointB,SIGNAL(OnPropertyAtomListChanged(void)),
            this,SLOT(PropertyAtomListChanged(void)));

    PointC = new CPropertyAtomList(this);
    connect(PointC,SIGNAL(OnPropertyAtomListChanged(void)),
            this,SLOT(PropertyAtomListChanged(void)));

    PointD = new CPropertyAtomList(this);
    connect(PointD,SIGNAL(OnPropertyAtomListChanged(void)),
            this,SLOT(PropertyAtomListChanged(void)));

    SET_FLAG(PropFlags,EPF_SCALAR_VALUE,true);
    SET_FLAG(PropFlags,EPF_CARTESIAN_GRADIENT,true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorsionProperty::IsReady(void)
{
    bool cready = true;

    cready &= PointA->GetNumberOfAtoms() > 0;
    cready &= PointB->GetNumberOfAtoms() > 0;
    cready &= PointC->GetNumberOfAtoms() > 0;
    cready &= PointD->GetNumberOfAtoms() > 0;

    return( cready );
}

//------------------------------------------------------------------------------

bool CTorsionProperty::IsFromStructure(CStructure* p_str)
{
    bool cready = true;

    cready &= PointA->IsFromStructure(p_str);
    cready &= PointB->IsFromStructure(p_str);
    cready &= PointC->IsFromStructure(p_str);
    cready &= PointD->IsFromStructure(p_str);

    return( cready );
}

//------------------------------------------------------------------------------

bool CTorsionProperty::ComposedBySingleAtomGroups(void)
{
    bool cready = true;

    cready &= PointA->GetNumberOfAtoms() == 1;
    cready &= PointB->GetNumberOfAtoms() == 1;
    cready &= PointC->GetNumberOfAtoms() == 1;
    cready &= PointD->GetNumberOfAtoms() == 1;

    return( cready );
}

//------------------------------------------------------------------------------

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

//------------------------------------------------------------------------------

double CTorsionProperty::GetScalarValue(void)
{
    if( IsReady() == false ) return(0.0);

    // point A
    double atotmass;
    CPoint acom = PointA->GetCOM(atotmass);
    if( atotmass == 0.0 ) return(0.0);

    // point B
    double btotmass;
    CPoint bcom = PointB->GetCOM(btotmass);
    if( btotmass == 0.0 ) return(0.0);

    // point C
    double ctotmass;
    CPoint ccom = PointC->GetCOM(ctotmass);
    if( ctotmass == 0.0 ) return(0.0);

    // point D
    double dtotmass;
    CPoint dcom = PointD->GetCOM(dtotmass);
    if( dtotmass == 0.0 ) return(0.0);

    CPoint f,g,h;

    f = acom - bcom;
    g = bcom - ccom;
    h = dcom - ccom;

    CPoint a,b;

    a.x = f.y*g.z - f.z*g.y;
    a.y = f.z*g.x - f.x*g.z;
    a.z = f.x*g.y - f.y*g.x;

    b.x = h.y*g.z - h.z*g.y;
    b.y = h.z*g.x - h.x*g.z;
    b.z = h.x*g.y - h.y*g.x;

    double a2 = a.x*a.x + a.y*a.y + a.z*a.z;
    double b2 = b.x*b.x + b.y*b.y + b.z*b.z;

    //! calculate scp and value
    double scp = (a.x*b.x+a.y*b.y+a.z*b.z)/sqrt(a2*b2);

    double value = 0.0;
    if ( scp > 1.0 ) {
        scp =  1.0;
        value = acos (1.0); // ! const
    } else if ( scp < -1.0 ) {
        scp = -1.0;
        value = acos (-1.0); // ! const
    } else {
        value = acos ( scp );
    }
    if( g.x*(a.y*b.z-a.z*b.y) +
            g.y*(a.z*b.x-a.x*b.z) +
            g.z*(a.x*b.y-a.y*b.x) > 0.0) {
        value = -value;
    }

    return(value);
}

//------------------------------------------------------------------------------

double CTorsionProperty::GetScalarDeviation(double target_value)
{
    double val = GetScalarValue();
    double dv = std::remainder(val - target_value, 2.0 * M_PI);
    return(dv);
}

//------------------------------------------------------------------------------

double CTorsionProperty::GetGradient(QVector<CAtomGrad>& grads)
{
    // point A
    double atotmass = 0.0;
    CPoint acom = PointA->GetCOM(atotmass);
    if( atotmass == 0.0 ) return(0.0);

    // point B
    double btotmass = 0.0;
    CPoint bcom = PointB->GetCOM(btotmass);
    if( btotmass == 0.0 ) return(0.0);

    // point C
    double ctotmass = 0.0;
    CPoint ccom = PointC->GetCOM(ctotmass);
    if( ctotmass == 0.0 ) return(0.0);

    // point D
    double dtotmass = 0.0;
    CPoint dcom = PointD->GetCOM(dtotmass);
    if( dtotmass == 0.0 ) return(0.0);

    CPoint f,g,h;

    f = acom - bcom;
    g = bcom - ccom;
    h = dcom - ccom;

    CPoint a,b;

    a.x = f.y*g.z - f.z*g.y;
    a.y = f.z*g.x - f.x*g.z;
    a.z = f.x*g.y - f.y*g.x;

    b.x = h.y*g.z - h.z*g.y;
    b.y = h.z*g.x - h.x*g.z;
    b.z = h.x*g.y - h.y*g.x;

    double fg = f.x*g.x + f.y*g.y + f.z*g.z;
    double hg = h.x*g.x + h.y*g.y + h.z*g.z;
    double a2 = a.x*a.x + a.y*a.y + a.z*a.z;
    double b2 = b.x*b.x + b.y*b.y + b.z*b.z;
    double gv = sqrt( g.x*g.x + g.y*g.y + g.z*g.z );

        //! calculate scp and value
    double scp = (a.x*b.x+a.y*b.y+a.z*b.z)/sqrt(a2*b2);

    double value = 0.0;
    if ( scp > 1.0 ) {
        scp =  1.0;
        value = acos (1.0); // ! const
    } else if ( scp < -1.0 ) {
        scp = -1.0;
        value = acos (-1.0); // ! const
    } else {
        value = acos ( scp );
    }
    if( g.x*(a.y*b.z-a.z*b.y) +
        g.y*(a.z*b.x-a.x*b.z) +
        g.z*(a.x*b.y-a.y*b.x) > 0.0) {
        value = -value;
    }

    // geo%grd(:,i) = geo%grd(:,i) + dv*( -gv/a2*a(:) )
    // geo%grd(:,j) = geo%grd(:,j) + dv*(  (gv/a2 + fg/(a2*gv))*a(:) - hg/(b2*gv)*b(:) )
    // geo%grd(:,k) = geo%grd(:,k) + dv*(  (hg/(b2*gv) - gv/b2)*b(:) - fg/(a2*gv)*a(:) )
    // geo%grd(:,l) = geo%grd(:,l) + dv*( gv/b2*b(:) )

    // allocate space
    int numofatms = PointA->GetNumberOfAtoms() + PointB->GetNumberOfAtoms()
                  + PointC->GetNumberOfAtoms() + PointD->GetNumberOfAtoms();
    grads.resize(numofatms);

    int index = 0;
    foreach(CAtom* p_atom, PointA->GetAtoms()){
        double    tmp = p_atom->GetMass() / atotmass;
        CAtomGrad grd;
        grd.Atom = p_atom;
        grd.Grad.x = tmp * (-gv/a2*a.x);
        grd.Grad.y = tmp * (-gv/a2*a.y);
        grd.Grad.z = tmp * (-gv/a2*a.z);
        grads[index++] = grd;
    }

    foreach(CAtom* p_atom, PointB->GetAtoms()){
        double    tmp = p_atom->GetMass() / btotmass;
        CAtomGrad grd;
        grd.Atom = p_atom;
        grd.Grad.x = tmp * ( (gv/a2 + fg/(a2*gv))*a.x - hg/(b2*gv)*b.x );
        grd.Grad.y = tmp * ( (gv/a2 + fg/(a2*gv))*a.y - hg/(b2*gv)*b.y );
        grd.Grad.z = tmp * ( (gv/a2 + fg/(a2*gv))*a.z - hg/(b2*gv)*b.z );
        grads[index++] = grd;
    }

    foreach(CAtom* p_atom, PointC->GetAtoms()){
        double    tmp = p_atom->GetMass() / ctotmass;
        CAtomGrad grd;
        grd.Atom = p_atom;
        grd.Grad.x = tmp * ( (hg/(b2*gv) - gv/b2)*b.x - fg/(a2*gv)*a.x );
        grd.Grad.y = tmp * ( (hg/(b2*gv) - gv/b2)*b.y - fg/(a2*gv)*a.y );
        grd.Grad.z = tmp * ( (hg/(b2*gv) - gv/b2)*b.z - fg/(a2*gv)*a.z );
        grads[index++] = grd;
    }

    foreach(CAtom* p_atom, PointD->GetAtoms()){
        double    tmp = p_atom->GetMass() / dtotmass;
        CAtomGrad grd;
        grd.Atom = p_atom;
        grd.Grad.x = tmp * ( gv/b2*b.x );
        grd.Grad.y = tmp * ( gv/b2*b.y );
        grd.Grad.z = tmp * ( gv/b2*b.z );
        grads[index++] = grd;
    }

    return(value);
}

//------------------------------------------------------------------------------

double CTorsionProperty::GetDeviationAndGradient(QVector<CAtomGrad>& grads,double target_value)
{
    return( std::remainder(GetGradient(grads) - target_value, 2.0 * M_PI) );
}

//------------------------------------------------------------------------------

CPropertyAtomList* CTorsionProperty::GetPointA(void)
{
    return(PointA);
}

//------------------------------------------------------------------------------

CPropertyAtomList* CTorsionProperty::GetPointB(void)
{
    return(PointB);
}

//------------------------------------------------------------------------------

CPropertyAtomList* CTorsionProperty::GetPointC(void)
{
    return(PointC);
}

//------------------------------------------------------------------------------

CPropertyAtomList* CTorsionProperty::GetPointD(void)
{
    return(PointD);
}

//------------------------------------------------------------------------------

bool CTorsionProperty::HasGradient(CStructure* p_structure)
{
    if( PointA->ContainsAnyAtomFrom(p_structure) ) return(true);
    if( PointB->ContainsAnyAtomFrom(p_structure) ) return(true);
    if( PointC->ContainsAnyAtomFrom(p_structure) ) return(true);
    if( PointD->ContainsAnyAtomFrom(p_structure) ) return(true);
    return(false);
}

//------------------------------------------------------------------------------

void CTorsionProperty::PropertyAtomListChanged(void)
{
    emit OnStatusChanged(ESC_OTHER);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTorsionProperty::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // core ----------------------------
    CGeoProperty::LoadData(p_ele);

    // datapoints ----------------------
    CXMLElement* p_pele;
    p_pele = p_ele->GetFirstChildElement("point_a");
    if( p_pele ) {
        PointA->LoadData(p_pele);
    }

    p_pele = p_ele->GetFirstChildElement("point_b");
    if( p_pele ){
        PointB->LoadData(p_pele);
    }

    p_pele = p_ele->GetFirstChildElement("point_c");
    if( p_pele ) {
        PointC->LoadData(p_pele);
    }

    p_pele = p_ele->GetFirstChildElement("point_d");
    if( p_pele ){
        PointD->LoadData(p_pele);
    }
}

//------------------------------------------------------------------------------

void CTorsionProperty::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // core ----------------------------
    CGeoProperty::SaveData(p_ele);

    // datapoints ----------------------
    CXMLElement* p_pele;

    p_pele = p_ele->CreateChildElement("point_a");
    PointA->SaveData(p_pele);

    p_pele = p_ele->CreateChildElement("point_b");
    PointB->SaveData(p_pele);

    p_pele = p_ele->CreateChildElement("point_c");
    PointC->SaveData(p_pele);

    p_pele = p_ele->CreateChildElement("point_d");
    PointD->SaveData(p_pele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTorsionProperty::Draw(void)
{
    if( IsReady() == false ) return;

    Setup = GetSetup<CGeoPropertySetup>();
    if( Setup == NULL ){
        ES_ERROR("setup is not available");
        return;
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    GLLoadObject(this);

    LabelTorsion();

    glDisable(GL_LINE_STIPPLE);
}

//------------------------------------------------------------------------------

void CTorsionProperty::LabelTorsion(void)
{    
    CSimplePoint<float> pos1 = PointA->GetCOM();
    CSimplePoint<float> pos2 = PointB->GetCOM();
    CSimplePoint<float> pos3 = PointC->GetCOM();
    CSimplePoint<float> pos4 = PointD->GetCOM();

    // calculate dihed ---------------------------
    double              dihed;
    dihed = CGeoMeasurement::GetTorsion(pos1,pos2,pos3,pos4);

    // draw dihed --------------------------------
    glLineWidth(Setup->LineWidth);
    if( Setup->LineStippleFactor > 0 ){
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(Setup->LineStippleFactor,Setup->LineStipplePattern);
    } else {
        glDisable(GL_LINE_STIPPLE);
    }

    CSimplePoint<float> mp = (pos2+pos3)*0.5;
    CSimplePoint<float> v1,v2,c1,c2;
    v1 = pos1-pos2;
    v2 = pos3-pos2;
    c1 = CrossDot(v1,v2);
    c2 = CrossDot(v2,c1);

    CSimplePoint<float> s1 = Norm(c2)+mp;

    v1 = pos4-pos3;
    v2 = pos2-pos3;
    c1 = CrossDot(v1,v2);
    c2 = CrossDot(v2,c1);

    CSimplePoint<float> s2 = Norm(c2)+mp;

    if( IsFlagSet(EPOF_SELECTED) ){
        glColor4fv(ColorsList.SelectionMaterial.Color);
    } else {
        glColor4fv(Setup->LineColor);
    }

    glBegin(GL_LINES);
        glVertex3fv(pos1);
        glVertex3fv(pos2);
        glVertex3fv(pos2);
        glVertex3fv(pos3);
        glVertex3fv(pos3);
        glVertex3fv(pos4);
        glVertex3fv(mp);
        glVertex3fv(s1);
        glVertex3fv(pos1);
        glVertex3fv(s1);
        glVertex3fv(mp);
        glVertex3fv(s2);
        glVertex3fv(pos4);
        glVertex3fv(s2);
    glEnd();

    float angle = CGeoMeasurement::GetAngle(s1,mp,s2);

    CArc arc;
    CSimplePoint<float>  pm;
    arc.Draw(s1,mp,s2,angle,pm);

    glDisable(GL_LINE_STIPPLE);

    // draw text and quotation -------------------
    CSimplePoint<float>  textpos;

    if( IsFlagSet<EGeoPropertyObjectFlag>(EGPOF_RELATIVE_LABEL_POS) ){
        textpos = GetLabelPosition() + pm;
    } else {
        textpos = GetLabelPosition();
    }

    if( IsFlagSet<EGeoPropertyObjectFlag>(EGPOF_SHOW_LABEL) ){
        QString text = PQ_ANGLE->GetRealValueText(dihed);

        if( Setup->ShowUnit == true ){
           text += " " + PQ_ANGLE->GetUnitName();
           }

        DrawText(textpos,text);
    }

    DrawLabelQuotationLine(pm,textpos);

    DrawCOMPosition(pos1);
    DrawCOMQuotation(pos1,PointA->GetAtoms());
    DrawCOMPosition(pos2);
    DrawCOMQuotation(pos2,PointB->GetAtoms());
    DrawCOMPosition(pos3);
    DrawCOMQuotation(pos3,PointC->GetAtoms());
    DrawCOMPosition(pos4);
    DrawCOMQuotation(pos4,PointD->GetAtoms());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


