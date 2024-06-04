// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <stdio.h>
#include <stdlib.h>

#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <Terminal.hpp>

#include <NemesisJScript.hpp>

#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <WelcomeWindow.hpp>
#include <AboutWindow.hpp>

#include <GlobalSetup.hpp>
#include <PluginDatabase.hpp>

#include <iostream>
#include <iomanip>

#include <JScriptFunctions.hpp>
#include <RootList.hpp>
#include <GlobalObjectList.hpp>
#include <PhysicalQuantities.hpp>
#include <WorkPanelList.hpp>
#include <GlobalDesktop.hpp>
#include <ProjectList.hpp>
#include <GraphicsSetupProfile.hpp>
#include <ElementColorsList.hpp>
#include <InserterList.hpp>
#include <JobScheduler.hpp>
#include <OptimizerSetupList.hpp>
#include <Project.hpp>
#include <RecentFileList.hpp>
#include <QMessageBox>
#include <BatchJobList.hpp>

#include <FTGLFontCache.hpp>

#include <MouseDriverSetup.hpp>

#include <openbabel/plugin.h>
#include <openbabel/obconversion.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace OpenBabel;

//------------------------------------------------------------------------------

CNemesisJScript*    NemesisJScript = NULL;
CTerminalOStream    tout;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CNemesisPlugNotification : public OpenBabel::OBPlugNotification {
public:
    CNemesisPlugNotification(CWelcomeWindow* p_pw,const QString& base_text);
    virtual ~CNemesisPlugNotification(){}
    virtual void notify(const std::string& text);
private:
   CWelcomeWindow*  ProgressWindow;
   QString          BaseText;
};

//------------------------------------------------------------------------------

CNemesisPlugNotification::CNemesisPlugNotification(CWelcomeWindow* p_pw,const QString& base_text)
{
    ProgressWindow = p_pw;
    BaseText = base_text;
}

//------------------------------------------------------------------------------

