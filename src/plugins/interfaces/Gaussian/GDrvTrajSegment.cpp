// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2013 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <Trajectory.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <Project.hpp>
#include <fstream>
#include <Structure.hpp>
#include <AtomList.hpp>
#include <PhysicalQuantity.hpp>
#include <PhysicalQuantities.hpp>
#include <qcustomplot.h>
#include <DistanceProperty.hpp>
#include <AngleProperty.hpp>
#include <TorsionProperty.hpp>
#include <PropertyList.hpp>
#include <PropertyAtomList.hpp>

#include "GaussianModule.hpp"
#include "GaussianUtils.hpp"
#include "GDrvTrajSegment.hpp"

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

QObject* GDrvTrajSegmentCB(void* p_data);

CExtUUID        GDrvTrajSegmentID(
                    "{GDRV_TRAJ_SEGMENT:9e765d39-5702-446f-bf71-2564b8a39dcd}",
                    "Gaussian Single Coordinate Driving");

CPluginObject   GDrvTrajSegmentObject(&GaussianPlugin,
                    GDrvTrajSegmentID,TRAJECTORY_SEGMENT_CAT,
                    ":/images/Gaussian/GDrvTrajSegment.svg",
                    GDrvTrajSegmentCB);

// -----------------------------------------------------------------------------

