
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011 Jiri Klusak, anim4tor@mail.muni.cz
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

#include <NemesisCoreModule.hpp>
#include <PluginObject.hpp>
#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>
#include <GraphicsObjectList.hpp>
#include <GraphicsViewList.hpp>

#include <Project.hpp>
#include <Atom.hpp>
#include <HistoryNode.hpp>
#include <SimpleVector.hpp>
#include <FortranMatrix.hpp>
#include <SciLapack.hpp>

#include <GLSelection.hpp>
#include <Transformation.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <ElementColorsList.hpp>

#include <PlaneObject.hpp>
#include <PlaneObjectHistory.hpp>
#include <PlaneSetup.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* PlaneObjectCB(void* p_data);

CExtUUID        PlaneObjectID(
                    "{PLANE_OBJECT:e78be954-3698-4ac4-a58e-2816930b8772}",
                    "Plane");

CPluginObject   PlaneObject(&NemesisCorePlugin,
                    PlaneObjectID,
                    GRAPHICS_OBJECT_CAT,
                    ":/images/NemesisCore/graphics/objects/Plane.svg",
                    PlaneObjectCB);

// -----------------------------------------------------------------------------

QObject* PlaneObjectCB(void* p_data)
{
    return(new CPlaneObject(static_cast<CGraphicsObjectList*>(p_data)));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPlaneObject::CPlaneObject(CGraphicsObjectList* p_gl)
    : CGraphicsObject(&PlaneObject,p_gl)
{
    Setup = NULL;

    Position.x = 0;
    Position.y = 0;
    Position.z = 0;
    PositionObject = NULL;

    // do not use (0,0,1) it is not visible to user for default scene orientation
    Direction.x = 1.0;
    Direction.y = 0.0;
    Direction.z = 0.0;
    DirectionObject = NULL;

    Size = 5;
    Rotation = 0;
    ClippingPlaneID = 0;

    // update setup
    SetupChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPlaneObject::SetPositionWH(const CPoint& position)
{
    if( PositionObject != NULL ) return(false);
    if( Position == position ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,tr("plane position"));
    if( p_history == NULL ) return (false);

    SetPosition(position,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CPlaneObject::SetPositionWH(CProObject* p_position)
{
    if( PositionObject == p_position ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,tr("plane position object"));
    if( p_history == NULL ) return (false);

    SetPosition(p_position,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CPlaneObject::SetDirectionWH(const CPoint& direction)
{
    if( DirectionObject != NULL ) return(false);
    if( Direction == direction ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,tr("plane direction"));
    if( p_history == NULL ) return (false);

    SetDirection(direction,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CPlaneObject::SetDirectionWH(CProObject* p_direction)
{
    if( DirectionObject == p_direction ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,tr("plane direction object"));
    if( p_history == NULL ) return (false);

    SetDirection(p_direction,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CPlaneObject::SetSizeWH(double size)
{
    if( Size == size ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,tr("plane size"));
    if( p_history == NULL ) return (false);

    SetSize(size,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CPlaneObject::SetRotationWH(double rotation)
{
    if( Rotation == rotation ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,tr("plane rotation"));
    if( p_history == NULL ) return (false);

    SetRotation(rotation,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CPlaneObject::SetClippingPlaneIDWH(int id)
{
    if( (id < 0) || (id > 5) ) return(false);
    if( ClippingPlaneID == id ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GRAPHICS,tr("plane cliping id"));
    if( p_history == NULL ) return (false);

    SetClippingPlaneID(id,p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPlaneObject::SetPosition(const CPoint& position,CHistoryNode* p_history)
{
    if( PositionObject != NULL ) return;
    if( Position == position ) return;

    if( p_history ){
        CHistoryItem* p_item = new CPlaneObjectChangePositionHI(this,position);
        p_history->Register(p_item);
    }

    Position = position;
    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CPlaneObject::SetPosition(CProObject* p_position,CHistoryNode* p_history)
{
    if( PositionObject == p_position ) return;

    if( p_history ){
        CHistoryItem* p_item = new CPlaneObjectChangePositionObjHI(this,p_position);
        p_history->Register(p_item);
    }

    if( PositionObject ){
        PositionObject->UnregisterObject(this);
    }

    PositionObject = p_position;

    if( PositionObject ){
        PositionObject->RegisterObject(this);
    }

    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CPlaneObject::SetDirection(const CPoint& direction,CHistoryNode* p_history)
{
    if( DirectionObject != NULL ) return;
    if( Direction == direction ) return;

    if( p_history ){
        CHistoryItem* p_item = new CPlaneObjectChangeDirectionHI(this,direction);
        p_history->Register(p_item);
    }

    Direction = direction;
    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CPlaneObject::SetDirection(CProObject* p_direction,CHistoryNode* p_history)
{
    if( DirectionObject == p_direction ) return;

    if( p_history ){
        CHistoryItem* p_item = new CPlaneObjectChangeDirectionObjHI(this,p_direction);
        p_history->Register(p_item);
    }

    if( DirectionObject ){
        DirectionObject->UnregisterObject(this);
    }

    DirectionObject = p_direction;

    if( DirectionObject ){
        DirectionObject->RegisterObject(this);
    }

    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CPlaneObject::SetSize(double size,CHistoryNode* p_history)
{
    if( Size == size ) return;

    if( p_history ){
        CHistoryItem* p_item = new CPlaneObjectChangeSizeHI(this,size);
        p_history->Register(p_item);
    }

    Size = size;
    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CPlaneObject::SetRotation(double rotation,CHistoryNode* p_history)
{
    if( Rotation == rotation ) return;

    if( p_history ){
        CHistoryItem* p_item = new CPlaneObjectChangeRotationHI(this,rotation);
        p_history->Register(p_item);
    }

    Rotation = rotation;
    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CPlaneObject::SetClippingPlaneID(int id,CHistoryNode* p_history)
{
    if( (id < 0) || (id > 5) ) return;
    if( ClippingPlaneID == id ) return;

    if( p_history ){
        CHistoryItem* p_item = new CPlaneObjectChangeClippingIDHI(this,id);
        p_history->Register(p_item);
    }

    ClippingPlaneID = id;
    emit OnStatusChanged(ESC_OTHER);
}


//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPoint CPlaneObject::GetPosition(void)
{
    if( PositionObject ){
        CGeoDescriptor descrip = PositionObject->GetGeoDescriptor();
        return(descrip.GetPoint1());
    } else {
        return(Position);
    }
}

//------------------------------------------------------------------------------

CProObject* CPlaneObject::GetPositionObject(void)
{
    return(PositionObject);
}

//------------------------------------------------------------------------------

CPoint CPlaneObject::GetDirection(void)
{
    if( DirectionObject ){
        CPoint pos = GetPosition();
        CGeoDescriptor descrip = DirectionObject->GetGeoDescriptor();
        return(descrip.GetPoint1()-pos);
    } else {
        return(Direction);
    }
}

//------------------------------------------------------------------------------

CProObject* CPlaneObject::GetDirectionObject(void)
{
    return(DirectionObject);
}

//------------------------------------------------------------------------------

double CPlaneObject::GetSize(void)
{
    return(Size);
}

//------------------------------------------------------------------------------

double CPlaneObject::GetRotation(void)
{
    return(Rotation);
}

//------------------------------------------------------------------------------

int CPlaneObject::GetClippingPlaneID(void)
{
    return(ClippingPlaneID);
}

//------------------------------------------------------------------------------

CGeoDescriptor CPlaneObject::GetGeoDescriptor(void)
{
    CGeoDescriptor descrip;

    descrip.SetType(EGDT_PLANE);
    descrip.SetPoint1(GetPosition());
    descrip.SetPoint2(GetDirection());

    return(descrip);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPlaneObject::Draw(void)
{
    if( IsFlagSet(EPOF_VISIBLE) == false ) return;

    // plane parameters
    if( IsFlagSet<EPlaneObjectFlag>(EPOF_FIT_TO_ATOMS) == true ){
        FitPlane();
    }

    // visual setup
    if( IsFlagSet<EPlaneObjectFlag>(EPOF_DO_NOT_SHOW) == false ){

        Setup = GetSetup<CPlaneSetup>();
        if( Setup == NULL ){
            ES_ERROR("setup is not available");
            return;
        }

        GLLoadObject(this);
        DrawPlane();
    }

    // clipping
    if( IsFlagSet<EPlaneObjectFlag>(EPOF_CLIP_PLANE) == true ){
        ClipPlane();
    }
}

//------------------------------------------------------------------------------

void CPlaneObject::GetObjectMetrics(CObjMetrics& metrics)
{
    if( IsFlagSet(EPOF_VISIBLE) == false ) return;
    if( IsFlagSet<EPlaneObjectFlag>(EPOF_DO_NOT_SHOW) == true ) return;

    if( IsFlagSet<EPlaneObjectFlag>(EPOF_FIT_TO_ATOMS) == true ){
        FitPlane();
    }

    CTransformation trans;
    CPoint pos = GetPosition();
    CPoint dir = GetDirection();

    float cylH = ::Size(dir);
    float rotAngle;

    if (cylH == 0) {
        rotAngle = 0;
    } else {
        rotAngle = acos(dir.z/cylH)*180.0/3.14; // konstanta je 180/PI
    }

    trans.Translate(pos.x,pos.y,pos.z);
    if( (dir.y != 0) || (dir.x != 0) ) {
        trans.Rotate(rotAngle,-dir.y,dir.x, 0);
    } else {
        if( rotAngle > 90.0 ) trans.Scale(1,1,-1);
    }

    trans.Scale(Size,Size,1);
    trans.Rotate(Rotation,0,0,1);

    for(unsigned int i = 0; i < Vertices.size(); i++) {
        metrics.CompareWith(trans.GetTransform(Vertices[i]));
    }

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPlaneObject::SetupChanged(void)
{
    Setup = GetSetup<CPlaneSetup>();
    if( Setup == NULL ){
        ES_ERROR("setup is not available");
        return;
    }

    // calculate vertices
    Vertices.clear();
    if( Setup->NumOfVertices <= 2 ) return;

    for(int i=0; i < Setup->NumOfVertices; i++){
        CPoint pos;
        pos.x = cos(2.0*i*M_PI/Setup->NumOfVertices);
        pos.y = sin(2.0*i*M_PI/Setup->NumOfVertices);
        pos.z = 0;
        Vertices.push_back(pos);
    }
}

// -----------------------------------------------------------------------------

void CPlaneObject::FitPlane(void)
{
    if( Objects.size() == 0 ) return;

    // calculate COM of atoms
    Position = CPoint();
    double totmass = 0.0;

    foreach(QObject* p_qobj,Objects) {
        CAtom* p_atom = dynamic_cast<CAtom*>(p_qobj);
        if( p_atom ){
            double mass = PeriodicTable.GetMass(p_atom->GetZ());
            Position += p_atom->GetPos()*mass;
            totmass += mass;
        }
    }

    if( totmass > 0 ){
        Position /= totmass;
    }

    // calculate parameters of plane -----------------
    CFortranMatrix a;
    a.CreateMatrix(3,3);

    double a11,a22,a33,a12,a13,a23;
    a11 = 0.0;;
    a22 = 0.0;;
    a33 = 0.0;;
    a12 = 0.0;;
    a13 = 0.0;;
    a23 = 0.0;;

    // construct matrix:
    foreach(QObject* p_qobj,Objects) {
        CAtom* p_atom = dynamic_cast<CAtom*>(p_qobj);
        if( p_atom ){
            CPoint pos = p_atom->GetPos();
            double mass = PeriodicTable.GetMass(p_atom->GetZ());
            a11 = a11 + mass*(pos.x - Position.x)*(pos.x - Position.x);
            a22 = a22 + mass*(pos.y - Position.y)*(pos.y - Position.y);
            a33 = a33 + mass*(pos.z - Position.z)*(pos.z - Position.z);
            a12 = a12 + mass*(pos.x - Position.x)*(pos.y - Position.y);
            a13 = a13 + mass*(pos.x - Position.x)*(pos.z - Position.z);
            a23 = a23 + mass*(pos.y - Position.y)*(pos.z - Position.z);
        }
    }

    a[0][0] = a11;
    a[1][1] = a22;
    a[2][2] = a33;

    a[0][1] = a12;
    a[0][2] = a13;
    a[1][2] = a23;

    a[1][0] = a12;
    a[2][0] = a13;
    a[2][1] = a23;

    // calculate eignevalues and eigenvectors of matrix
    CVector eigenvalues;
    eigenvalues.CreateVector(3);
    eigenvalues.SetZero();

    CVector work;
    work.CreateVector(26*3);
    work.SetZero();

    // now solve eigenproblem
    int info = CSciLapack::syev('V','L', a, eigenvalues, work);

    if( info != 0 ) {
        ES_ERROR("unable to diagonalize plane matrix");
    }

    Direction.x = a[0][0];
    Direction.y = a[1][0];
    Direction.z = a[2][0];

    return;
}

// -----------------------------------------------------------------------------

void CPlaneObject::DrawPlane(void)
{
    glPushMatrix();
        CPoint pos = GetPosition();
        CPoint dir = GetDirection();


        float cylH = ::Size(dir);
        float rotAngle;

        if (cylH == 0) {
            rotAngle = 0;
        } else {
            rotAngle = acos(dir.z/cylH)*180.0/3.14; // konstanta je 180/PI
        }

        glTranslated(pos.x,pos.y,pos.z);
        if( (dir.y != 0) || (dir.x != 0) ) {
            glRotatef(rotAngle,-dir.y,dir.x, 0);
        } else {
            if( rotAngle > 90.0 ) glScalef(1,1,-1);
        }

        glScaled(Size,Size,1);
        glRotatef(Rotation,0,0,1);

        glEnable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);

        if( IsFlagSet(EPOF_SELECTED) == true ) {
            glColor4fv(ColorsList.SelectionMaterial.Color);
        } else {
            glColor4fv(Setup->FrontColor);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glCullFace(GL_BACK);
        glBegin(GL_POLYGON);
            glNormal3f(0,0,1);
            for(unsigned int i = 0; i < Vertices.size(); i++) {
                glVertex3dv(Vertices[i]);
            }
        glEnd();

        if( IsFlagSet(EPOF_SELECTED) == true ) {
            glColor4fv(ColorsList.SelectionMaterial.Color);
        } else {
            glColor4fv(Setup->BackColor);
        }

        glCullFace(GL_FRONT);
        glBegin(GL_POLYGON);
            glNormal3f(0,0,-1);
            for(unsigned int i = 0; i < Vertices.size(); i++) {
                glVertex3dv(Vertices[i]);
            }
        glEnd();

        glDisable(GL_CULL_FACE);

    glPopMatrix();
}

//------------------------------------------------------------------------------

void CPlaneObject::ClipPlane(void)
{
    // enable clipping plane
    GLenum plane_id = GL_CLIP_PLANE0 + ClippingPlaneID;

    CPoint pos = GetPosition();
    CPoint dir = GetDirection();

    GLdouble plane[4];
    plane[0] = dir.x;
    plane[1] = dir.y;
    plane[2] = dir.z;
    plane[3] = -(dir.x*pos.x + dir.y*pos.y + dir.z*pos.z);

    glClipPlane(plane_id,plane);
    glEnable(plane_id);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPlaneObject::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load object info --------------------------
    CGraphicsObject::LoadData(p_ele);

    // specific data -----------------------------
    p_ele->GetAttribute("px",Position.x);
    p_ele->GetAttribute("py",Position.y);
    p_ele->GetAttribute("pz",Position.z);

    // position object
    GetProject()->RequestToLinkObject(this,p_ele,"po");

    p_ele->GetAttribute("dx",Direction.x);
    p_ele->GetAttribute("dy",Direction.y);
    p_ele->GetAttribute("dz",Direction.z);

    // direction object
    GetProject()->RequestToLinkObject(this,p_ele,"do");

    p_ele->GetAttribute("sz",Size);
    p_ele->GetAttribute("rt",Rotation);
}

//------------------------------------------------------------------------------

void CPlaneObject::LinkObject(const QString& role,CProObject* p_object)
{
    if( role == "po" ){
        SetPosition(p_object);
    }
    if( role == "do" ){
        SetDirection(p_object);
    }
}

//------------------------------------------------------------------------------

void CPlaneObject::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // core data ---------------------------------
    CGraphicsObject::SaveData(p_ele);

    // specific data -----------------------------
    p_ele->SetAttribute("px",Position.x);
    p_ele->SetAttribute("py",Position.y);
    p_ele->SetAttribute("pz",Position.z);

    if( PositionObject ){
        p_ele->SetAttribute("po",PositionObject->GetIndex());
    } else {
        int index = -1;
        p_ele->SetAttribute("po",index);
    }

    p_ele->SetAttribute("dx",Direction.x);
    p_ele->SetAttribute("dy",Direction.y);
    p_ele->SetAttribute("dz",Direction.z);

    if( DirectionObject ){
        p_ele->SetAttribute("do",DirectionObject->GetIndex());
    } else {
        int index = -1;
        p_ele->SetAttribute("do",index);
    }

    p_ele->SetAttribute("sz",Size);
    p_ele->SetAttribute("rt",Rotation);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPlaneObject::StartManipulation(CHistoryNode* p_history)
{
    if( p_history == NULL ) return;

    CHistoryItem* p_item = new CPlaneObjectBackupGeoHI(this);
    p_history->Register(p_item);

    // use delayed event notification
    StartEvent();
}

//------------------------------------------------------------------------------

void CPlaneObject::Move(const CPoint& dmov,CGraphicsView* p_view)
{
    if( PositionObject != NULL ) return;

    CPoint _dmov = dmov;

    CTransformation coord = p_view->GetTrans();
    coord.Invert();
    coord.Apply(_dmov);

    Position += _dmov;

    // use delayed event notification
    RaiseEvent();
}

//------------------------------------------------------------------------------

void CPlaneObject::Rotate(const CPoint& drot,CGraphicsView* p_view)
{
    if( DirectionObject != NULL ) return;

    CTransformation strans = p_view->GetTrans();
    strans.Invert();
    CPoint x(1.0,0.0,0.0);
    CPoint y(0.0,1.0,0.0);
    CPoint z(0.0,0.0,1.0);
    strans.Apply(x);
    strans.Apply(y);
    strans.Apply(z);

    CTransformation otrans;
    otrans.Rotate(-drot.y,y);
    otrans.Rotate(drot.z,z);
    otrans.Apply(Direction);

    Rotation += drot.x*180/3.14;

    // use delayed event notification
    RaiseEvent();
}

//------------------------------------------------------------------------------

void CPlaneObject::EndManipulation(void)
{
    // end delayed event notification
    EndEvent();
}

//------------------------------------------------------------------------------

void CPlaneObject::RespondToEvent(void)
{
    // respond to delayed notification
    emit OnStatusChanged(ESC_OTHER);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPlaneObject::RemoveFromRegistered(CProObject* p_object,CHistoryNode* p_history)
{
    if( p_object == NULL ) return;

    if( p_object == PositionObject ) {
        CGeoDescriptor descr = PositionObject->GetGeoDescriptor();
        SetPosition(NULL,p_history);
        SetPosition(descr.GetPoint1(),p_history);
    }

    if( p_object == DirectionObject ) {
        CGeoDescriptor descr = DirectionObject->GetGeoDescriptor();
        SetDirection(NULL,p_history);
        SetDirection(descr.GetPoint1(),p_history);
    }

    CGraphicsObject::RemoveFromRegistered(p_object,p_history);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
