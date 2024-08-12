#include "Components/SFSplineFollowingComponent.h"

#include "Components/SFSplineFollowingTypes.h"

#include <Components/SplineComponent.h>
#include <GameFramework/CharacterMovementComponent.h>

// Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code.
static constexpr float min_tick_time = 1e-6f;

USFSplineFollowingComponent::USFSplineFollowingComponent() :
    MovementComponent( nullptr ),
    DistanceOnSpline( 0.0f ),
    MaxSimulationIterations( 4 ),
    MaxSimulationTimeStep( 0.05f ),
    SplineSnapMultiplier( 0.01f ),
    InitialPosition( 0.0f ),
    bStartsMovementDuringBeginPlay( true ),
    bLoops( false ),
    LoopCount( 0 ),
    bResetLoopCountWhenStopped( true ),
    Destination( FVector::ZeroVector ),
    DestinationDistance( 0.0f )
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    bAutoActivate = true;
}

float USFSplineFollowingComponent::GetNormalizedDistanceOnSpline() const
{
    if ( FollowedSplineComponent == nullptr )
    {
        return -1.0f;
    }

    return DistanceOnSpline / FollowedSplineComponent->GetSplineLength();
}

void USFSplineFollowingComponent::ToggleSplineMovement( const bool it_is_active )
{
    const auto it_is_enabled = it_is_active && FollowedSplineComponent;

    if ( it_is_enabled && bResetLoopCountWhenStopped )
    {
        LoopCount = 0;
    }

    SetComponentTickEnabled( it_is_enabled );
}

void USFSplineFollowingComponent::SetDistanceOnSpline( const float distance_on_spline )
{
    auto new_location = FVector::Zero();
    auto new_rotation = FRotator::ZeroRotator;

    SetDistanceOnSplineInternal( new_location, new_rotation, distance_on_spline );
    GetOwner()->SetActorLocation( new_location );
    GetOwner()->SetActorRotation( new_rotation );

    MovementComponent->StopActiveMovement();
    SplineMarkerProcessor.UpdateLastProcessedMarker( GetNormalizedDistanceOnSpline(), MovementComponent->Velocity.Length() );
}

void USFSplineFollowingComponent::SetNormalizedDistanceOnSpline( const float normalized_distance_on_spline )
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    const auto distance_on_spline = normalized_distance_on_spline * FollowedSplineComponent->GetSplineLength();

    SetDistanceOnSpline( distance_on_spline );
}

bool USFSplineFollowingComponent::FollowSpline( const FSFFollowSplineInfos & follow_spline_infos )
{
    if ( !ensureAlwaysMsgf( follow_spline_infos.SplineComponent != nullptr, TEXT( "Invalid spline to follow" ) ) )
    {
        return false;
    }

    if ( follow_spline_infos.SplineComponent == FollowedSplineComponent )
    {
        return false;
    }

    FollowedSplineComponent = follow_spline_infos.SplineComponent;
    SplineMarkerProcessor.Initialize( FollowedSplineComponent );
    LoopCount = 0;

    if ( follow_spline_infos.bAttachToSpline )
    {
        GetOwner()->AttachToComponent( FollowedSplineComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale );
    }

    bLoops = follow_spline_infos.bLoops;

    MovementComponent->StopActiveMovement();

    if ( follow_spline_infos.bOverrideRotationSpeed )
    {
        MovementComponent->RotationRate.Yaw = follow_spline_infos.RotationSpeedOverride;
    }

    SetNormalizedDistanceOnSpline( follow_spline_infos.NormalizedDistanceOnSpline );
    ToggleSplineMovement( follow_spline_infos.bEnableMovement );

    return true;
}

void USFSplineFollowingComponent::UnFollowSpline()
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    ToggleSplineMovement( false );

    FollowedSplineComponent = nullptr;

    GetOwner()->DetachFromActor( FDetachmentTransformRules::KeepWorldTransform );
}

bool USFSplineFollowingComponent::IsFollowingSpline() const
{
    return FollowedSplineComponent != nullptr && IsComponentTickEnabled();
}

void USFSplineFollowingComponent::InitializeComponent()
{
    Super::InitializeComponent();

    SetComponentTickEnabled( false );

    if ( MovementComponent == nullptr )
    {
        SetMovementComponent();
    }
}

