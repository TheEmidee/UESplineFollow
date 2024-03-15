#pragma once

#include <ComponentVisualizer.h>
#include <Editor/UnrealEdEngine.h>
#include <Modules/ModuleInterface.h>
#include <UnrealEdGlobals.h>

class FSplineFollowEditorModule : public IModuleInterface
{
public:
    FSplineFollowEditorModule();

    void StartupModule() override;
    void ShutdownModule() override;

private:
    template < typename _VISUALIZER_TYPE_, typename _COMPONENT_TYPE_ >
    void RegisterComponentVisualizer()
    {
        if ( GUnrealEd != nullptr )
        {
            // Make a new instance of the visualizer
            TSharedPtr< FComponentVisualizer > visualizer = MakeShareable( new _VISUALIZER_TYPE_() );

            // Register it to our specific component class
            GUnrealEd->RegisterComponentVisualizer( _COMPONENT_TYPE_::StaticClass()->GetFName(), visualizer );
            visualizer->OnRegister();
        }
    }

    template < typename _COMPONENT_TYPE_ >
    void UnregisterComponentVisualizer()
    {
        if ( GUnrealEd != nullptr )
        {
            GUnrealEd->UnregisterComponentVisualizer( _COMPONENT_TYPE_::StaticClass()->GetFName() );
        }
    }
};
