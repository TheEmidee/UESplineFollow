#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "SFAT_WaitSplineFollowerReachesPosition.generated.h"

class USFSplineFollowingMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FSWOnSplineFollowerPositionReachedDelegate );

UCLASS()
class SPLINEFOLLOW_API USFAT_WaitSplineFollowerReachesPosition final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static USFAT_WaitSplineFollowerReachesPosition * WaitSplineFollowerReachesPosition( UGameplayAbility * owning_ability, USFSplineFollowingMovementComponent * spline_following_movement_component, float normalized_position, bool trigger_once = true );
    void Activate() override;

private:
    void OnSplinePositionReached( float normalized_position );

    UPROPERTY()
    TObjectPtr< USFSplineFollowingMovementComponent > SplineFollowingMovementComponent;

    float NormalizedPosition;
    bool bTriggerOnce;

    UPROPERTY( BlueprintAssignable )
    FSWOnSplineFollowerPositionReachedDelegate OnSplineFollowerPositionReached;
};
