// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <QSvgWidget>

#include <PluginObject.hpp>
#include <ProjectList.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>
#include <NemesisCoreModule.hpp>
#include <OpenBabelUtils.hpp>

#include <ErrorSystem.hpp>
#include <PeriodicTable.hpp>

#include <Structure.hpp>
#include <Project.hpp>
#include <Atom.hpp>
#include <AtomList.hpp>
#include <ResidueList.hpp>
#include <BondList.hpp>
#include <RestraintList.hpp>
#include <ContainerModel.hpp>

#include <PODesignerGeneral.hpp>
#include <PODesignerFlags.hpp>
#include <PODesignerRefBy.hpp>

#include <Graphics.hpp>
#include <GraphicsProfileList.hpp>
#include <GraphicsObject.hpp>
#include <GraphicsObjectList.hpp>

#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <OpenBabelUtils.hpp>
#include <Trajectory.hpp>

#include <StructureDesigner.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* StructureDesignerCB(void* p_data);

CExtUUID        StructureDesignerID(
                    "{STRUCTURE_DESIGNER:c4db7f4b-1ec3-4493-886b-397aa797149b}",
                    "Structure");

CPluginObject   StructureDesignerObject(&NemesisCorePlugin,
                    StructureDesignerID,DESIGNER_CAT,
                    ":/images/NemesisCore/structure/Structure.svg",
                    StructureDesignerCB);

// -----------------------------------------------------------------------------

