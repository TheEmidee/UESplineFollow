#include "Abilities/SFAT_WaitSplineFollowerReachesPosition.h"

#include "Components/SFSplineFollowingMovementComponent.h"

USFAT_WaitSplineFollowerReachesPosition * USFAT_WaitSplineFollowerReachesPosition::WaitSplineFollowerReachesPosition( UGameplayAbility * owning_ability, USFSplineFollowingMovementComponent * spline_following_movement_component, float normalized_position, bool trigger_once /*= true*/ )
{
    auto * task = NewAbilityTask< USFAT_WaitSplineFollowerReachesPosition >( owning_ability );
    task->SplineFollowingMovementComponent = spline_following_movement_component;
    task->NormalizedPosition = normalized_position;
    task->bTriggerOnce = trigger_once;
    return task;
}

void USFAT_WaitSplineFollowerReachesPosition::Activate()
{
    Super::Activate();

    if ( SplineFollowingMovementComponent == nullptr )
    {
        EndTask();
        return;
    }

    SplineFollowingMovementComponent->RegisterPositionObserver( FSWOnSplineFollowingReachedPositionDelegate::CreateUObject( this, &ThisClass::OnSplinePositionReached ), NormalizedPosition, bTriggerOnce );
}

void USFAT_WaitSplineFollowerReachesPosition::OnSplinePositionReached( float /*normalized_position*/ )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnSplineFollowerPositionReached.Broadcast();
        EndTask();
    }
}
