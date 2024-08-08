#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "SFSplineFollowingInterface.generated.h"

UINTERFACE( MinimalAPI, NotBlueprintable )
class USFSplineFollowingInterface : public UInterface
{
    GENERATED_BODY()
};

class SPLINEFOLLOW_API ISFSplineFollowingInterface
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    virtual float GetDistanceOnSpline() const = 0;

    UFUNCTION( BlueprintCallable )
    virtual float GetNormalizedDistanceOnSpline() const = 0;

    UFUNCTION( BlueprintCallable )
    virtual void ToggleSplineMovement( bool enable ) = 0;

    UFUNCTION( BlueprintCallable )
    virtual void SetDistanceOnSpline( float new_distance ) = 0;
};