void CNemesisPlugNotification::notify(const std::string& text)
{
    if( ProgressWindow ){
        QString file(text.c_str());
        QFileInfo finfo(file);
        QString msg;
        msg = BaseText.arg(finfo.baseName());
        ProgressWindow->ChangeNotification(msg);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CNemesisJScript::CNemesisJScript(void)
{
    NemesisJScript = this;
    InitStatus = SO_CONTINUE;
}

//------------------------------------------------------------------------------

CNemesisJScript::~CNemesisJScript(void)
{
    NemesisJScript = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CNemesisJScript::Init(int argc,char* argv[])
{
    // encode program options
    int result = NemesisOptions.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    PrintWelcomeText();

    qApp->setWindowIcon(QIcon(":/images/NemesisCore/MainIcon.svg"));

    // initialize terminal output device to standard output stream
    tout.SetDevice(&cout);

    // init subsystems
    result = InitSubsystems();

    InitStatus = result;
    return( result );
}

//------------------------------------------------------------------------------

bool CNemesisJScript::Run(void)
{
    // run GUI event loop - only when some project or WP is opened
    bool result = true;
    if( (Projects->GetNumberOfProjects() > 0) || (WorkPanels->GetNumberOfWorkPanels() > 0) ){
        result = QApplication::exec() != 0;
    }
    return(result);
}

//------------------------------------------------------------------------------

void CNemesisJScript::Finalize(void)
{
    if( InitStatus == SO_CONTINUE ){
        // save only if initialization was successfull
        FinalizeSubsystems();
    }

    if( hasUncaughtException() == true ) {
        QScriptValue evalue = uncaughtException();
        QTextStream stream(stderr);
        stream << "nemesis: line " << uncaughtExceptionLineNumber() << " - " << evalue.toString() << Qt::endl;
    }

    PrintFinalText();

    if( ErrorSystem.IsError() || NemesisOptions.GetOptVerbose() ){
        ErrorSystem.PrintErrors(stderr);
    }

    cout << endl;
}

//------------------------------------------------------------------------------

void CNemesisJScript::PostCloseEvent(void)
{
    emit OnPostCloseEvent();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CNemesisJScript::PrintWelcomeText(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    CSmallString time_and_date;
    time_and_date = dt.GetSDateAndTime();

    cout << endl;
    cout << "# ==============================================================================" << endl;
    cout << "#                     NEMESIS - A Molecular Modeling Package                     " << endl;
    cout << "# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    cout << "# Started at : " << time_and_date << endl;
    cout << "# ==============================================================================" << endl;
}

//------------------------------------------------------------------------------

void CNemesisJScript::PrintFinalText(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    CSmallString time_and_date;
    time_and_date = dt.GetSDateAndTime();

    cout << endl;
    cout << "# ==============================================================================" << endl;
    cout << "# Finished at : " << time_and_date << endl;
    cout << "# ==============================================================================" << endl;

    int numofobjects = PluginDatabase.GetNumberOfOpenedObjects();
    cout << "PLUSULA: Opened objects = " << numofobjects << endl;
}

//------------------------------------------------------------------------------

int CNemesisJScript::InitSubsystems(void)
{
    QApplication::setWindowIcon(QIcon(":/images/NemesisCore/MainIcon.svg"));

    bool gui = true;
    if( (NemesisOptions.IsOptPrintIFormatsSet()) || (NemesisOptions.IsOptPrintOBExtensionsSet()) ){
        gui = false;
    }

    CWelcomeWindow* p_ww = NULL;

    if( gui ){
        // create welcome window
        p_ww = new CWelcomeWindow();
        p_ww->SetMaxProgressSteps(9);
        connect(this,SIGNAL(SetupLevelChanged(const QString&)),p_ww,SLOT(IncrementProgress(const QString&)));
        connect(this,SIGNAL(SetupNotificationChanged(const QString&)),p_ww,SLOT(ChangeNotification(const QString&)));
    } else {
        connect(this,SIGNAL(SetupLevelChanged(const QString&)),this,SLOT(IncrementProgressNoGUI(const QString&)));
        cout << endl;
        cout << "Initializing Nemesis subsystems ..." << endl;
    }

    emit SetupLevelChanged(tr("Global objects ...."));
    GlobalSetup = new CGlobalSetup();
    GlobalSetup->Init();

    if( NemesisOptions.GetOptKeepLastDir() == false ){
        QString pwd = QDir::currentPath();
        GlobalSetup->SetLastOpenFilePath(pwd,GenericProjectID);
    }

    if( NemesisOptions.GetOptVerbose() ){
        GlobalSetup->PrintSetup();
    }

    RootList = new CRootList(NULL);
    Projects = new CProjectList(RootList);
    GlobalObjectList = new CGlobalObjectList(RootList);
    GraphicsSetupProfile = new CGraphicsSetupProfile(GlobalObjectList);
    WorkPanels = new CWorkPanelList(GlobalObjectList);
    PhysicalQuantities = new CPhysicalQuantities(GlobalObjectList);
    Inserters = new CInserterList(GlobalObjectList);
    OptimizerSetups = new COptimizerSetupList(GlobalObjectList);
    JobScheduler = new CJobScheduler(GlobalObjectList);
    GlobalDesktop = new CGlobalDesktop(GlobalObjectList);
    MouseDriverSetup = new CMouseDriverSetup(GlobalObjectList);
    RecentFiles = new CRecentFileList(GlobalObjectList);
    BatchJobs = new CBatchJobList(GlobalObjectList);

    // events
    connect(this,SIGNAL(OnPostCloseEvent(void)),
            this,SLOT(QuitApplication()),Qt::QueuedConnection);

    // register JScript objects
    RegisterObjects();

    emit SetupLevelChanged(tr("Loading Nemesis plugins ...."));
    connect(&PluginDatabase,SIGNAL(PluginLoaded(const QString&)),this,SLOT(PluginLoaded(const QString&)));
    PluginDatabase.SetPluginPath(GlobalSetup->GetPluginsLocation());
    PluginDatabase.LoadPlugins(GlobalSetup->GetPluginsSetup());

    // do we have any plugin loaded?
    if( PluginDatabase.GetNumberOfLoadedModules() == 0 ){
        ES_ERROR("no plugin modules are loaded");
        QMessageBox::critical(NULL,tr("Error"),
                              tr("No Nemesis plugins are loaded! Did you correctly install or activate Nemesis plugins?"),
                              QMessageBox::Ok,QMessageBox::Ok);
        return(SO_USER_ERROR);
    }

    emit SetupLevelChanged(tr("Physical quantities ...."));
    PhysicalQuantities->LoadConfig();

    emit SetupLevelChanged(tr("Graphics setup ...."));
    GraphicsSetupProfile->InitAllSetupObjects();
    GraphicsSetupProfile->LoadDefaultConfig();
    ColorsList.LoadColors();

    emit SetupLevelChanged(tr("Build setup ...."));
    Inserters->LoadConfig();
    OptimizerSetups->LoadConfig();

    emit SetupLevelChanged(tr("Job scheduler ...."));
    JobScheduler->LoadConfig();
    JobScheduler->StartScheduler();

    emit SetupLevelChanged(tr("Init OpenBabel ...."));
    CNemesisPlugNotification* p_obn = NULL;
    if( gui ) {
       p_obn = new CNemesisPlugNotification(p_ww,tr("Loading OpenBabel plugin %1 ...."));
    }
    OpenBabel::OBPlugNotification::PlugNotification = p_obn;
    OpenBabel::OBPlugin::LoadAllPlugins();    // this will initialize OBPlugin subsystem
    OpenBabel::OBPlugNotification::PlugNotification = NULL;
    delete p_obn;

    emit SetupLevelChanged(tr("Global desktop ...."));
    RecentFiles->Load();
    GlobalDesktop->LoadDefaultDesktop();
    MouseDriverSetup->LoadUserSetup();

    // OpenGLDebugging
    if( NemesisOptions.GetOptGLDebug() ){
        cout << "OpenGLDebug: requested" << endl;
    }

    emit SetupLevelChanged("All done.");

    // force quit the window
    QGuiApplication::sync();

    bool exit = false;

    if( NemesisOptions.IsOptPrintIFormatsSet() ){
        PrintIFormats();
        exit = true;
    }

    if( NemesisOptions.IsOptPrintOBExtensionsSet() ){
        PrintOBExtensions();
        exit = true;
    }

    if( exit ){
        QApplication::quit();
        return(SO_CONTINUE);
    }

    // and finaly create initial project and restore global workpanels
    if( NemesisOptions.GetNumberOfProgArgs() == 0 ) {
        // only if no extra argument is provided
        GlobalDesktop->RestoreInitialProject();
    } else {
        ParseArguments();
    }

    GlobalDesktop->RestoreWorkPanels();

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

void CNemesisJScript::PrintIFormats(void)
{
    cout << endl;
    cout << "# Structure import formats ..." << endl;
    cout << endl;    
    cout << "# Format         Plugin         Description                                     " << endl;
    cout << "# -------- -------------------- ------------------------------------------------" << endl;

    CSimpleIteratorC<CPluginObject>    I(PluginDatabase.GetObjectList());
    CPluginObject*                     p_obj;
    while( (p_obj = I.Current()) != NULL ) {
        if ( ((p_obj->GetCategoryUUID() == JOB_CAT)&&(p_obj->HasAttribute("EPF_IMPORT_STRUCTURE"))) ||
             (p_obj->GetCategoryUUID() == IMPORT_STRUCTURE_CAT) ) {
            QString format = p_obj->GetAttributeValue("FORMAT");
            if( ! format.isEmpty() ){
                cout << left << setw(10) << format.toStdString() << " ";
                cout << setw(20) << p_obj->GetPluginModule()->GetModuleUUID().GetName().toStdString() << " ";
                cout << p_obj->GetName().toStdString() << endl;
            }
        }
        I++;
    }

    cout << endl;
    cout << "# Trajectory import formats ..." << endl;
    cout << endl;
    cout << "# Format         Plugin         Description                                     " << endl;
    cout << "# -------- -------------------- ------------------------------------------------" << endl;

    I.SetToBegin();
    while( (p_obj = I.Current()) != NULL ) {
        if ( ((p_obj->GetCategoryUUID() == JOB_CAT)&&(p_obj->HasAttribute("EPF_IMPORT_TRAJECTORY"))) ||
             (p_obj->GetCategoryUUID() == IMPORT_TRAJECTORY_CAT) ) {
            QString format = p_obj->GetAttributeValue("FORMAT");
            if( ! format.isEmpty() ){
                cout << left << setw(10) << format.toStdString() << " ";
                cout << setw(20) << p_obj->GetPluginModule()->GetModuleUUID().GetName().toStdString() << " ";
                cout << p_obj->GetName().toStdString() << endl;
            }
        }
        I++;
    }
}

//------------------------------------------------------------------------------

void CNemesisJScript::PrintOBExtensions(void)
{
    cout << endl;
    cout << "# OpenBabel imput file formats for -ob import plugin" << endl;
    cout << endl;
    cout << "# File Format  Description                                                      " << endl;
    cout << "# ------------ -----------------------------------------------------------------" << endl;

    OBConversion co;
    vector<string> formats = co.GetSupportedInputFormat();

    for(unsigned int i = 0; i < formats.size(); i++) {
        CSmallString formatString, extension,description;
        formatString = formats.at(i).c_str();
        int pos = formatString.FindSubString("--");
        if( pos != -1 ) {
            extension = formatString.GetSubString(0, pos-1);
            description = formatString.GetSubString(pos+3,formatString.GetLength()-pos+4);
            cout << left << setw(14) << extension << " ";
            cout << description << endl;
        }
    }
}

//------------------------------------------------------------------------------

void CNemesisJScript::IncrementProgressNoGUI(const QString& text)
{
    cout << " >>> " << text.toStdString() << endl;
}

//------------------------------------------------------------------------------

void CNemesisJScript::OpenGLMessageLogged(const QOpenGLDebugMessage& debugMessage)
{
   cout << debugMessage.message().toStdString() << endl;
}

//------------------------------------------------------------------------------

void CNemesisJScript::ParseArguments(void)
{
    CProject* p_project = NULL;

    int narg = 0;
    while( narg < NemesisOptions.GetNumberOfProgArgs() ){
        CSmallString arg = NemesisOptions.GetProgArg(narg);
        if( arg == "-npr" ) {
            narg++;
            CSmallString file = NemesisOptions.GetProgArg(narg);
            narg++;

            // update last open path
            GlobalSetup->SetLastOpenFilePathFromFile(QString(file),GenericProjectID);

            // load project
            CProject* p_project = Projects->OpenProject(file);

            if( p_project == NULL ){
                ES_ERROR("unable to load project");
                QMessageBox::critical(NULL, tr("Open Project"),
                                      tr("An error occurred during project opening!"),
                                      QMessageBox::Ok,
                                      QMessageBox::Ok);
                return;
            }

            p_project->ShowProject();
            continue;
        }
        if( arg == "-build" ) {
            narg++;
            // create build project
            CExtUUID mp_uuid(NULL);
            mp_uuid.LoadFromString("{BUILD_PROJECT:b64d16f0-b73f-4747-9a13-212ab9a15d38}");
            p_project = Projects->NewProject(mp_uuid);
            if( p_project == NULL ){
                ES_ERROR("unable to create build project");
                QMessageBox::StandardButton result = QMessageBox::critical(NULL, tr("New Build Project"),
                                      tr("An error occurred during project opening!"),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok);
                if( result == QMessageBox::Cancel ) return;
                continue;
            }
            p_project->ShowProject();
            continue;
        }
        if( arg == "-traj" ){
            narg++;
            // create trajectory project
            CExtUUID mp_uuid(NULL);
            mp_uuid.LoadFromString("{TRAJECTORY_PROJECT:c4dd64d9-da20-413d-a2df-6f38697e4d2d}");
            p_project = Projects->NewProject(mp_uuid);
            if( p_project == NULL ){
                ES_ERROR("unable to create trajectory project");
                QMessageBox::StandardButton result = QMessageBox::critical(NULL, tr("New Trajectory Project"),
                                      tr("An error occurred during project opening!"),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok);
                if( result == QMessageBox::Cancel ) return;
                continue;
            }
            p_project->ShowProject();
            continue;
        }

        if( arg == "-sketch" ){
            narg++;
            // create sketch project
            CExtUUID mp_uuid(NULL);
            mp_uuid.LoadFromString("{SKETCH_PROJECT:6b047926-9a91-4e5f-884a-500db358ddb2}");
            p_project = Projects->NewProject(mp_uuid);
            if( p_project == NULL ){
                ES_ERROR("unable to create sketch project");
                QMessageBox::StandardButton result = QMessageBox::critical(NULL, tr("New Sketch Project"),
                                      tr("An error occurred during project opening!"),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok);
                if( result == QMessageBox::Cancel ) return;
                continue;
            }
            p_project->ShowProject();
            continue;
        }

        if( arg.GetLength() > 1 ){
            if( arg[0] != '-' ){
                narg++;
                CSmallString error;
                error << "Incorrect argument provided: " << arg;
                ES_ERROR(error);
                QMessageBox::StandardButton result = QMessageBox::critical(NULL, tr("Illegal argument"),
                                      QString(error),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok);
                if( result == QMessageBox::Cancel ) return;
                continue;
            }
            // if no project - create BuildProject
            if( p_project == NULL ){
                // create build project
                CExtUUID mp_uuid(NULL);
                mp_uuid.LoadFromString("{BUILD_PROJECT:b64d16f0-b73f-4747-9a13-212ab9a15d38}");
                p_project = Projects->NewProject(mp_uuid);
                if( p_project == NULL ){
                    ES_ERROR("unable to create build project");
                    QMessageBox::StandardButton result = QMessageBox::critical(NULL, tr("New Build Project"),
                                          tr("An error occurred during project opening!"),
                                          QMessageBox::Ok | QMessageBox::Cancel,
                                          QMessageBox::Ok);
                    if( result == QMessageBox::Cancel ) return;
                    continue;
                }
                p_project->ShowProject();
            }
            // import data if supported by project
            if( p_project->ProcessArguments(narg) == false ){
                narg++;
                CSmallString error;
                error << "Unable to process argument: " << arg;
                ES_ERROR(error);
                QMessageBox::StandardButton result = QMessageBox::critical(NULL, tr("Wrong argument"),
                                      QString(error),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok);
                if( result == QMessageBox::Cancel ) return;
                continue;
            }
        }

    }
}

//------------------------------------------------------------------------------

void CNemesisJScript::PluginLoaded(const QString& base_name)
{
    QString text;
    text = QString(tr("Loading Nemesis plugin %1 ....")).arg(base_name);
    emit SetupNotificationChanged(text);
}

//------------------------------------------------------------------------------

void CNemesisJScript::FinalizeSubsystems(void)
{
    // ---------------------------------
    // this will remove all setup designers
    GraphicsSetupProfile->CloseAllObjectDesigners();
    GraphicsSetupProfile->SaveDefaultConfig();

    OptimizerSetups->CloseAllObjectDesigners();
    OptimizerSetups->SaveConfig();

    // ---------------------------------
    MouseDriverSetup->SaveUserSetup();
    GlobalSetup->Save();
    RecentFiles->Save();
    GlobalDesktop->SaveDefaultDesktop();

    // ---------------------------------
    // here we should not have any opened project
    // so just to be sure
    Projects->RemoveAllProjects();

    // close all workpanels
    WorkPanels->RemoveAllWorkPanels(true);

    // process all events in the loop
    qApp->processEvents();

    // ---------------------------------
    // some part might be actually in
    // void CMainWindow::closeEvent(QCloseEvent * event)
    // because of order of object destructions
    Inserters->SaveConfig();

    // destroy global objects ---------
    delete RootList;
    RootList = NULL;
    Projects = NULL;
    GlobalObjectList = NULL;
    GraphicsSetupProfile = NULL;
    WorkPanels = NULL;
    PhysicalQuantities = NULL;
    Inserters = NULL;
    OptimizerSetups = NULL;
    JobScheduler = NULL;
    GlobalDesktop = NULL;
    MouseDriverSetup = NULL;

    // destroy cache - if not destroyed here
    // then there can be a race condition due to unspecific
    // order of static object destructions
    FTGLFontCache.DestroyAllFonts();

    // process all events in the loop
    qApp->processEvents();

    // close all plugins
    PluginDatabase.UnloadPlugins();
}

//------------------------------------------------------------------------------

void CNemesisJScript::RegisterObjects(void)
{
    // register metaobjects
    ProObjectRegisterMetaObject();
    ProjectRegisterMetaObject();

    // register functions ------------------------
    QScriptValue fce;

    fce = newFunction(NemesisQuit);
    globalObject().setProperty("quit",fce);

    fce = newFunction(NemesisPrint);
    globalObject().setProperty("print",fce);

    // constructable objects ---------------------
    CAboutWindow::Register(this);

    // static objects ----------------------------
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CNemesisJScript::QuitApplication(void)
{
    foreach(QObject* p_qobj,Projects->children()) {
        CProject* p_pro = dynamic_cast<CProject*>(p_qobj);
        if( p_pro == NULL ) continue;
        p_pro->BringToFront();
        if( Projects->CanCloseProject(p_pro) == true ) p_pro->CloseProject();
    }

    if( Projects->GetNumberOfProjects() == 0 ){
        WorkPanels->RemoveAllWorkPanels(false);
    }

    // close application
    if( Projects->GetNumberOfProjects() == 0 ){
        qApp->quit();
    }
}

//------------------------------------------------------------------------------

void CNemesisJScript::ShallQuitApplication(void)
{
    if( Projects->GetNumberOfProjects() != 0 ) return;

    WorkPanels->RemoveAllWorkPanels(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

