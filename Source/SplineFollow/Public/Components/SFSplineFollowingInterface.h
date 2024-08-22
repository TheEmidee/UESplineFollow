#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "SFSplineFollowingInterface.generated.h"

class USplineComponent;
struct FSFFollowSplineInfos;

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
    virtual void ToggleSplineMovement( bool it_is_active ) = 0;

    UFUNCTION( BlueprintCallable )
    virtual void SetDistanceOnSpline( float distance_on_spline ) = 0;

    UFUNCTION( BlueprintCallable )
    virtual void SetNormalizedDistanceOnSpline( float normalized_distance_on_spline ) = 0;

    UFUNCTION( BlueprintCallable )
    virtual bool FollowSpline( const FSFFollowSplineInfos & follow_spline_infos ) = 0;

    UFUNCTION( BlueprintCallable )
    virtual void UnFollowSpline() = 0;

    UFUNCTION( BlueprintCallable )
    virtual bool IsFollowingSpline() const = 0;

    UFUNCTION( BlueprintCallable )
    virtual float GetCurrentSpeed() const = 0;

    UFUNCTION( BlueprintCallable )
    virtual USplineComponent * GetFollowedSplineComponent() const = 0;
};
