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
};
