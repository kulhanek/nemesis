// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@enzim.hu,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
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

#include <ProjectStatusBar.hpp>
#include <QFrame>
#include <QLabel>
#include <ProjectList.hpp>
#include <Project.hpp>
#include <HistoryList.hpp>
#include <SelectionList.hpp>
#include <JobList.hpp>
#include <SelectionHandler.hpp>
#include <QIcon>
#include <QHBoxLayout>
#include <QMovie>
#include <QFile>
#include <QTimer>
#include <MouseDriverSetup.hpp>
#include <MouseDriver.hpp>
#include <ErrorSystem.hpp>

//------------------------------------------------------------------------------

#define SB_ICON_SIZE 16

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProjectStatusBar::CProjectStatusBar(QWidget* p_parent,CProject* p_project)
    : QStatusBar(p_parent)
{
    Project = p_project;

    // flags -------------------------------------
    FlagsSection = new QFrame(NULL);
    FlagsSection->setFrameStyle(QFrame::StyledPanel);

    ChangeStatusLabel = new QLabel(FlagsSection);
    ChangeStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/Changed.png").pixmap(SB_ICON_SIZE,QIcon::Disabled));

    LockStructureStatusLabel = new QLabel("S",FlagsSection);
    LockStructureStatusLabel->setToolTip("Structures");
    LockTopologyStatusLabel = new QLabel("T",FlagsSection);
    LockTopologyStatusLabel->setToolTip("Topology");
    LockGeometryStatusLabel = new QLabel("C",FlagsSection);
    LockGeometryStatusLabel->setToolTip("Geometry");
    LockGraphicsStatusLabel = new QLabel("G",FlagsSection);
    LockGraphicsStatusLabel->setToolTip("Graphics");
    LockDescriptionStatusLabel = new QLabel("D",FlagsSection);
    LockDescriptionStatusLabel->setToolTip("Description");
    LockPropertiesStatusLabel = new QLabel("P",FlagsSection);
    LockPropertiesStatusLabel->setToolTip("Properties");

    JobStatusLabel = new QLabel(FlagsSection);
    JobStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/JobStatus.png").pixmap(SB_ICON_SIZE,QIcon::Disabled));

    QHBoxLayout* p_layout = new QHBoxLayout;
    p_layout->setContentsMargins(2,2,2,2);
    p_layout->addWidget(ChangeStatusLabel);
    p_layout->addWidget(LockStructureStatusLabel);
    p_layout->addWidget(LockTopologyStatusLabel);
    p_layout->addWidget(LockGeometryStatusLabel);
    p_layout->addWidget(LockGraphicsStatusLabel);
    p_layout->addWidget(LockDescriptionStatusLabel);
    p_layout->addWidget(LockPropertiesStatusLabel);
    p_layout->addWidget(JobStatusLabel);
    FlagsSection->setLayout(p_layout);

    // mouse -------------------------------------
    MouseSection = new QFrame(NULL);
    MouseSection->setFrameStyle(QFrame::StyledPanel);

    MouseModeLabel = new QLabel(MouseSection);
    MouseModeLabel->setToolTip(tr("Mouse mode"));

    MouseManipLevelLabel = new QLabel(MouseSection);
    MouseManipLevelLabel->setToolTip(tr("Direction of mouse manipulation"));

    PrimaryDriverLabel = new QLabel(MouseSection);
    PrimaryDriverLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/PrimaryDriver.svg").pixmap(SB_ICON_SIZE,QIcon::Disabled));
    PrimaryDriverLabel->setToolTip(tr("Primary manipulator"));

    SecondaryDriverLabel = new QLabel(MouseSection);
    SecondaryDriverLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/SecondaryDriver.svg").pixmap(SB_ICON_SIZE,QIcon::Disabled));
    SecondaryDriverLabel->setToolTip(tr("Secondary manipulator"));

    p_layout = new QHBoxLayout;
    p_layout->setContentsMargins(2,2,2,2);
    p_layout->addWidget(MouseModeLabel);
    p_layout->addWidget(MouseManipLevelLabel);
    p_layout->addWidget(PrimaryDriverLabel);
    p_layout->addWidget(SecondaryDriverLabel);
    MouseSection->setLayout(p_layout);

    // notify ------------------------------------
    NotifyLabel = new QLabel(NULL);
    NotifyLabel->setFrameStyle(QFrame::StyledPanel);
    NotifyLabel->setScaledContents(true);

    ProgressBar = new QProgressBar(NULL);

    addWidget(FlagsSection);
    addWidget(MouseSection);
    addWidget(NotifyLabel,60);

    LastTextNotificationType = ETNT_CLEAR;
    ProgressType = EPNT_END;
    TextNotificationTimer = new QTimer(this);

    // signals -----------------------------------
    connect(TextNotificationTimer,SIGNAL(timeout()),
            this,SLOT(EndOfTextNotification()));
    // -------------
    connect(Project,SIGNAL(OnStatusChanged(EStatusChanged)),
            this,SLOT(UpdateStatus(void)));
    connect(Project,SIGNAL(OnTextNotification(ETextNotificationType,const QString&,int)),
            this,SLOT(UpdateTextNotification(ETextNotificationType,const QString&,int)));
    connect(Project,SIGNAL(OnProgressNotification(EProgressNotificationType,int,const QString&)),
            this,SLOT(UpdateProgressNotification(EProgressNotificationType,int,const QString&)));
    // -------------
    connect(Project->GetSelection(),SIGNAL(OnSelectionRequestChanged(void)),
            this,SLOT(UpdateStatus(void)));
    // -------------
    connect(Project->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this,SLOT(UpdateStatus(void)));
    // -------------
    connect(Project->GetJobs(),SIGNAL(OnJobAdded(CJob*)),
            this,SLOT(UpdateStatus(void)));
    connect(Project->GetJobs(),SIGNAL(OnJobStatusChanged(CJob*)),
            this,SLOT(UpdateStatus(void)));
    connect(Project->GetJobs(),SIGNAL(OnJobRemoved(CJob*)),
            this,SLOT(UpdateStatus(void)));
    // -------------
    connect(MouseDriverSetup,SIGNAL(OnMouseSetupChanged(void)),
            this,SLOT(UpdateStatus(void)));
    connect(Project->GetMouseHandler(),SIGNAL(OnDriverStatusChanged(void)),
            this,SLOT(UpdateStatus(void)));

    UpdateStatus();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProjectStatusBar::UpdateStatus(void)
{
    if( Project == NULL ) {
        FlagsSection->hide();
        NotifyLabel->hide();
        return;
    }


    // flags -------------------------------------
    if( Project->IsFlagSet(EPOF_PROJECT_CHANGED) ) {
        ChangeStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/Changed.png").pixmap(SB_ICON_SIZE,QIcon::Normal));
    } else {
        ChangeStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/Changed.png").pixmap(SB_ICON_SIZE,QIcon::Disabled));
    }

    CLockLevels curr_levels = Project->GetHistory()->GetLockModeLevels();

    LockStructureStatusLabel->setEnabled(!curr_levels.testFlag(EHCL_STRUCTURES));
    LockTopologyStatusLabel->setEnabled(!curr_levels.testFlag(EHCL_TOPOLOGY));
    LockGeometryStatusLabel->setEnabled(!curr_levels.testFlag(EHCL_GEOMETRY));
    LockGraphicsStatusLabel->setEnabled(!curr_levels.testFlag(EHCL_GRAPHICS));
    LockDescriptionStatusLabel->setEnabled(!curr_levels.testFlag(EHCL_DESCRIPTION));
    LockPropertiesStatusLabel->setEnabled(!curr_levels.testFlag(EHCL_PROPERTY));

    if( Project->GetJobs()->children().count() > 0 ) {
        JobStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/JobStatus.png").pixmap(SB_ICON_SIZE,QIcon::Normal));
    } else {
        JobStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/JobStatus.png").pixmap(SB_ICON_SIZE,QIcon::Disabled));
    }

    if( Project->GetJobs()->children().count() > 0 ) {
        JobStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/JobStatus.png").pixmap(SB_ICON_SIZE,QIcon::Normal));
    } else {
        JobStatusLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/JobStatus.png").pixmap(SB_ICON_SIZE,QIcon::Disabled));
    }

    // mouse -------------------------------------
    switch(MouseDriverSetup->MouseMode){
        case EMM_SRT:
            MouseModeLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/SRT.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            MouseModeLabel->setToolTip(tr("Mouse mode - SRT (Select|Rotate|Translate) mode"));
            break;
        case EMM_SELECT:
            MouseModeLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/Select.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            MouseModeLabel->setToolTip(tr("Mouse mode - Select mode"));
            break;
        case EMM_ROTATE:
            MouseModeLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/Rotate.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            MouseModeLabel->setToolTip(tr("Mouse mode - Rotate mode"));
            break;
        case EMM_TRANSLATE:
            MouseModeLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/Move.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            MouseModeLabel->setToolTip(tr("Mouse mode - Translate mode"));
            break;
        case EMM_SCALE:
            MouseModeLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/Scale.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            MouseModeLabel->setToolTip(tr("Mouse mode - Scale mode"));
            break;
    }
    switch(Project->GetMouseHandler()->GetSecondaryDriver()){
        case EMD_NONE:
        case EMD_SCENE:
            SecondaryDriverLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/SecondaryDriver.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            SecondaryDriverLabel->setToolTip(tr("Secondary manipulator"));
            break;
        case EMD_ATOM_MANIP_RELAX:
            // FIXME
            break;
        case EMD_ATOM_MANIP:
            SecondaryDriverLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/AtomManipulation.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            SecondaryDriverLabel->setToolTip(tr("Atom manipulator"));
            break;
        case EMD_DIR_MANIP:
            SecondaryDriverLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/DirectionManipulation.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            SecondaryDriverLabel->setToolTip(tr("Direction manipulator"));
            break;
        case EMD_MOL_MANIP:
            SecondaryDriverLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/MoleculeManipulation.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            SecondaryDriverLabel->setToolTip(tr("Molecule manipulator"));
            break;
        case EMD_OBJ_MANIP:
            SecondaryDriverLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/ObjectManipulation.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
            SecondaryDriverLabel->setToolTip(tr("Object manipulator"));
            break;
    }

    CMouseDriver* p_drv = Project->GetMouseHandler()->GetActiveDriver();
    if( p_drv != NULL ){
        switch(p_drv->GetManipLevel()){
            case EML_YZ:
                MouseManipLevelLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/YZ_direction.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
                MouseManipLevelLabel->setToolTip(tr("YZ directions"));
                break;
            case EML_X:
                MouseManipLevelLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/X_direction.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
                MouseManipLevelLabel->setToolTip(tr("X direction"));
                break;
        }
    } else {
        MouseManipLevelLabel->setPixmap(QIcon(":/images/NemesisCore/project/ProjectStatusBar/YZ_direction.svg").pixmap(SB_ICON_SIZE,QIcon::Normal));
        MouseManipLevelLabel->setToolTip(tr("YZ directions"));
    }

    if( Project->GetMouseHandler()->IsSecondaryDriverActive() ){
        PrimaryDriverLabel->setEnabled(false);
        SecondaryDriverLabel->setEnabled(true);
    } else {
        PrimaryDriverLabel->setEnabled(true);
        SecondaryDriverLabel->setEnabled(false);
    }

    // notify ------------------------------------
    if( LastTextNotificationType == ETNT_CLEAR ) {
        CSelectionHandler* p_sh = Project->GetSelection()->GetRequestType();
        if( p_sh != NULL ) {
            QString sel_req;
            sel_req = p_sh->GetHandlerDescription();
            sel_req += " ";
            sel_req += Project->GetSelection()->GetRequestReason();
            NotifyLabel->setText(sel_req);
        } else {
            NotifyLabel->setText("");
        }
    }

    FlagsSection->show();
    NotifyLabel->show();
}

//------------------------------------------------------------------------------

void CProjectStatusBar::UpdateTextNotification(ETextNotificationType type,const QString& text,int time)
{
    LastTextNotificationType = type;
    QString full_text;

    QString style;
    switch(LastTextNotificationType){
        case ETNT_CLEAR:
        case ETNT_TEXT:
            style = "";
            full_text = text;
        break;
        case ETNT_WARNING:
            style = "color: blue;";
            ES_WARNING(text);
            full_text = "<b>" + text + "</b>";
        break;
        case ETNT_ERROR:
            style = "color: yellow; background-color: red;";
            ES_ERROR(text);
            full_text = "<b>" + text + "</b>";
        break;
    }
    NotifyLabel->setStyleSheet(style);
    if( ProgressType == EPNT_PROGRESS ){
        ProgressBar->hide();
        NotifyLabel->show();
        if( ProgressBar->parent() == this ) removeWidget(ProgressBar);
        if( NotifyLabel->parent() != this ) addWidget(NotifyLabel,60);
    }

    if( type == ETNT_CLEAR ){
        EndOfTextNotification();
        return;
    }

    NotifyLabel->setText(full_text);
    if( time > 0 ) {
        TextNotificationTimer->setSingleShot(true);
        TextNotificationTimer->start(time);
    } else {
        TextNotificationTimer->stop();
    }
}

//------------------------------------------------------------------------------

void CProjectStatusBar::EndOfTextNotification(void)
{
    LastTextNotificationType = ETNT_CLEAR;

    if( ProgressType == EPNT_PROGRESS ){
        NotifyLabel->hide();
        ProgressBar->show();
        ProgressBar->reset();
        if( NotifyLabel->parent() == this ) removeWidget(NotifyLabel);
        if( ProgressBar->parent() != this ) addWidget(ProgressBar,60);
    }

    TextNotificationTimer->stop();
    NotifyLabel->setStyleSheet("");

    if( Project != NULL ){
        CSelectionHandler* p_sh = Project->GetSelection()->GetRequestType();
        if( p_sh != NULL ) {
            QString sel_req;
            sel_req = p_sh->GetHandlerDescription();
            sel_req += " ";
            sel_req += Project->GetSelection()->GetRequestReason();
            NotifyLabel->setText(sel_req);
            return;
        }
    }

    NotifyLabel->setText("");
}

//------------------------------------------------------------------------------

void CProjectStatusBar::UpdateProgressNotification(EProgressNotificationType type,int progress,const QString& text)
{
    ProgressType = type;
    // if i have text, i wanna show it, in other case will be show only percent
    /// TODO check if its ok now - after delete commented line under this
    //if( ProgressText.isEmpty() ){
    if( !text.isEmpty() ){
        ProgressText = text;
    } else {
        ProgressText = "%p%";
    }

    switch(ProgressType){
        case EPNT_START:
            ProgressMaximum = progress;
            ProgressBar->setMaximum(ProgressMaximum);
            NotifyLabel->hide();
            ProgressBar->show();
            ProgressBar->reset();
            if( NotifyLabel->parent() == this ) removeWidget(NotifyLabel);
            if( ProgressBar->parent() != this ) addWidget(ProgressBar,60);
        break;

        case EPNT_PROGRESS:
            CurrentProgress = progress;
            ProgressBar->setFormat(ProgressText);
            ProgressBar->setValue(CurrentProgress);
        break;

        case EPNT_END:
            if( (LastTextNotificationType != ETNT_ERROR) &&
                (LastTextNotificationType != ETNT_WARNING) ){

                ProgressBar->hide();
                NotifyLabel->show();
                if( ProgressBar->parent() == this ) removeWidget(ProgressBar);
                if( NotifyLabel->parent() != this ) addWidget(NotifyLabel,60);

                if( Project != NULL ){
                    CSelectionHandler* p_sh = Project->GetSelection()->GetRequestType();
                    if( p_sh != NULL ) {
                        QString sel_req;
                        sel_req = p_sh->GetHandlerDescription();
                        sel_req += " ";
                        sel_req += Project->GetSelection()->GetRequestReason();
                        NotifyLabel->setText(sel_req);
                        return;
                    }
                }

                NotifyLabel->setText("");
            }
        break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
