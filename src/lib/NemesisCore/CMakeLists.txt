# ==============================================================================
# Nemesis CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(NEMESIS_CORE_SRC
    # core part ----------------------------------
        NemesisCoreMainHeader.cpp
        NemesisCoreModule.cpp

    # common part --------------------------------
        common/CategoryUUID.cpp

        common/ExtComObject.cpp
        common/ExtComObjectDesigner.cpp

        common/ContainerModel.cpp
        common/TreeModel.cpp
        common/ListModel.cpp
        common/DummyModel.cpp

        common/GlobalSetup.cpp
        common/DesktopSystem.cpp
        common/GeoMeasurement.cpp

        common/OpenBabelUtils.cpp
        common/ImpexFormats.cpp

    # units --------------------------------------
        units/PhysicalQuantity.cpp
        units/StdPhysicalQuantity.cpp
        units/StdPhysicalQuantityModel.cpp
        units/PhysicalQuantities.cpp
        units/PhysicalQuantitiesModel.cpp
        units/CombPhysicalQuantity.cpp
        units/FCPhysicalQuantity.cpp

    # scripting feature --------------------------
        jscript/NemesisOptions.cpp
        jscript/NemesisJScript.cpp
        jscript/JScriptFunctions.cpp
        jscript/RootList.cpp
        jscript/GlobalObjectList.cpp
        jscript/GlobalDesktop.cpp

    # workpanel ----------------------------------
        workpanel/WorkPanelDockWidget.cpp
        workpanel/WorkPanelDialogWidget.cpp
        workpanel/WorkPanelContainer.cpp
        workpanel/WorkPanel.cpp
        workpanel/WorkPanelList.cpp
        workpanel/WorkPanelModel.cpp

    # project ------------------------------------
        project/StaticIndexes.cpp
        project/GeoDescriptor.cpp
        project/ObjMetrics.cpp
        project/ProObject.cpp        
        project/HistoryItem.cpp
        project/HistoryNode.cpp
        project/HistoryList.cpp
        project/HistoryListModel.cpp
        project/RegisteredObject.cpp
        project/ProObjectHistory.cpp
        project/ProObjectModel.cpp
        project/ProObjectDesigner.cpp
        project/ProObjectRegObjectsModel.cpp
        project/Project.cpp
        project/ProjectList.cpp
        project/ProjectListModel.cpp
        project/ProjectStatusBar.cpp
        project/ProjectDesktop.cpp
        project/RecentFile.cpp
        project/RecentFileList.cpp

        project/utils/PODesignerGeneral.cpp
        project/utils/PODesignerFlags.cpp
        project/utils/PODesignerRefBy.cpp      
        project/utils/PODesignerStructures.cpp
        project/utils/PODesignerTrajectories.cpp
	project/utils/PODesignerDockingTabsResults.cpp

    # structure -----------------------------------
        structure/AtomData.cpp
        structure/Atom.cpp
        structure/AtomBondsModel.cpp
        structure/AtomHistory.cpp
        structure/AtomDesigner.cpp
        structure/AtomList.cpp
        structure/AtomListModel.cpp
        structure/AtomListHistory.cpp
        structure/BondData.cpp
        structure/Bond.cpp
        structure/BondHistory.cpp
        structure/BondDesigner.cpp
        structure/BondList.cpp
        structure/BondListModel.cpp
        structure/BondListHistory.cpp
        structure/Residue.cpp
        structure/ResidueModel.cpp
        structure/ResidueDesigner.cpp
        structure/ResidueHistory.cpp
        structure/ResidueList.cpp
        structure/ResidueListModel.cpp
        structure/ResidueListHistory.cpp
        structure/PBCInfo.cpp
        structure/Structure.cpp
        structure/StructureHistory.cpp
        structure/StructureDesigner.cpp
        structure/StructureList.cpp
        structure/StructureListDesigner.cpp
        structure/StructureListModel.cpp
        structure/StructureListHistory.cpp

    # trajectory ----------------------------------
        trajectory/Snapshot.cpp
        trajectory/SnapshotFilter.cpp
        trajectory/SnapshotFilterHistory.cpp
        trajectory/TrajectoryList.cpp
        trajectory/TrajectoryListModel.cpp
        trajectory/TrajectoryListHistory.cpp
        trajectory/TrajectoryListDesigner.cpp
        trajectory/Trajectory.cpp
        trajectory/TrajectoryModelProperties.cpp 
	trajectory/TrajectoryModelSegments.cpp
        trajectory/TrajectoryModelFilters.cpp
        trajectory/TrajectoryHistory.cpp
        trajectory/TrajectoryDesigner.cpp
        trajectory/TrajectorySegment.cpp
        trajectory/TrajectorySegmentHistory.cpp
        trajectory/ImportTrajectory.cpp

        trajectory/segments/XYZTrajSegment.cpp
        trajectory/segments/XYZTrajSegmentDesigner.cpp
        trajectory/segments/PDBQTTrajSegment.cpp
        trajectory/segments/PDBQTTrajSegmentDesigner.cpp
        trajectory/segments/PDBQTTrajSegmentModel.cpp
        trajectory/segments/NormalVibMode.cpp
        trajectory/segments/VibTrajSegment.cpp
        trajectory/segments/VibTrajSegmentDesigner.cpp

    # restraints ----------------------------------
        structure/Restraint.cpp
        structure/RestraintDesigner.cpp
        structure/RestraintHistory.cpp
        structure/RestraintList.cpp
        structure/RestraintListModel.cpp
        structure/RestraintListHistory.cpp

    # selection -----------------------------------
        selection/SelectionHandler.cpp
        selection/SelectionRequest.cpp
        selection/SelectionList.cpp
        selection/SelectionModel.cpp

        selection/handlers/AngleSelection.cpp
        selection/handlers/AtomSelection.cpp
        selection/handlers/AtomSeqSelection.cpp
        selection/handlers/AtomListSelection.cpp
        selection/handlers/AtomsAndBondsSelection.cpp
        selection/handlers/BondSelection.cpp
        selection/handlers/BondListSelection.cpp
        selection/handlers/ConnectAtomSelection.cpp
        selection/handlers/DistanceSelection.cpp
        selection/handlers/GraphicsObjectSelection.cpp
        selection/handlers/GraphicsObjectListSelection.cpp
        selection/handlers/StructureSelection.cpp
        selection/handlers/StructureListSelection.cpp
        selection/handlers/ObjectSelection.cpp
        selection/handlers/ResidueListSelection.cpp
        selection/handlers/TorsionSelection.cpp
        selection/handlers/DirectionSelection.cpp
        selection/handlers/MoleculeSelection.cpp
        selection/handlers/MoleculeListSelection.cpp
        selection/handlers/PointSelection.cpp
        selection/handlers/ResidueSelection.cpp
        selection/handlers/PropertySelection.cpp
        selection/handlers/PropertyRestraintSelection.cpp
        selection/handlers/PropertyListSelection.cpp
        selection/handlers/RestraintSelection.cpp
        selection/handlers/RestraintListSelection.cpp

    # masks ---------------------------------------
        selection/masks/asl/maskparser/ASLMask.tab.c
        selection/masks/asl/maskparser/ASLMask.yy.c
        selection/masks/asl/maskparser/ASLParser.cpp
        selection/masks/asl/ASLSelection.cpp
        selection/masks/asl/ASLMask.cpp

    # graphics ------------------------------------  
        graphics/utils/ElementColors.cpp
        graphics/utils/ElementColorsList.cpp
        graphics/utils/GraphicsUtil.cpp
        graphics/utils/GLSelection.cpp
        graphics/utils/GODesignerFlags.cpp
        graphics/utils/GODesignerObjects.cpp
        graphics/utils/GODesignerSetup.cpp
        graphics/utils/GODesignerRefBy.cpp
        graphics/utils/GODesignerColorMode.cpp
        graphics/utils/GSDesignerGeneral.cpp
        graphics/utils/GSDesignerRefBy.cpp
        graphics/utils/GOColorMode.cpp
        graphics/utils/GOColorModeHistory.cpp
        graphics/utils/GOColorModeDesigner.cpp
        graphics/utils/FTGLFontCache.cpp
        graphics/utils/FTGLFont.cpp

        graphics/GraphicsObject.cpp
        graphics/GraphicsObjectHistory.cpp
        graphics/GraphicsObjectModel.cpp

        graphics/GraphicsProfile.cpp
        graphics/GraphicsProfileObject.cpp
        graphics/GraphicsProfileDesigner.cpp
        graphics/GraphicsProfileHistory.cpp
        graphics/GraphicsProfileModel.cpp

        graphics/GraphicsObjectList.cpp
        graphics/GraphicsObjectListDesigner.cpp
        graphics/GraphicsObjectListHistory.cpp
        graphics/GraphicsObjectListModel.cpp

        graphics/GraphicsProfileList.cpp
        graphics/GraphicsProfileListHistory.cpp
        graphics/GraphicsProfileListModel.cpp
        graphics/GraphicsProfileListDesigner.cpp

        graphics/GraphicsViewManipulator.cpp
        graphics/GraphicsViewStereo.cpp
        graphics/GraphicsView.cpp
        graphics/GraphicsViewDesigner.cpp
        graphics/GraphicsViewList.cpp
        graphics/GraphicsViewListDesigner.cpp
        graphics/GraphicsViewListModel.cpp
        graphics/GraphicsSetup.cpp
        graphics/GraphicsSetupList.cpp
        graphics/GraphicsSetupProfile.cpp

        graphics/primary/GraphicsCommonView.cpp
        graphics/primary/GraphicsCommonGLScene.cpp
        graphics/primary/GraphicsPrimaryView.cpp

        graphics/shadow/GraphicsShadowViewDockWidget.cpp
        graphics/shadow/GraphicsShadowControls.cpp
        graphics/shadow/GraphicsShadowFSNotice.cpp
        graphics/shadow/GraphicsShadowCtrlNotice.cpp
        graphics/shadow/GraphicsShadowView.cpp

        graphics/Graphics.cpp

        graphics/links/GraphicsObjectLink.cpp
        graphics/links/GraphicsObjectEnd.cpp
        graphics/links/GraphicsObjectLinkDesigner.cpp

        graphics/standard/LightSetup.cpp
        graphics/standard/LightSetupDesigner.cpp
        graphics/standard/LightObject.cpp
        graphics/standard/LightObjectDesigner.cpp
        graphics/standard/LightObjectHistory.cpp

        graphics/standard/BackgroundSetup.cpp
        graphics/standard/BackgroundSetupDesigner.cpp
        graphics/standard/BackgroundObject.cpp
        graphics/standard/BackgroundObjectDesigner.cpp

        graphics/standard/StandardModelSetup.cpp
        graphics/standard/StandardModelSetupDesigner.cpp
        graphics/standard/StandardModelObject.cpp
        graphics/standard/StandardModelObjectDesigner.cpp
        graphics/standard/StandardModelObjectHistory.cpp

        graphics/standard/FastModelSetup.cpp
        graphics/standard/FastModelSetupDesigner.cpp
        graphics/standard/FastModelObject.cpp
        graphics/standard/FastModelObjectHistory.cpp
        graphics/standard/FastModelObjectDesigner.cpp

        graphics/standard/XYZAxesSetup.cpp
        graphics/standard/XYZAxesSetupDesigner.cpp
        graphics/standard/XYZAxesObject.cpp
        graphics/standard/XYZAxesObjectDesigner.cpp

        graphics/standard/SpecAxesSetup.cpp
        graphics/standard/SpecAxesSetupDesigner.cpp
        graphics/standard/SpecAxesObject.cpp
        graphics/standard/SpecAxesObjectHistory.cpp
        graphics/standard/SpecAxesObjectDesigner.cpp

        graphics/standard/AtomLabelSetup.cpp
        graphics/standard/AtomLabelSetupDesigner.cpp
        graphics/standard/AtomLabelObject.cpp
        graphics/standard/AtomLabelObjectDesigner.cpp
        graphics/standard/AtomLabelObjectHistory.cpp

        graphics/standard/GeometryLabelSetup.cpp
        graphics/standard/GeometryLabelSetupDesigner.cpp
        graphics/standard/GeometryLabelObject.cpp
        graphics/standard/GeometryLabelObjectHistory.cpp
        graphics/standard/GeometryLabelObjectDesigner.cpp

        graphics/standard/FreezedAtomsSetup.cpp
        graphics/standard/FreezedAtomsSetupDesigner.cpp
        graphics/standard/FreezedAtomsObject.cpp
        graphics/standard/FreezedAtomsObjectDesigner.cpp

        graphics/standard/BoxSetup.cpp
        graphics/standard/BoxSetupDesigner.cpp
        graphics/standard/BoxObject.cpp
        graphics/standard/BoxObjectDesigner.cpp
        graphics/standard/BoxObjectHistory.cpp

        graphics/standard/PBCBoxSetup.cpp
        graphics/standard/PBCBoxSetupDesigner.cpp
        graphics/standard/PBCBoxObject.cpp
        graphics/standard/PBCBoxObjectDesigner.cpp
        graphics/standard/PBCBoxObjectHistory.cpp

        graphics/standard/PlaneSetup.cpp
        graphics/standard/PlaneSetupDesigner.cpp
        graphics/standard/PlaneObject.cpp
        graphics/standard/PlaneObjectDesigner.cpp
        graphics/standard/PlaneObjectHistory.cpp

        graphics/standard/VectorSetup.cpp
        graphics/standard/VectorSetupDesigner.cpp
        graphics/standard/VectorObject.cpp
        graphics/standard/VectorObjectDesigner.cpp
        graphics/standard/VectorObjectHistory.cpp

        graphics/standard/PointSetup.cpp
        graphics/standard/PointSetupDesigner.cpp
        graphics/standard/PointObject.cpp
        graphics/standard/PointObjectDesigner.cpp
        graphics/standard/PointObjectHistory.cpp

        graphics/standard/TextSetup.cpp
        graphics/standard/TextSetupDesigner.cpp
        graphics/standard/TextObject.cpp
        graphics/standard/TextObjectDesigner.cpp
        graphics/standard/TextObjectHistory.cpp

    # properties ---------------------------------
        properties/Property.cpp
        properties/GraphicsProperty.cpp
        properties/PropertyHistory.cpp
        properties/PropertyList.cpp
        properties/PropertyListHistory.cpp
        properties/PropertyListModel.cpp

        properties/utils/GeoProperty.cpp
        properties/utils/GeoPropertyHistory.cpp
        properties/utils/PropertyAtomList.cpp
        properties/utils/PropertyAtomListHistory.cpp
        properties/utils/PropertyAtomListModel.cpp
        properties/utils/PRDesignerGeneral.cpp
        properties/utils/PRDesignerAtoms.cpp
        properties/utils/PRDesignerGraphics.cpp
        properties/utils/PRDesignerValue.cpp
        properties/utils/PRDesignerGeoGraphics.cpp
        properties/utils/GeoPropertySetup.cpp
        properties/utils/GeoPropertySetupDesigner.cpp

        properties/standard/EnergyProperty.cpp
        properties/standard/EnergyPropertyHistory.cpp
        properties/standard/EnergyPropertyDesigner.cpp

        properties/standard/DistanceProperty.cpp
        properties/standard/DistancePropertyDesigner.cpp

        properties/standard/AngleProperty.cpp
        properties/standard/AnglePropertyDesigner.cpp

        properties/standard/TorsionProperty.cpp
        properties/standard/TorsionPropertyDesigner.cpp

        properties/standard/DistanceToPositionProperty.cpp
        properties/standard/DistanceToPositionPropertyHistory.cpp
        properties/standard/DistanceToPositionPropertyDesigner.cpp

    # manipulators -------------------------------
        manip/Manipulator.cpp
        manip/ObjectManipulator.cpp
        manip/MouseDriver.cpp
        manip/MouseDriverSetup.cpp
        manip/MouseHandler.cpp
        manip/drivers/SceneMouseDriver.cpp
        manip/drivers/AtomManipMouseDriver.cpp
        manip/drivers/DirManipMouseDriver.cpp
        manip/drivers/MolManipMouseDriver.cpp
        manip/drivers/ObjManipMouseDriver.cpp

    # build --------------------------------------
        build/Fragment.cpp
        build/FragmentPalette.cpp
        build/Inserter.cpp
        build/DefaultInserter.cpp
        build/InserterList.cpp
        build/OptimizerSetup.cpp
        build/Optimizer.cpp
        build/OptimizerSetupList.cpp

    # widgets ------------------------------------
        widgets/NemesisStyle.cpp
        widgets/PeriodicTableWidget.cpp
        widgets/TerminalEdit.cpp
        widgets/TerminalWidget.cpp
        widgets/TerminalOStream.cpp
        widgets/ColorPushButton.cpp
        widgets/QuantitySpinBox.cpp
        widgets/QuantityLineEdit.cpp
        widgets/FTGLFontComboBox.cpp
        widgets/ObjectLinkWidgetPushButton.cpp
        widgets/ObjectLinkWidget.cpp
        widgets/LineStippleComboBox.cpp

    # windows ------------------------------------
        windows/AboutWindow.cpp
        windows/WelcomeWindow.cpp
        windows/ConfigMainWindow.cpp
        windows/MainWindow.cpp
        windows/MainWindowFile.cpp
        windows/MainWindowEdit.cpp
        windows/MainWindowSelection.cpp
        windows/MainWindowStructure.cpp
        windows/MainWindowGeometry.cpp
        windows/MainWindowProperties.cpp
        windows/MainWindowGraphics.cpp
        windows/MainWindowMouse.cpp
        windows/MainWindowSetup.cpp
        windows/MainWindowTools.cpp
        windows/MainWindowHelp.cpp
        windows/RenderWindow.cpp
        windows/CrashCentre.cpp

    # job ----------------------------------------
        job/Job.cpp
        job/JobThread.cpp
        job/JobList.cpp
        job/JobListModel.cpp
        job/JobScheduler.cpp
        job/JobSchedulerModel.cpp
        job/ImportJob.cpp

    # batch jobs ---------------------------------
        batchjob/BatchJob.cpp
        batchjob/BatchJobList.cpp
        batchjob/BatchJobListModel.cpp

    # dialogs ------------------------------------
        dialogs/RegExpDialog.cpp
        dialogs/MaskDialog.cpp
        dialogs/SelBondOrderDialog.cpp
        dialogs/GraphicsTypeDialog.cpp
        dialogs/TextDialog.cpp
        )

