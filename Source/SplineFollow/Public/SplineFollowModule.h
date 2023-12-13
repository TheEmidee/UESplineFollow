#pragma once

#include <CoreMinimal.h>
#include <Modules/ModuleInterface.h>
#include <Modules/ModuleManager.h>
#include <Stats/Stats.h>

class SPLINEFOLLOW_API ISplineFollowModule : public IModuleInterface
{

public:
    static ISplineFollowModule & Get()
    {
        static auto & singleton = FModuleManager::LoadModuleChecked< ISplineFollowModule >( "SplineFollow" );
        return singleton;
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "SplineFollow" );
    }
};
