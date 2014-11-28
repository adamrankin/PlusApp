MODEL_CATALOG_START()

MODEL_TABLE_START("Tracking fixtures" "See below a list of fixtures that can be used for mounting tracker markers (both optical and electromagnetic) on various tools and objects.")
MODEL_TABLE_ROW(
  ID "SensorHolder"
  DESCRIPTION "Clip to mount a MarkerHolder or 8mm Ascension EM sensor to an object. Smallest size."
  )
MODEL_TABLE_ROW(
  ID "SensorHolder-Wing"
  DESCRIPTION "Clip to mount a MarkerHolder or 8mm Ascension EM sensor to an object. With a wing to make it easier to fix it by glue or screws."
  )
MODEL_TABLE_ROW(
  ID "SensorHolder-Wing-Flat"
  DESCRIPTION "Clip to mount a MarkerHolder or 8mm Ascension EM sensor to an object. With a wing to make it easier to fix it by glue or screws. Low profile."
  )
MODEL_TABLE_ROW(
  ID "MarkerHolder_120mm-even_long"
  DESCRIPTION "Holder for visible-light printed black&white optical tracker markers (such as MicronTracker)."
  PRINTABLE_FILES "TrackingFixtures/MarkerHolder_120mm-even_long.stl" "TrackingFixtures/Marker_01-04.pdf"
  )
MODEL_TABLE_ROW(
  ID "MarkerHolder_120mm-odd_long"
  DESCRIPTION "Holder for visible-light printed black&white optical tracker markers (such as MicronTracker)."
  PRINTABLE_FILES "TrackingFixtures/MarkerHolder_120mm-odd_long.stl" "TrackingFixtures/Marker_01-04.pdf"
  )
MODEL_TABLE_ROW(
  ID "MarkerHolder_120mm-even_short"
  DESCRIPTION "Holder for visible-light printed black&white optical tracker markers (such as MicronTracker)."
  PRINTABLE_FILES "TrackingFixtures/MarkerHolder_120mm-even_short.stl" "TrackingFixtures/Marker_01-04.pdf"
  )
MODEL_TABLE_ROW(
  ID "MarkerHolder_120mm-odd_short"
  DESCRIPTION "Holder for visible-light printed black&white optical tracker markers (such as MicronTracker)."
  PRINTABLE_FILES "TrackingFixtures/MarkerHolder_120mm-odd_short.stl" "TrackingFixtures/Marker_01-04.pdf"
  )
# Add remaining experimental tools
FOREACH(MODELFILE CauteryHolder_1.0 MarkerHolder_120mm_Stickable_1.0 MarkerHolder_120mm_Winged_1.0 MarkerHolder_120mm-Short_2.0 NeedleClip-Assembly_1.1 NeedleClip-Assembly_1.5mm_2.0 NeedleHubClip-Assembly_1.5mm_1.0 NeedleGrabberFlappy-Assembly_1.0 NeedleGrabber-SensorBagHolder_1.0 OrientationsLR_1.0 Plug-L_60mm_3.0 SensorHolder_Wing_1.0 SensorHolder-GlueHoles-Ordered_2mm_1.0 SensorHolder-Ordered_2mm_1.0 SensorHolder-Ordered-HolesInterface_2mm_1.0)
  MODEL_TABLE_ROW(ID ${MODELFILE} DESCRIPTION "Experimental") 
ENDFOREACH()
MODEL_TABLE_END()

MODEL_TABLE_START("Tools" "See below a list of tools for tracking, calibration, and simulation.")
MODEL_TABLE_ROW(
  ID "Stylus_60mm"
  DESCRIPTION "Pointer tool with built-in sensor holder. 60mm long, sharp tip."
  )
MODEL_TABLE_ROW(
  ID "Stylus_100mm"
  DESCRIPTION "Pointer tool with built-in sensor holder. 100mm long, sharp tip."
  )
# Add remaining experimental tools
FOREACH(MODELFILE Stylus_Candycane_1.0 Stylus_Candycane_70mm_1.0)
  MODEL_TABLE_ROW(ID ${MODELFILE} DESCRIPTION "Experimental") 
ENDFOREACH()
MODEL_TABLE_ROW(
  ID "SPL40-1.0"
  PRINTABLE_FILES "SimProbeLinear/SPL40-1.0.stl"  
  EDIT_LINK "${CATALOG_URL}/SimProbeLinear"
  DESCRIPTION "Simulated 40mm wide linear ultrasound probe."
  )
MODEL_TABLE_END()

MODEL_TABLE_START("Calibration phantoms" "See below a list of ultrasound calibration phantoms.")
MODEL_TABLE_ROW(
  ID "fCal-2.0"
  IMAGE_FILE "fCalPhantom/fCal_2/PhantomDefinition_fCal_2.0_Wiring_2.0.png"
  PRINTABLE_FILES "fCalPhantom/fCal_2/fCal_2.0.stl"
  EDIT_LINK "${CATALOG_URL}/fCalPhantom/fCal_2"
  DESCRIPTION "Phantom for freehand spatial ultrasound calibration for shallow depth (up to 9 cm)."
  )
MODEL_TABLE_ROW(
  ID "fCal-3.1"
  IMAGE_FILE "fCalPhantom/fCal_3/fCal3.1.png"
  PRINTABLE_FILES
    "fCalPhantom/fCal_3/fCal_3.1.stl"
    "fCalPhantom/fCal_3/fCal_3.1_back.stl"
    "fCalPhantom/fCal_3/fCal_3.1_front.stl"
    "fCalPhantom/fCal_3/fCal_3.1_left.stl"
    "fCalPhantom/fCal_3/fCal_3.1_spacer.stl"
  EDIT_LINK "${CATALOG_URL}/fCalPhantom/fCal_3"
  DESCRIPTION "Phantom for freehand spatial ultrasound calibration for deep structures (up to 30 cm)."
  )
MODEL_TABLE_END()  

MODEL_CATALOG_END()
