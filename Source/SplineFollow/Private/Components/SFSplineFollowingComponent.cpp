#include "Components/SFSplineFollowingComponent.h"

#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

USFSplineFollowingComponent::USFSplineFollowingComponent() :
    MovementComponent( nullptr ),
    SplineDistance( 0.0f ),
    Destination( FVector::ZeroVector )
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USFSplineFollowingComponent::BeginPlay()
{
    Super::BeginPlay();

    MovementComponent = GetOwner()->GetComponentByClass< UCharacterMovementComponent >();
}

void USFSplineFollowingComponent::TickComponent( const float delta_time, const ELevelTick tick_type, FActorComponentTickFunction * this_tick_function )
{
    Super::TickComponent( delta_time, tick_type, this_tick_function );

    UpdateDestination( delta_time );
}

void USFSplineFollowingComponent::UpdateDestination( float delta_time )
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    if ( MovementComponent == nullptr )
    {
        return;
    }

    const auto destination_distance = SplineDistance + MovementComponent->GetMaxSpeed() * delta_time;

    Destination = FollowedSplineComponent->GetLocationAtDistanceAlongSpline( destination_distance, ESplineCoordinateSpace::World );
}