void USFSplineFollowingComponent::OnRegister()
{
    Super::OnRegister();

    if ( MovementComponent == nullptr )
    {
        SetMovementComponent();
    }
}

void USFSplineFollowingComponent::BeginPlay()
{
    Super::BeginPlay();

    UpdateInitialPosition();
    ToggleSplineMovement( bStartsMovementDuringBeginPlay );
}

void USFSplineFollowingComponent::TickComponent( const float delta_time, const ELevelTick tick_type, FActorComponentTickFunction * this_tick_function )
{
    if ( HasStoppedSimulation() )
    {
        return;
    }

    Super::TickComponent( delta_time, tick_type, this_tick_function );

    if ( MovementComponent == nullptr )
    {
        return;
    }

    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

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

void USFSplineFollowingComponent::UpdateInitialPosition()
{
    if ( FollowedSplineComponent != nullptr && MovementComponent != nullptr )
    {
        SetNormalizedDistanceOnSpline( InitialPosition );
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

    DistanceOnSpline = DestinationDistance;
    DestinationDistance = DistanceOnSpline + MovementComponent->GetMaxSpeed() * delta_time;

    const auto spline_length = FollowedSplineComponent->GetSplineLength();

    if ( DestinationDistance >= spline_length )
    {
        if ( bLoops )
        {
            SplineMarkerProcessor.ProcessSplineMarkers( spline_length, MovementComponent->Velocity.Length(), GetOwner() );
            SplineMarkerProcessor.Reset();
            DestinationDistance -= spline_length;

            ++LoopCount;
        }
        else
        {
            DestinationDistance = spline_length;
        }
    }
    else
    {
        SplineMarkerProcessor.ProcessSplineMarkers( DistanceOnSpline, MovementComponent->Velocity.Length(), GetOwner() );
    }

    Destination = FollowedSplineComponent->GetLocationAtDistanceAlongSpline( DestinationDistance, ESplineCoordinateSpace::World );
}

void USFSplineFollowingComponent::FollowDestination() const
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    const auto current_location = FollowedSplineComponent->GetLocationAtDistanceAlongSpline( DistanceOnSpline, ESplineCoordinateSpace::World );
    const auto actor_location = MovementComponent->GetActorFeetLocation();
    auto desired_movement = Destination - current_location;

    if ( !MovementComponent->CurrentRootMotion.HasAdditiveVelocity() )
    {
        const auto spline_offset = actor_location - current_location;
        const auto spline_offset_dir = spline_offset.GetSafeNormal();
        const auto spline_offset_distance = spline_offset.SquaredLength();

        desired_movement -= spline_offset_dir * spline_offset_distance * SplineSnapMultiplier;
    }

    const auto move_input = desired_movement.GetSafeNormal();
    MovementComponent->AddInputVector( move_input );
}

bool USFSplineFollowingComponent::HasReachedDestination() const
{
    if ( MovementComponent == nullptr )
    {
        return false;
    }

    const auto current_direction = MovementComponent->Velocity.GetSafeNormal();
    const auto current_location = MovementComponent->GetActorFeetLocation();

    // check if moved too far
    const auto to_target = ( Destination - current_location ).GetSafeNormal();
    const auto segment_dot = FVector::DotProduct( to_target, current_direction );
    if ( segment_dot < 0.0f )
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
    return MovementComponent == nullptr || IsActive() == false;
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
    }

    // no less than MIN_TICK_TIME (to avoid potential divide-by-zero during simulation).
    return FMath::Max( min_tick_time, remaining_time );
}

void USFSplineFollowingComponent::SetDistanceOnSplineInternal( FVector & updated_location, FRotator & updated_rotation, float distance_on_spline )
{
    if ( FollowedSplineComponent == nullptr || MovementComponent == nullptr )
    {
        return;
    }

    updated_location = FollowedSplineComponent->GetLocationAtDistanceAlongSpline( distance_on_spline, ESplineCoordinateSpace::World );

    if ( MovementComponent->bOrientRotationToMovement )
    {
        updated_rotation = FollowedSplineComponent->GetRotationAtDistanceAlongSpline( distance_on_spline, ESplineCoordinateSpace::World );
    }

    DistanceOnSpline = distance_on_spline;
}

void USFSplineFollowingComponent::SetMovementComponent()
{
    MovementComponent = GetOwner()->GetComponentByClass< UCharacterMovementComponent >();
}
