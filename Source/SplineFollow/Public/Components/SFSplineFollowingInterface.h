#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "SFSplineFollowingInterface.generated.h"

class USplineComponent;
struct FSFFollowSplineInfos;

DECLARE_DELEGATE_OneParam( FSWOnSplineFollowingReachedPositionDelegate, float );

UINTERFACE( MinimalAPI, NotBlueprintable )
class USFSplineFollowingInterface : public UInterface
{
    GENERATED_BODY()
};

class SPLINEFOLLOW_API ISFSplineFollowingInterface
{
    GENERATED_BODY()

public:
    struct FPositionObserver
    {
        FPositionObserver() :
            NormalizedPosition( 0.0f ),
            bTriggerOnce( false ),
            bHasBeenTriggered( false )
        {}

        FSWOnSplineFollowingReachedPositionDelegate Callback;
        float NormalizedPosition;
        bool bTriggerOnce;
        bool bHasBeenTriggered;
    };
    
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

    virtual void RegisterPositionObserver( const FSWOnSplineFollowingReachedPositionDelegate & delegate, float normalized_position, bool trigger_once = true ) = 0;
};
