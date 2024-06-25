#ifndef GraphicsViewH
#define GraphicsViewH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2012 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <QTimer>
#include <QImage>
#include <NemesisCoreMainHeader.hpp>
#include <ProObject.hpp>
#include <Manipulator.hpp>
#include <GraphicsProfile.hpp>
#include <GLSelection.hpp>
#include <GraphicsViewManipulator.hpp>
#include <SmallColor.hpp>
#include <GraphicsViewStereo.hpp>

//------------------------------------------------------------------------------

class CGraphics;
class CGraphicsViewList;
class CHistoryItem;
class CManipulator;
class CXMLElement;
class CGraphicsShadowView;
class CGraphicsCommonView;

//------------------------------------------------------------------------------

/// available projection modes

enum EProjectionMode {
    EPM_PERSPECTIVE,            // perspective projections
    EPM_ORTHOGRAPHIC            // orthographic projection
};

//------------------------------------------------------------------------------

/// graphic view

class NEMESIS_CORE_PACKAGE CGraphicsView : public CProObject,
                                           public CGraphicsViewManipulator {
Q_OBJECT
public:
// constructors and destructors ------------------------------------------------
    CGraphicsView(CGraphicsViewList* p_owner,bool primary);
    ~CGraphicsView(void);

// executive methods -----------------------------------------------------------
    /// delete view
    void Delete(void);

    /// attach shadow view opengl canvas
    void AttachShadowView(void);

    /// try to attach shadow view opengl canvas
    void TryToAttachShadowView(void);

    /// close shadow view
    void CloseShadowView(void);

    /// set mouse cursor
    void SetCursor(const QCursor& cursor);

// information methods ---------------------------------------------------------
    /// return owner of this list
    CGraphicsViewList*      GetViews(void) const;

    /// return graphics subsystem
    CGraphics*              GetGraphics(void) const;

    /// return graphics profiles subsystem
    CGraphicsProfileList*   GetProfiles(void) const;

    /// is OpenGL canvas attached?
    bool IsOpenGLCanvasAttached(void);

    /// is primary view?
    bool IsPrimaryView(void);

    /// return active profile for this view
    CGraphicsProfile* GetActiveProfile(void);

    /// return used profile for this view
    CGraphicsProfile* GetUsedProfile(void);

    /// get attached scene width
    int GetWidth(void);

    /// get attached scene height
    int GetHeight(void);

    /// is quad buffer stereo available
    bool IsQuadBufferStereoAvailable(void);

    /// is multisampling available
    bool IsMultiSamplingAvailable(void);

// visualization setup ---------------------------------------------------------
    /// synchronize with primary view
    void SyncWithPrimaryView(bool set);

    /// is synchronized with the primary view
    bool IsSynchronizedWithPrimaryView(void) const;

    /// set projection mode
    void SetProjectionMode(EProjectionMode mode);

    /// set projection setup
    void SetProjectionData(double near,double focal,double far,double fovy);

    /// return projection mode
    EProjectionMode GetProjectionMode(void);

    /// get projection setup
    void GetProjectionData(double& near,double& focal,double& far,double& fovy);

    /// set stereo mode
    void SetStereoMode(EStereoMode mode);

    /// set eye separation
    void SetEyeSeparation(double esep);

    /// return stereo mode
    EStereoMode GetStereoMode(void);

    /// set eye separation
    double GetEyeSeparation(void);

    /// swap stereo eyes
    void StereoSwapEyes(bool swap);

    /// are stereo eyes swapped
    bool AreStereoEyesSwapped(void);

    /// set depth cueing
    void SetDepthCueing(bool set);

    /// set depth cueing data
    void SetDepthCueingData(GLenum dcm,double dcd, double dcs, double dce,const CColor& dcc);

    /// is depth cueing enabled
    bool IsDepthCueingEnabled(void);

    /// get depth cueing data
    void GetDepthCueingData(GLenum& dcm,double& dcd, double& dcs, double& dce,CColor& dcc);

// executive methods -----------------------------------------------------------
    /// set used profile, if no profile is set then the active profile is used
    void SetUsedProfile(CGraphicsProfile* p_profile);

    /// fit scene to drawing area
    void FitScene(bool animated=true);

    /// fit scene on the ProObject
    void FitScene(CProObject* p_object, bool animated=true);

    /// set rotation center
    void SetRotationCentre(const CPoint& centre,bool animated=true);

    /// repaint this view and possibly all associated views
    void Repaint(EDrawLevel level);

    /// repaint only this view
    void RepaintOnlyThisView(void);

    /// render scene into QImage
    QImage Render(int width = 0, int height = 0);

    /// select object by mouse
    const CSelObject SelectObject(int mousex,int mousey);

// changes propagation ---------------------------------------------------------
    /// set name
    virtual void SetName(const QString& name,CHistoryNode* p_history=NULL);

    /// set description
    virtual void SetDescription(const QString& descrip,CHistoryNode* p_history=NULL);

// input/output methods --------------------------------------------------------
    /// load all profiles
    virtual void LoadData(CXMLElement* p_ele);

    /// save all profiles
    virtual void SaveData(CXMLElement* p_ele);

// signals ---------------------------------------------------------------------
public:
    /// should emit OnManipulatorChanged
    virtual void EmitManipulatorChanged(void);

signals:
    void OnManipulatorChanged(void);

// section of private data -----------------------------------------------------
private:
    CGraphicsProfile*       Profile;            // graphics profile to use
    CGraphicsCommonView*    DrawGLCanvas;       // attached OpenGL canvas
    bool                    SyncToPrimaryView;  // synchronize with primary view
    int                     Width;              // w and h if GLCanvas is not attached
    int                     Height;

    // selection data
    int     SelAreaSize;
    int     SelBuffSize;
    GLuint* SelBuffer;

    // fit scene data
    QTimer*             FitSceneTimer;
    int                 TickIntervals;
    double              ScaleTick;
    CPoint              PosTick;
    CPoint              CentreTick;
    CTransformation     TransTick;

    // visualization setup
    EProjectionMode ProjectionMode;

    // scene parameters
    double      Near;           // near plane
    double      Focal;          // focal plane
    double      Far;            // far plane
    double      Fovy;           // horizontal field of view
    double      EyeSep;         // eye separation

    // camera parameters
    EStereoMode StereoMode;
    bool        SwapEyes;

    // depth cueing
    bool        DepthCueing;
    GLenum      DCMode;
    double      DCDensity;
    double      DCStart;
    double      DCEnd;
    CColor      DCColor;

    // camera position
    CPoint      Position;       // camera position
    CPoint      Reference;      // reference point
    CPoint      ViewUp;         // camera vertical axis

    // shadow view data
    CXMLDocument            ShadowData;
    CGraphicsShadowView*    ShadowView;

    CXMLElement* GetShadowData(bool create=false);

// section of private methods --------------------------------------------------
private:
    /// attach opengl scene from common view, OpenGL context MUST be active
    void AttachOpenGLCanvas(CGraphicsCommonView* p_scene);

    /// detach opengl scene from common view, OpenGL context MUST be active
    void DetachOpenGLCanvas(void);

// these methods are executed by CGLWidget
    /// draw OpenGL scene
    void DrawGL(void);

// projections -----------------------------------
    /// init monoscopic view
    void InitMono(void);

    /// init monoscopic view for selection
    void InitMonoSelection(int x,int y,int w,int h);

// raw scene painting by manipulator
public:
    /// move scene
    virtual void Move(const CPoint& dmov,CGraphicsView* p_view);

    /// rotate scene
    virtual void Rotate(const CPoint& drot,CGraphicsView* p_view);

    /// scale scene
    virtual void ChangeScale(double increment,CGraphicsView* p_view);

    /// draw scene in local frame
    virtual void RawDraw(void);

private slots:
    void FitSceneTick(void);
    void GraphicsProfileListChanged(void);

    friend class CGraphicsCommonGLScene;
    friend class CGraphicsCommonView;
    friend class CGraphicsShadowView;
};

//------------------------------------------------------------------------------

#endif