# program windows --------------------------------------------------------------
SET(NEMESIS_UIS
        windows/CrashCentre.ui
        windows/AboutWindow.ui
        windows/WelcomeWindow.ui
        windows/ConfigMainWindow.ui
        windows/RenderWindow.ui
        structure/AtomDesigner.ui
        structure/BondDesigner.ui
        structure/ResidueDesigner.ui
        structure/RestraintDesigner.ui
        structure/StructureDesigner.ui
        structure/StructureListDesigner.ui
        graphics/utils/GODesignerFlags.ui
        graphics/utils/GODesignerObjects.ui
        graphics/utils/GODesignerSetup.ui
        graphics/utils/GODesignerRefBy.ui
        graphics/utils/GSDesignerGeneral.ui
        graphics/utils/GSDesignerRefBy.ui
        graphics/standard/BackgroundSetupDesigner.ui
        graphics/standard/LightSetupDesigner.ui
        graphics/standard/LightObjectDesigner.ui
        graphics/standard/StandardModelSetupDesigner.ui
        graphics/standard/StandardModelObjectDesigner.ui
        graphics/standard/GeometryLabelSetupDesigner.ui
        graphics/standard/XYZAxesSetupDesigner.ui
        graphics/standard/FastModelSetupDesigner.ui
        graphics/standard/FastModelObjectDesigner.ui
        graphics/standard/AtomLabelObjectDesigner.ui
        graphics/standard/BoxSetupDesigner.ui
        graphics/standard/BoxObjectDesigner.ui
        graphics/standard/PlaneSetupDesigner.ui
        graphics/standard/VectorSetupDesigner.ui
        graphics/standard/VectorObjectDesigner.ui
        graphics/standard/FreezedAtomsSetupDesigner.ui
        graphics/standard/PointSetupDesigner.ui
        graphics/standard/PointObjectDesigner.ui
        graphics/standard/AtomLabelSetupDesigner.ui
        graphics/standard/BackgroundObjectDesigner.ui
        graphics/standard/FreezedAtomsObjectDesigner.ui
        graphics/standard/XYZAxesObjectDesigner.ui
        graphics/GraphicsProfileDesigner.ui
        graphics/standard/PBCBoxObjectDesigner.ui
        graphics/standard/PBCBoxSetupDesigner.ui
        graphics/standard/GeometryLabelObjectDesigner.ui
        graphics/standard/PlaneObjectDesigner.ui
        dialogs/RegExpDialog.ui
        dialogs/MaskDialog.ui
        dialogs/SelBondOrderDialog.ui
        dialogs/GraphicsTypeDialog.ui
        dialogs/TextDialog.ui
        widgets/ObjectLinkWidget.ui
        graphics/standard/TextSetupDesigner.ui
        graphics/standard/TextObjectDesigner.ui
        properties/utils/PRDesignerGeneral.ui
        project/utils/PODesignerStructures.ui
        project/utils/PODesignerGeneral.ui
        project/utils/PODesignerRefBy.ui
        project/utils/PODesignerFlags.ui
        project/utils/PODesignerDockingTabsResults.ui
        properties/utils/PRDesignerAtoms.ui
        properties/utils/PRDesignerGraphics.ui
        properties/standard/EnergyPropertyDesigner.ui
        properties/standard/DistancePropertyDesigner.ui
        graphics/links/GraphicsObjectLinkDesigner.ui
        properties/utils/GeoPropertySetupDesigner.ui
        properties/utils/PRDesignerValue.ui
        properties/utils/PRDesignerGeoGraphics.ui
        graphics/utils/GOColorModeDesigner.ui
        graphics/utils/GODesignerColorMode.ui
        properties/standard/AnglePropertyDesigner.ui
        properties/standard/TorsionPropertyDesigner.ui
        properties/standard/DistanceToPositionPropertyDesigner.ui
        graphics/standard/SpecAxesSetupDesigner.ui
        graphics/standard/SpecAxesObjectDesigner.ui
        graphics/GraphicsObjectListDesigner.ui
        graphics/GraphicsViewListDesigner.ui
        graphics/GraphicsViewDesigner.ui
        graphics/GraphicsProfileListDesigner.ui
        graphics/shadow/GraphicsShadowControls.ui
        graphics/shadow/GraphicsShadowFSNotice.ui
        graphics/shadow/GraphicsShadowCtrlNotice.ui
        trajectory/TrajectoryDesigner.ui
        project/utils/PODesignerTrajectories.ui
        trajectory/TrajectoryListDesigner.ui
        trajectory/segments/PDBQTTrajSegmentDesigner.ui
        trajectory/segments/XYZTrajSegmentDesigner.ui
        trajectory/segments/VibTrajSegmentDesigner.ui
        )