QObject* GDrvTrajSegmentCB(void* p_data)
{
    CTrajectory* p_traj = static_cast<CTrajectory*>(p_data);
    if( p_traj == NULL ){
        ES_ERROR("CGDrvTrajSegment requires active trajectory");
        return(NULL);
    }

    QObject* p_build_wp = new CGDrvTrajSegment(p_traj);
    return(p_build_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGDrvTrajSegment::CGDrvTrajSegment(CTrajectory* p_traj)
    : CTrajectorySegment(&GDrvTrajSegmentObject,p_traj,true)
{
    CVKey = GetProject()->GetFreeObjectIndex();
    EnergyKey = GetProject()->GetFreeObjectIndex();
    Model = NULL;
    PQ_CV = PQ_DISTANCE;    // fallback unit for CV
    CVScaleFac = 1.0;

    // response to energy unit change
    connect(PQ_ENERGY,SIGNAL(OnUnitChanged(void)),
            this,SLOT(CreateModel(void)));
}

//------------------------------------------------------------------------------

CGDrvTrajSegment::~CGDrvTrajSegment(void)
{
    foreach(CSnapshot* p_snap, Snapshots){
        delete p_snap;
    }
    Snapshots.clear();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

long int CGDrvTrajSegment::GetNumberOfSnapshots(void)
{
    return(Snapshots.count());
}

//------------------------------------------------------------------------------

CSnapshot* CGDrvTrajSegment::GetCurrentSnapshot(void)
{
    if( (SnapshotIndex < 1) || (SnapshotIndex > Snapshots.count()) ) return(NULL);
    return(Snapshots.at(SnapshotIndex-1));
}

//------------------------------------------------------------------------------

QAbstractItemModel* CGDrvTrajSegment::GetEnergyModel(void)
{
    if( Model == NULL ){
        CreateModel();
    }
    return(Model);
}

//------------------------------------------------------------------------------

const QString& CGDrvTrajSegment::GetMethod(void) const
{
    return(Method);
}

//------------------------------------------------------------------------------

const QString& CGDrvTrajSegment::GetCVDef(void) const
{
    return(SCDDef);
}

//------------------------------------------------------------------------------

CPhysicalQuantity* CGDrvTrajSegment::GetCVUnit(void) const
{
    return(PQ_CV);
}

//------------------------------------------------------------------------------

void CGDrvTrajSegment::CreateModel(void)
{
    if( Model == NULL ){
        Model = new QStandardItemModel(this);
    } else {
        Model->clear();
    }

    QStringList header;
    QString cv_hdr = QString(tr("Value [%1]")).arg(PQ_CV->GetUnitName());
    QString rel_energy_hdr = QString(tr("Relative Energy [%1]")).arg(PQ_ENERGY->GetUnitName());

    header << tr("ID") << cv_hdr << tr("Energy [a.u.]") << rel_energy_hdr;
    Model->setHorizontalHeaderLabels(header);

    // get first energy
    double first_energy = 0;
    if( Snapshots.count() > 0 ){
        first_energy = Snapshots.first()->GetProperty(EnergyKey);
    }

    // load items
    int i = 0;
    foreach(CSnapshot* p_snap,Snapshots){

        double cv = p_snap->GetProperty(CVKey);
        double energy = p_snap->GetProperty(EnergyKey);
        double rel_energy = energy - first_energy;
        // convert to kcal/mol
        rel_energy *= 627.509608;

        QStandardItem*          p_item;
        QList<QStandardItem*>   items;

        // snapshot ID
        p_item = new QStandardItem(QString("%1").arg(i+1));
        p_item->setTextAlignment(Qt::AlignLeft);
        items << p_item;

        // coordinate value
        p_item = new QStandardItem(PQ_CV->GetRealValueText(cv));
        p_item->setTextAlignment(Qt::AlignLeft);
        items << p_item;

        // absolute energy
        p_item = new QStandardItem(QString().asprintf("%.9f",energy));
        p_item->setTextAlignment(Qt::AlignRight);
        items << p_item;

        p_item = new QStandardItem(PQ_ENERGY->GetRealValueText(rel_energy));
        p_item->setTextAlignment(Qt::AlignRight);
        items << p_item;

        Model->appendRow(items);

        i++;
    }
}

//------------------------------------------------------------------------------

void CGDrvTrajSegment::PopulateGraphData(QCPCurve* p_graph)
{
    // get first energy
    double first_energy = 0;
    if( Snapshots.count() > 0 ){
        first_energy = Snapshots.first()->GetProperty(EnergyKey);
    }

    p_graph->data()->clear();

    // load items
    foreach(CSnapshot* p_snap,Snapshots){

        double cv = p_snap->GetProperty(CVKey);
        double energy = p_snap->GetProperty(EnergyKey);
        double rel_energy = energy - first_energy;
        // convert to kcal/mol
        rel_energy *= 627.509608;

        p_graph->addData(PQ_CV->GetRealValue(cv),PQ_ENERGY->GetRealValue(rel_energy));
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGDrvTrajSegment::LoadTrajectoryData(void)
{
    std::ifstream   sin;

    sin.open(FileName.toLatin1());
    if( !sin ) {
        CSmallString error;
        error << tr("unable open trajectory segment") << " '" << FileName << "'";
        GetProject()->TextNotification(ETNT_ERROR,QString(error),ETNT_ERROR_DELAY);
        return;
    }

    int lineno = 0;

    // read method
    if( CGaussianUtils::ReadMethod(sin,lineno,Method) == false ) return;

    // read SCD
    QString mod;
    if( CGaussianUtils::ReadSCDDef(sin,lineno,SCDDef,mod,CVScaleFac,PQ_CV) == false ) return;

    // read snapshots
    while( sin.eof() == false ){
        CSnapshot* p_snap = new CSnapshot(this);
        p_snap->InitSnapshot();
        if( ReadSnapshot(sin,p_snap,lineno,mod) == true ) {
            Snapshots.append(p_snap);
        } else {
            delete p_snap;
            break;
        }
    }

    // notify about changes
    GetTrajectory()->EmitOnTrajectorySegmentsChanged();
}

//------------------------------------------------------------------------------

bool CGDrvTrajSegment::ReadSnapshot(std::istream& sin,CSnapshot* p_snap,int& lineno,const QString& mod)
{
    // read geometry
    vector<CGAtom>  atoms;
    double          energy;

    while( CGaussianUtils::FindGeometryOrOptEnd(sin,lineno) ) {
        // read geometry
        atoms.clear();
        if( CGaussianUtils::ReadGeometry(sin,lineno,atoms) == false ) {
            CSmallString error;
            error << "unable to read snapshot geometry at line " << lineno;
            ES_ERROR(error);
            return(false);
        }

        // read energy
        string type;
        if( CGaussianUtils::ReadEnergy(sin,lineno,energy,type) == false ) {
            if( sin ){
                CSmallString error;
                error << "unable to read snapshot energy at line " << lineno;
                ES_ERROR(error);
            }
            return(false);
        }
    }

    // read SCD value
    double value;
    if( CGaussianUtils::ReadSCDValue(sin,lineno,mod,value) == false ) {
        if( sin ){
            CSmallString error;
            error << "unable to read coordinate value at line " << lineno;
            ES_ERROR(error);
        }
        return(false);
    }
    p_snap->SetProperty(CVKey,value*CVScaleFac);

    // update snapshot
    std::vector<CGAtom>::iterator   it = atoms.begin();
    std::vector<CGAtom>::iterator   ie = atoms.end();

    int i = 0;
    while( it != ie ){
        if( i >= GetNumberOfAtoms() ){
            ES_ERROR("more atoms than in the structure");
            return(false);
        }
        CGAtom atom = *it;
        p_snap->SetPos(i,atom.Pos);
        it++;
        i++;
    }

    // set energy
    p_snap->SetProperty(EnergyKey,energy);

    return( true );
}

//------------------------------------------------------------------------------

bool CGDrvTrajSegment::CreateGeoProperty(void)
{
    CStructure* p_str = GetStructure();
    if( p_str == NULL ) return(false);

    stringstream str(SCDDef.toStdString().c_str());
    char smod;
    int  a1,a2,a3,a4;
    str >> smod;
    switch(smod){
        case 'B':{
            CDistanceProperty* p_prop = dynamic_cast<CDistanceProperty*>(GetProject()->GetProperties()->CreateProperty(DistancePropertyID,"",""));
            str >> a1 >> a2;
            if( p_prop ){
                CAtom* p_atom;
                p_atom = p_str->GetAtomByTrajIndex(a1-1);
                if( p_atom ) p_prop->GetPointA()->AddAtom(p_atom);
                p_atom = p_str->GetAtomByTrajIndex(a2-1);
                if( p_atom ) p_prop->GetPointB()->AddAtom(p_atom);
                // create graphical representation
                p_prop->CreateGraphicsObject();
            }

            return(true);
        }
        case 'A': {
            CAngleProperty* p_prop = dynamic_cast<CAngleProperty*>(GetProject()->GetProperties()->CreateProperty(AnglePropertyID,"",""));
            str >> a1 >> a2 >> a3;
            if( p_prop ){
                CAtom* p_atom;
                p_atom = p_str->GetAtomByTrajIndex(a1-1);
                if( p_atom ) p_prop->GetPointA()->AddAtom(p_atom);
                p_atom = p_str->GetAtomByTrajIndex(a2-1);
                if( p_atom ) p_prop->GetPointB()->AddAtom(p_atom);
                p_atom = p_str->GetAtomByTrajIndex(a3-1);
                if( p_atom ) p_prop->GetPointC()->AddAtom(p_atom);
                // create graphical representation
                p_prop->CreateGraphicsObject();
            }
            return(true);
        }
        case 'D':{
            CTorsionProperty* p_prop = dynamic_cast<CTorsionProperty*>(GetProject()->GetProperties()->CreateProperty(TorsionPropertyID,"",""));
            str >> a1 >> a2 >> a3 >> a4;
            if( p_prop ){
                CAtom* p_atom;
                p_atom = p_str->GetAtomByTrajIndex(a1-1);
                if( p_atom ) p_prop->GetPointA()->AddAtom(p_atom);
                p_atom = p_str->GetAtomByTrajIndex(a2-1);
                if( p_atom ) p_prop->GetPointB()->AddAtom(p_atom);
                p_atom = p_str->GetAtomByTrajIndex(a3-1);
                if( p_atom ) p_prop->GetPointC()->AddAtom(p_atom);
                p_atom = p_str->GetAtomByTrajIndex(a4-1);
                if( p_atom ) p_prop->GetPointD()->AddAtom(p_atom);
                // create graphical representation
                p_prop->CreateGraphicsObject();
            }        
            return(true);
        }
    }
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

