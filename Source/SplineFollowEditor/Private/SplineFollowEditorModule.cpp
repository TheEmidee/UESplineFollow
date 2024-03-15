#include "SplineFollowEditorModule.h"

#include "SFSplineMarkerComponentVisualizer.h"

IMPLEMENT_MODULE( FSplineFollowEditorModule, SplineFollowEditor )

#define LOCTEXT_NAMESPACE "SplineFollowEditor"

FSplineFollowEditorModule::FSplineFollowEditorModule()
{
}

void FSplineFollowEditorModule::StartupModule()
{
    FCoreDelegates::OnPostEngineInit.AddLambda( [ this ]() {
        RegisterComponentVisualizer< FSFSplineMarkerComponentVisualizer, USFSplineComponent >();
    } );
}

void FSplineFollowEditorModule::ShutdownModule()
{
    if ( !UObjectInitialized() )
    {
        return;
    }

    UnregisterComponentVisualizer< USFSplineComponent >();
}

#undef LOCTEXT_NAMESPACE