# resource files ---------------------------------------------------------------
SET(NEMESIS_CORE_RCS
        NemesisCore.qrc
        )

# QT5 related binding ----------------------------------------------------------
QT5_WRAP_UI(NEMESIS_CORE_SRC ${NEMESIS_UIS})
QT5_ADD_RESOURCES(NEMESIS_CORE_SRC ${NEMESIS_CORE_RCS})

# Nemesis uses plugins, so dynamic linking is obligatory!

# create shared library --------------------------------------------------------

ADD_DEFINITIONS(-DNEMESIS_CORE_BUILDING_DLL)

ADD_LIBRARY(nemesis_core_shared SHARED ${NEMESIS_CORE_SRC})

ADD_DEPENDENCIES(nemesis_core_shared plusula_shared)

TARGET_LINK_LIBRARIES(nemesis_core_shared plusula
                ${FTGL_LIB}
                ${OPENGL_glu_LIBRARY}
                ${OPENGL_gl_LIBRARY}
                ${OPEN_BABEL_LIB}
                Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Script Qt5::OpenGL Qt5::Svg
                ${SCIMAFIC_CLIB_NAME}
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

SET_TARGET_PROPERTIES(nemesis_core_shared PROPERTIES
                        OUTPUT_NAME NemesisCore
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${NEMESIS_VERSION})

INSTALL(TARGETS
            nemesis_core_shared
        DESTINATION
            lib
        )

# ------------------------------------------------------------------------------