QObject* StructureDesignerCB(void* p_data)
{
    CStructure* p_str = static_cast<CStructure*>(p_data);
    QObject* p_object = new CStructureDesigner(p_str);
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureDesigner::CStructureDesigner(CStructure* p_str)
    : CProObjectDesigner(&StructureDesignerObject,p_str)
{
    Object = p_str;
    WidgetUI.setupUi(this);

    // attached common setup part ----------------
    General = new CPODesignerGeneral(WidgetUI.generalGB,Object,this);
    Flags = new CPODesignerFlags(WidgetUI.flagsGB,Object,this);
    RefBy = new CPODesignerRefBy(WidgetUI.refByTab,Object,this);

    // init models -------------------------------
    StatModel = new QStandardItemModel(WidgetUI.statTV);
    WidgetUI.statTV->setModel(StatModel);

    // create headers
    QStringList  header;
    header << tr("Atom") << tr("Count") << tr("Mole fraction") << tr("Mass fraction");
    StatModel->setHorizontalHeaderLabels(header);
    if( StatModel != NULL ){
        for(int i=0; i < StatModel->columnCount(); i++){
            WidgetUI.statTV->resizeColumnToContents(i);
        }
    }

    ResiduesModel = Object->GetResidues()->GetContainerModel(this);
    WidgetUI.residuesTV->setModel(ResiduesModel);
    WidgetUI.residuesTV->setDragDropMode(QAbstractItemView::DragOnly);
    WidgetUI.residuesTV->setDragEnabled(true);

    if( ResiduesModel != NULL ){
        for(int i=0; i < ResiduesModel->columnCount(); i++){
            WidgetUI.residuesTV->resizeColumnToContents(i);
        }
    }

    AtomsModel = Object->GetAtoms()->GetContainerModel(this);
    WidgetUI.atomsTV->setModel(AtomsModel);
    WidgetUI.atomsTV->setDragDropMode(QAbstractItemView::DragOnly);
    WidgetUI.atomsTV->setDragEnabled(true);

    if( AtomsModel != NULL ){
        for(int i=0; i < AtomsModel->columnCount(); i++){
            WidgetUI.atomsTV->resizeColumnToContents(i);
        }
    }

    BondsModel = Object->GetBonds()->GetContainerModel(this);
    WidgetUI.bondsTV->setModel(BondsModel);
    WidgetUI.bondsTV->setDragDropMode(QAbstractItemView::DragOnly);
    WidgetUI.bondsTV->setDragEnabled(true);

    if( BondsModel != NULL ){
        for(int i=0; i < BondsModel->columnCount(); i++){
            WidgetUI.bondsTV->resizeColumnToContents(i);
        }
    }

    RestraintsModel = Object->GetRestraints()->GetContainerModel(this);
    WidgetUI.restraintsTV->setModel(RestraintsModel);
    WidgetUI.restraintsTV->setDragDropMode(QAbstractItemView::DragOnly);
    WidgetUI.restraintsTV->setDragEnabled(true);

    if( RestraintsModel != NULL ){
        for(int i=0; i < RestraintsModel->columnCount(); i++){
            WidgetUI.restraintsTV->resizeColumnToContents(i);
        }
    }

    // trajectory
    if( Object->GetProject()->GetTrajectories() ){
        WidgetUI.trajectoryGB->show();
    } else {
        WidgetUI.trajectoryGB->hide();
    }
    WidgetUI.trajectoryLO->setProject(Object->GetProject());
    WidgetUI.trajectoryLO->setObjectBaseMIMEType("trajectory.indexes");
    WidgetUI.trajectoryLO->setSelectionHandler(NULL);
    WidgetUI.trajectoryLO->showRemoveButton(true);

    // external events ---------------------------
    connect(Object, SIGNAL(OnStatusChanged(EStatusChanged)),
            this, SLOT(InitValues()));
    //--------------
    if( Object->GetProject() ){
        connect(Object->GetProject()->GetHistory(), SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
                this, SLOT(ProjectLockChanged(EHistoryChangeMessage)));
    }

    // change events -----------------------------
    connect(WidgetUI.totalChargeSB, SIGNAL(valueChanged(int)),
            this, SLOT(SetChangedFlagTrue()));
    connect(WidgetUI.seqIndexSB, SIGNAL(valueChanged(int)),
            this, SLOT(SetChangedFlagTrue()));
    connect(WidgetUI.trajectoryLO, SIGNAL(OnObjectChanged(void)),
            this, SLOT(SetChangedFlagTrue()));

    connect(WidgetUI.aSB, SIGNAL(valueChanged(double)),
            this, SLOT(FlagGeometryChanged()));
    connect(WidgetUI.bSB, SIGNAL(valueChanged(double)),
            this, SLOT(FlagGeometryChanged()));
    connect(WidgetUI.cSB, SIGNAL(valueChanged(double)),
            this, SLOT(FlagGeometryChanged()));
    connect(WidgetUI.alphaSB, SIGNAL(valueChanged(double)),
            this, SLOT(FlagGeometryChanged()));
    connect(WidgetUI.betaSB, SIGNAL(valueChanged(double)),
            this, SLOT(FlagGeometryChanged()));
    connect(WidgetUI.gammaSB, SIGNAL(valueChanged(double)),
            this, SLOT(FlagGeometryChanged()));

    connect(WidgetUI.aPeriodicCB, SIGNAL(clicked(bool)),
            this, SLOT(FlagGeometryChanged()));
    connect(WidgetUI.bPeriodicCB, SIGNAL(clicked(bool)),
            this, SLOT(FlagGeometryChanged()));
    connect(WidgetUI.cPeriodicCB, SIGNAL(clicked(bool)),
            this, SLOT(FlagGeometryChanged()));

    // statistics ----------------
    connect(Object->GetResidues(), SIGNAL(OnResidueListChanged()),
            this, SLOT(UpdateStatistics()));
    connect(Object->GetAtoms(), SIGNAL(OnAtomListChanged()),
            this, SLOT(UpdateStatistics()));
    connect(Object->GetBonds(), SIGNAL(OnBondListChanged()),
            this, SLOT(UpdateStatistics()));
    connect(Object->GetRestraints(), SIGNAL(OnRestraintListChanged()),
            this, SLOT(UpdateStatistics()));

    // InChI
    connect(WidgetUI.generateInChIPB, SIGNAL(clicked(bool)),
            this, SLOT(GenerateInChI(void)));

    // designer from TreeView ----
    connect(WidgetUI.residuesTV, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(ObjectTVDblClicked(const QModelIndex&)));
    connect(WidgetUI.atomsTV, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(ObjectTVDblClicked(const QModelIndex&)));
    connect(WidgetUI.bondsTV, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(ObjectTVDblClicked(const QModelIndex&)));
    connect(WidgetUI.restraintsTV, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(ObjectTVDblClicked(const QModelIndex&)));

    // actions -------------------
    connect(WidgetUI.boundingBoxTB, SIGNAL(clicked()),
            this, SLOT(SetFromBoundingBox()));
    // ------------
    connect(WidgetUI.showBoxPB, SIGNAL(clicked()),
            this, SLOT(ShowBox()));
    // ------------
    connect(WidgetUI.moveCOMToBoxCenterPB, SIGNAL(clicked()),
            this, SLOT(MoveCOMToBoxCenter()));
    // ------------
    connect(WidgetUI.boundinBoxAroundBoxCenterPB, SIGNAL(clicked()),
            this, SLOT(BoundinBoxAroundBoxCenter()));
    // ------------
    connect(WidgetUI.buttonBox_1, SIGNAL(clicked (QAbstractButton *)),
            SLOT(ButtonBoxClicked(QAbstractButton *)));
    // ------------
    connect(WidgetUI.buttonBox_2, SIGNAL(clicked (QAbstractButton *)),
            SLOT(ButtonBoxClicked(QAbstractButton *)));

    // units
    WidgetUI.aSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.bSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.cSB->setPhysicalQuantity(PQ_DISTANCE);

    WidgetUI.alphaSB->setPhysicalQuantity(PQ_ANGLE);
    WidgetUI.betaSB->setPhysicalQuantity(PQ_ANGLE);
    WidgetUI.gammaSB->setPhysicalQuantity(PQ_ANGLE);

    WidgetUI.axLE->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.ayLE->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.azLE->setPhysicalQuantity(PQ_DISTANCE);

    WidgetUI.bxLE->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.byLE->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.bzLE->setPhysicalQuantity(PQ_DISTANCE);

    WidgetUI.cxLE->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.cyLE->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.czLE->setPhysicalQuantity(PQ_DISTANCE);

    WidgetUI.volumeLE->setPhysicalQuantity(PQ_VOLUME);
    WidgetUI.radiusLE->setPhysicalQuantity(PQ_DISTANCE);

    WidgetUI.massLE->setPhysicalQuantity(PQ_MASS);

    WidgetUI.bufferSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.bufferSB->setInternalValue(2.8);

    // initial values
    ProjectLockChanged(EHCM_LOCK_LEVEL);
    InitAllValues();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureDesigner::ButtonBoxClicked(QAbstractButton *button)
{
    if( (WidgetUI.buttonBox_1->standardButton(button) == QDialogButtonBox::Reset) ||
        (WidgetUI.buttonBox_2->standardButton(button) == QDialogButtonBox::Reset) ) {
        InitAllValues();
        return;
    }

    if( (WidgetUI.buttonBox_1->standardButton(button) == QDialogButtonBox::Apply) ||
        (WidgetUI.buttonBox_2->standardButton(button) == QDialogButtonBox::Apply) ) {
        ApplyAllValues();
        return;
    }

    if( (WidgetUI.buttonBox_1->standardButton(button) == QDialogButtonBox::Close) ||
        (WidgetUI.buttonBox_2->standardButton(button) == QDialogButtonBox::Close) ) {
        close();
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureDesigner::InitAllValues(void)
{
    if( IsItChangingContent() == true ) return;

    General->InitValues();
    Flags->InitValues();
    RefBy->InitValues();
    InitValues();

    SetChangedFlag(false);
}

//------------------------------------------------------------------------------

void CStructureDesigner::ApplyAllValues(void)
{
    if( IsChangedFlagSet() == false ) return;

    if( Object->BeginChangeWH(EHCL_COMPOSITE,Object->CComObject::GetType().GetName()) == NULL ) return;

    Changing = true;
        General->ApplyValues();
        Flags->ApplyValues();
        RefBy->ApplyValues();
        ApplyValues();
    Changing = false;

    Object->EndChangeWH(); // this also repaint the project

    // some changes can be prohibited - reinit visualization
    InitAllValues();

    // do not repaint project here, it is done in EndChangeWH
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureDesigner::InitValues(void)
{
    if( IsItChangingContent() ) return;

    WidgetUI.totalChargeSB->setValue(Object->GetTotalCharge());
    WidgetUI.seqIndexSB->setValue(Object->GetSeqIndex());
    WidgetUI.trajectoryLO->setObject(Object->GetTrajectory());

    // box info
    PBCInfo = Object->PBCInfo;

    if( PBCInfo.IsValid() ){
        WidgetUI.validLA->hide();
    } else {
        WidgetUI.validLA->show();
    }

    WidgetUI.aSB->setInternalValue(PBCInfo.GetAVectorSize());
    WidgetUI.bSB->setInternalValue(PBCInfo.GetBVectorSize());
    WidgetUI.cSB->setInternalValue(PBCInfo.GetCVectorSize());

    WidgetUI.alphaSB->setInternalValue(PBCInfo.GetAlpha());
    WidgetUI.betaSB->setInternalValue(PBCInfo.GetBeta());
    WidgetUI.gammaSB->setInternalValue(PBCInfo.GetGamma());

    WidgetUI.aPeriodicCB->setChecked(PBCInfo.IsPeriodicAlongA());
    WidgetUI.bPeriodicCB->setChecked(PBCInfo.IsPeriodicAlongB());
    WidgetUI.cPeriodicCB->setChecked(PBCInfo.IsPeriodicAlongC());

    CPoint vec = PBCInfo.GetAVector();
    WidgetUI.axLE->setInternalValue(vec.x);
    WidgetUI.ayLE->setInternalValue(vec.y);
    WidgetUI.azLE->setInternalValue(vec.z);

    vec = PBCInfo.GetBVector();
    WidgetUI.bxLE->setInternalValue(vec.x);
    WidgetUI.byLE->setInternalValue(vec.y);
    WidgetUI.bzLE->setInternalValue(vec.z);

    vec = PBCInfo.GetCVector();
    WidgetUI.cxLE->setInternalValue(vec.x);
    WidgetUI.cyLE->setInternalValue(vec.y);
    WidgetUI.czLE->setInternalValue(vec.z);

    WidgetUI.volumeLE->setInternalValue(PBCInfo.GetVolume());
    WidgetUI.radiusLE->setInternalValue(PBCInfo.GetLargestSphereRadius());

    UpdateStatistics();

    SetChangedFlag(false);
}

//------------------------------------------------------------------------------

void CStructureDesigner::ApplyValues(void)
{
    Object->SetTotalCharge(WidgetUI.totalChargeSB->value());
    Object->SetSeqIndexWH(WidgetUI.seqIndexSB->value());
    Object->SetTrajectoryWH(WidgetUI.trajectoryLO->getObject<CTrajectory*>());

    // box
    CPoint sizes;
    sizes.x = WidgetUI.aSB->getInternalValue();
    sizes.y = WidgetUI.bSB->getInternalValue();
    sizes.z = WidgetUI.cSB->getInternalValue();

    CPoint angles;
    angles.x = WidgetUI.alphaSB->getInternalValue();
    angles.y = WidgetUI.betaSB->getInternalValue();
    angles.z = WidgetUI.gammaSB->getInternalValue();

    bool   pa = WidgetUI.aPeriodicCB->isChecked();
    bool   pb = WidgetUI.bPeriodicCB->isChecked();
    bool   pc = WidgetUI.cPeriodicCB->isChecked();

    Object->SetBoxWH(pa,pb,pc,sizes,angles);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureDesigner::OpenProjectDesigner(void)
{
    CProject* p_proj;
    if((p_proj = Object->GetProject()) == NULL) {
        LOGIC_ERROR("Project == NULL");
    }
    p_proj->OpenObjectDesigner();
}

//------------------------------------------------------------------------------

void CStructureDesigner::ObjectTVDblClicked(const QModelIndex& index)
{
    QAbstractItemModel* p_amodel = (QAbstractItemModel*)(index.model());
    CContainerModel* p_model = dynamic_cast<CContainerModel*>(p_amodel);
    if( p_model == NULL ) return;
    CProObject* p_obj = dynamic_cast<CProObject*>(p_model->GetItem(index));
    if( p_obj == NULL ) return;
    p_obj->OpenObjectDesigner();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureDesigner::UpdateStatistics(void)
{
    // invalidate inchi
    WidgetUI.inChIKeyLE->setText("");
    WidgetUI.inChITE->setText("");

    // update counters
    int     num;
    int     atomNum;
    QString snum;
    num = Object->GetResidues()->children().count();
    snum.setNum(num);
    WidgetUI.residuesLE->setText(snum);

    atomNum = Object->GetAtoms()->children().count();
    snum.setNum(atomNum);
    WidgetUI.atomsLE->setText(snum);

    num = Object->GetBonds()->children().count();
    snum.setNum(num);
    WidgetUI.bondsLE->setText(snum);

    num = Object->GetRestraints()->children().count();
    snum.setNum(num);
    WidgetUI.restraintsLE->setText(snum);

    StatModel->removeRows(0,StatModel->rowCount());

    double massTot = 0.0;

    // list of atomic numbers Z and element counters count
    std::vector<int> Z;
    // order C, H, O, S, others (z = 6, 1, 8, 16, others)
    Z.push_back(6);
    Z.push_back(1);
    Z.push_back(8);
    Z.push_back(16);

    std::vector<int> count(4, 0);

    // calculate statistics
    unsigned int i;
    foreach(QObject* p_qobj,Object->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        // set i to proper element
        i = 0;
        while(i < Z.size() && Z[i] != p_atom->GetZ()) {
            i++;
        }
        // element not found - create new
        if(i >= Z.size()) {
            Z.push_back(p_atom->GetZ());
            count.push_back(0);
        }
        // set values
        count[i]++;
        massTot += PeriodicTable.GetMass(Z[i]);
    }

    // 2 SUMMARY FORMULA
    // extract information and put to window
    double massF, moleF;
    i = 0;
    QList<QStandardItem*> list;
    // formula: <h1>CH<sub>4</sub></h1>
    QString formula("<H1>");
    while(i < Z.size()) {
        if(count[i] == 0) {
            i++;
            continue;
        }
        //! draw statistics table
        // element symbol item
        QStandardItem* symbolItem = new QStandardItem(QString(PeriodicTable.GetSymbol(Z[i])));
        symbolItem->setEditable(false);
        // element count item
        QStandardItem* countItem = new QStandardItem(QString().setNum(count[i]));
        countItem->setEditable(false);
        // molar fraction of element
        moleF = count[i] / (double)atomNum;
        QStandardItem* moleFItem = new QStandardItem(QString().setNum(moleF,'f',3));
        moleFItem->setEditable(false);
        // mass fraction of element = (elementCount*elementMass/totalMass)
        massF =  (count[i] * PeriodicTable.GetMass(Z[i])) / massTot;
        QStandardItem* massFItem = new QStandardItem(QString().setNum(massF,'f',3));
        massFItem->setEditable(false);
        list << symbolItem << countItem << moleFItem << massFItem;

        StatModel->appendRow(list);
        list.clear();
        //! draw formula
        formula.append(QString(PeriodicTable.GetSymbol(Z[i])));
        if(count[i] > 1) {
            formula.append("<sub>");
            formula.append(QString().setNum(count[i]));
            formula.append("</sub>");
        }
        i++;
    }
    formula.append("</H1>");

    WidgetUI.formulaLA->setText(formula);
    WidgetUI.massLE->setInternalValue(massTot);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureDesigner::FlagGeometryChanged(void)
{
    bool   pa = WidgetUI.aPeriodicCB->isChecked();
    bool   pb = WidgetUI.bPeriodicCB->isChecked();
    bool   pc = WidgetUI.cPeriodicCB->isChecked();

    PBCInfo.SetPeriodicity(pa,pb,pc);

    CPoint sizes;
    sizes.x = WidgetUI.aSB->getInternalValue();
    sizes.y = WidgetUI.bSB->getInternalValue();
    sizes.z = WidgetUI.cSB->getInternalValue();

    CPoint angles;
    angles.x = WidgetUI.alphaSB->getInternalValue();
    angles.y = WidgetUI.betaSB->getInternalValue();
    angles.z = WidgetUI.gammaSB->getInternalValue();

    PBCInfo.SetDimmensions(sizes,angles);

    if( PBCInfo.IsValid() ){
        WidgetUI.validLA->hide();
    } else {
        WidgetUI.validLA->show();
    }

    sizes = PBCInfo.GetAVector();
    WidgetUI.axLE->setInternalValue(sizes.x);
    WidgetUI.ayLE->setInternalValue(sizes.y);
    WidgetUI.azLE->setInternalValue(sizes.z);

    sizes = PBCInfo.GetBVector();
    WidgetUI.bxLE->setInternalValue(sizes.x);
    WidgetUI.byLE->setInternalValue(sizes.y);
    WidgetUI.bzLE->setInternalValue(sizes.z);

    sizes = PBCInfo.GetCVector();
    WidgetUI.cxLE->setInternalValue(sizes.x);
    WidgetUI.cyLE->setInternalValue(sizes.y);
    WidgetUI.czLE->setInternalValue(sizes.z);

    WidgetUI.volumeLE->setInternalValue(PBCInfo.GetVolume());
    WidgetUI.radiusLE->setInternalValue(PBCInfo.GetLargestSphereRadius());

    SetChangedFlag(true);
}

//------------------------------------------------------------------------------

void CStructureDesigner::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;
    WidgetUI.boxWidget->setEnabled(! Object->IsHistoryLocked(EHCL_GEOMETRY));
    WidgetUI.orderGB->setEnabled(! Object->IsHistoryLocked(EHCL_TOPOLOGY));
}

//------------------------------------------------------------------------------

void CStructureDesigner::SetFromBoundingBox(void)
{
    double buffer = WidgetUI.bufferSB->getInternalValue();

    CObjMetrics metrics;
    Object->GetObjectMetrics(metrics);
    CPoint sizes;

    if( metrics.IsValid() ){
        sizes = metrics.GetHighPoint()-metrics.GetLowPoint();
        sizes.x += buffer;
        sizes.y += buffer;
        sizes.z += buffer;
        CPoint angles;
        angles.x = M_PI/2.0;
        angles.y = M_PI/2.0;
        angles.z = M_PI/2.0;
        PBCInfo.SetPeriodicity(true,true,true);
        PBCInfo.SetDimmensions(sizes,angles);
    }

    WidgetUI.aSB->blockSignals(true);
    WidgetUI.bSB->blockSignals(true);
    WidgetUI.cSB->blockSignals(true);
    WidgetUI.alphaSB->blockSignals(true);
    WidgetUI.betaSB->blockSignals(true);
    WidgetUI.gammaSB->blockSignals(true);
    WidgetUI.aPeriodicCB->blockSignals(true);
    WidgetUI.bPeriodicCB->blockSignals(true);
    WidgetUI.cPeriodicCB->blockSignals(true);


    WidgetUI.aSB->setInternalValue(PBCInfo.GetAVectorSize());
    WidgetUI.bSB->setInternalValue(PBCInfo.GetBVectorSize());
    WidgetUI.cSB->setInternalValue(PBCInfo.GetCVectorSize());

    WidgetUI.alphaSB->setInternalValue(PBCInfo.GetAlpha());
    WidgetUI.betaSB->setInternalValue(PBCInfo.GetBeta());
    WidgetUI.gammaSB->setInternalValue(PBCInfo.GetGamma());

    WidgetUI.aPeriodicCB->setChecked(PBCInfo.IsPeriodicAlongA());
    WidgetUI.bPeriodicCB->setChecked(PBCInfo.IsPeriodicAlongB());
    WidgetUI.cPeriodicCB->setChecked(PBCInfo.IsPeriodicAlongC());

    WidgetUI.aPeriodicCB->blockSignals(false);
    WidgetUI.bPeriodicCB->blockSignals(false);
    WidgetUI.cPeriodicCB->blockSignals(false);
    WidgetUI.alphaSB->blockSignals(false);
    WidgetUI.betaSB->blockSignals(false);
    WidgetUI.gammaSB->blockSignals(false);
    WidgetUI.aSB->blockSignals(false);
    WidgetUI.bSB->blockSignals(false);
    WidgetUI.cSB->blockSignals(false);

    FlagGeometryChanged();
}

//------------------------------------------------------------------------------

void CStructureDesigner::ShowBox(void)
{
    CGraphics* p_graphics = NULL;
    if( Object->GetProject() ){
        p_graphics = Object->GetProject()->GetGraphics();
    }
    if( p_graphics == NULL ) return;

    CGraphicsProfile* p_profile = p_graphics->GetProfiles()->GetActiveProfile();
    if( p_profile == NULL ){
        ES_ERROR("no active profile");
        return;
    }

    // apply values first to prevent any further problems
    ApplyAllValues();

    CExtUUID pbcbox_id("{PBC_BOX_OBJECT:2929a64d-921a-414b-8dcd-c81e53881ce1}");

    CHistoryNode* p_hnode = Object->BeginChangeWH(EHCL_GRAPHICS,"show PBC box");
    if( p_hnode == NULL ) return;

        CGraphicsObject* p_obj = p_graphics->GetObjects()->CreateObject(pbcbox_id,QString(),QString(),p_hnode);
        if( p_obj == NULL ) {
            CSmallString error;
            error << "unable to create object " << pbcbox_id.GetStringForm();
            RUNTIME_ERROR(error);
        }
        p_obj->AddObject(Object,p_hnode);

        p_profile->AddObject(p_obj,-1,p_hnode);

    Object->EndChangeWH();
}

//------------------------------------------------------------------------------

void CStructureDesigner::MoveCOMToBoxCenter(void)
{
    Object->GetAtoms()->MoveCOMToWH(EGO_BOX_CENTER,EGS_ALL_ATOMS,true);
}

//------------------------------------------------------------------------------

void CStructureDesigner::BoundinBoxAroundBoxCenter(void)
{
    CObjMetrics metrics;
    Object->GetObjectMetrics(metrics);
    CPoint pos;

    if( metrics.IsValid() ){
        pos = metrics.GetHighPoint()+metrics.GetLowPoint();
        pos /= 2.0;
        pos = PBCInfo.GetBoxCenter() - pos;
    }

    Object->GetAtoms()->MoveByWH(pos,EGS_ALL_ATOMS);
}

//------------------------------------------------------------------------------

void CStructureDesigner::GenerateInChI(void)
{
    QString inchi;
    QString inchikey;
    COpenBabelUtils::GetInChI(Object,inchi,inchikey);
    WidgetUI.inChITE->setText(inchi);
    WidgetUI.inChIKeyLE->setText(inchikey);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




