#include "Components/SFSplineFollowingComponent.h"

#include "Components/SFSplineFollowingMovementComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code.
static constexpr float min_tick_time = 1e-6f;

USFSplineFollowingComponent::USFSplineFollowingComponent() :
    MovementComponent( nullptr ),
    SplineDistance( 0.0f ),
    MaxSimulationIterations( 4 ),
    MaxSimulationTimeStep( 0.05f ),
    Destination( FVector::ZeroVector ),
    DestinationDistance( 0.0f )
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USFSplineFollowingComponent::FollowSpline( const FSFFollowSplineInfos & spline_infos )
{
    FollowedSplineComponent = spline_infos.SplineComponent;

    const auto & transform = FollowedSplineComponent->GetTransformAtDistanceAlongSpline( 0.0f, ESplineCoordinateSpace::World );
    const auto feet_location = MovementComponent->GetActorFeetLocation();
    const auto actor_location = MovementComponent->GetActorLocation();
    GetOwner()->SetActorLocationAndRotation( transform.GetLocation() + actor_location - feet_location, transform.GetRotation() );
    SplineDistance = 0.0f;
}

void USFSplineFollowingComponent::BeginPlay()
{
    Super::BeginPlay();

    MovementComponent = GetOwner()->GetComponentByClass< UCharacterMovementComponent >();
}

void USFSplineFollowingComponent::TickComponent( const float delta_time, const ELevelTick tick_type, FActorComponentTickFunction * this_tick_function )
{
    Super::TickComponent( delta_time, tick_type, this_tick_function );

    if ( MovementComponent->HasAnimRootMotion() || MovementComponent->CurrentRootMotion.HasOverrideVelocity() )
    {
        return;
    }

    auto remaining_time = delta_time;
    auto iterations = 0;

    while ( remaining_time >= min_tick_time && ( iterations < MaxSimulationIterations ) && IsValid( GetOwner() ) && !HasStoppedSimulation() )
    {
        iterations++;

        const auto time_tick = GetSimulationTimeStep( remaining_time, iterations );
        remaining_time -= time_tick;

        UpdateDestination( time_tick );
        FollowDestination();
    }
}

void USFSplineFollowingComponent::UpdateDestination( const float delta_time )
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    if ( MovementComponent == nullptr )
    {
        return;
    }

    if ( !HasReachedDestination() )
    {
        return;
    }

    SplineDistance = DestinationDistance;
    DestinationDistance = SplineDistance + MovementComponent->GetMaxSpeed() * delta_time;
    Destination = FollowedSplineComponent->GetLocationAtDistanceAlongSpline( DestinationDistance, ESplineCoordinateSpace::World );
}

void USFSplineFollowingComponent::FollowDestination() const
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    const auto current_location = FollowedSplineComponent->GetLocationAtDistanceAlongSpline( SplineDistance, ESplineCoordinateSpace::World );
    const auto actor_location = MovementComponent->GetActorFeetLocation();
    auto desired_movement = Destination - current_location;

    if ( !MovementComponent->CurrentRootMotion.HasAdditiveVelocity() )
    {
        const auto spline_offset = actor_location - current_location;
        const auto spline_offset_dir = spline_offset.GetSafeNormal();
        const auto spline_offset_distance = spline_offset.SquaredLength();

        desired_movement -= spline_offset_dir * spline_offset_distance * 0.01f;
    }

    const auto move_input = desired_movement.GetSafeNormal();
    MovementComponent->AddInputVector( move_input );
}

bool USFSplineFollowingComponent::HasReachedDestination()
{
    if ( MovementComponent == nullptr )
    {
        return false;
    }

    const auto current_direction = GetOwner()->GetActorForwardVector();
    const auto current_location = MovementComponent->GetActorFeetLocation();

    // check if moved too far
    const auto to_target = ( Destination - MovementComponent->GetActorFeetLocation() ).GetSafeNormal();
    const auto segment_dot = FVector::DotProduct( to_target, current_direction );
    if ( segment_dot > 0.0f )
    {
        return true;
    }

    // get cylinder of moving agent
    auto agent_radius = 0.0f;
    auto agent_half_height = 0.0f;
    const auto * moving_agent = GetOwner();
    moving_agent->GetSimpleCollisionCylinder( agent_radius, agent_half_height );

    // check if they overlap (with added AcceptanceRadius)
    const auto to_goal = Destination - current_location;

    const auto dist_2d_sq = to_goal.SizeSquared2D();
    const auto use_radius = 10.0f + ( agent_radius * 0.05f );
    if ( dist_2d_sq > FMath::Square( use_radius ) )
    {
        return false;
    }

    const auto z_diff = FMath::Abs( to_goal.Z );
    const auto use_height = agent_half_height * 1.05f;
    if ( z_diff > use_height )
    {
        return false;
    }

    return true;
}

bool USFSplineFollowingComponent::HasStoppedSimulation() const
{
    return false;
    // return MovementComponent == nullptr || IsActive() == false;
}

float USFSplineFollowingComponent::GetSimulationTimeStep( float remaining_time, const int32 iterations ) const
{
    if ( remaining_time > MaxSimulationTimeStep )
    {
        if ( iterations < MaxSimulationIterations )
        {
            // Subdivide moves to be no longer than MaxSimulationTimeStep seconds
            remaining_time = FMath::Min( MaxSimulationTimeStep, remaining_time * 0.5f );
        }
        else
        {
            // If this is the last iteration, just use all the remaining time. This is better than cutting things short, as the simulation won't move far enough otherwise.
            // Print a throttled warning.
#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
            if ( const auto * const world = GetWorld() )
            {
                // Don't report during long hitches, we're more concerned about normal behavior just to make sure we have reasonable simulation settings.
                if ( world->DeltaTimeSeconds < 0.20f )
                {
                    static uint32 warning_count = 0;
                }
            }
#endif
        }
    }

    // no less than MIN_TICK_TIME (to avoid potential divide-by-zero during simulation).
    return FMath::Max( min_tick_time, remaining_time );
}
