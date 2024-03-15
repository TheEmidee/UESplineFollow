#include "Components/SFSplineFollowingMovementComponent.h"

#include "Components/SFSplineComponent.h"
#include "Components/SFSplineMarkers.h"
#include "Components/SplineComponent.h"

// Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code.
static constexpr float MinTickTime = 1e-6f;

void USFSplineSpeedProvider::Setup_Implementation( USplineComponent * followed_spline_component, USFSplineFollowingMovementComponent * spline_following_movement_component )
{
}

float USFSplineSpeedProvider::GetSpeed_Implementation( float normalized_position_on_spline, USplineComponent * followed_spline_component, float delta_time )
{
    return 0.0f;
}

USFSplineSpeedProvider_Constant::USFSplineSpeedProvider_Constant() :
    Speed( 100.0f )
{
}

float USFSplineSpeedProvider_Constant::GetSpeed_Implementation( float /*normalized_position_on_spline*/, USplineComponent * /*followed_spline_component*/, float /*delta_time*/ )
{
    return Speed;
}

float USFSplineSpeedProvider_CurveFloat::GetSpeed_Implementation( const float normalized_position_on_spline, USplineComponent * /*followed_spline_component*/, float /*delta_time*/ )
{
    if ( CurveFloat == nullptr )
    {
        return 0.0f;
    }

    return CurveFloat->GetFloatValue( normalized_position_on_spline );
}

FSFFollowSplineInfos::FSFFollowSplineInfos()
{
    SplineComponent = nullptr;
    NormalizedDistanceOnSpline = 0.0f;
    bEnableMovement = false;
    bLoops = false;
    SpeedProviderClassOverride = nullptr;
    bAttachToSpline = true;
    bOverrideRotationSpeed = false;
    RotationSpeedOverride = 0.0f;
}

FSFFollowSplineInfos::FSFFollowSplineInfos( const AActor * actor, const float normalized_distance_on_spline, const bool it_enables_movement, const bool loops, TSubclassOf< USFSplineSpeedProvider > speed_provider_class_override, const bool it_attaches_to_spline, const bool it_overrides_rotation_speed, const float rotation_speed_override ) :
    SplineComponent( actor->GetComponentByClass< USplineComponent >() ),
    NormalizedDistanceOnSpline( normalized_distance_on_spline ),
    bEnableMovement( it_enables_movement ),
    bLoops( loops ),
    SpeedProviderClassOverride( speed_provider_class_override ),
    bAttachToSpline( it_attaches_to_spline ),
    bOverrideRotationSpeed( it_overrides_rotation_speed ),
    RotationSpeedOverride( rotation_speed_override )
{}

FSFFollowSplineInfos::FSFFollowSplineInfos( USplineComponent * const spline_component, const float normalized_distance_on_spline, const bool it_enables_movement, const bool loops, TSubclassOf< USFSplineSpeedProvider > speed_provider_class_override, const bool it_attaches_to_spline, const bool it_overrides_rotation_speed, const float rotation_speed_override ) :
    SplineComponent( spline_component ),
    NormalizedDistanceOnSpline( normalized_distance_on_spline ),
    bEnableMovement( it_enables_movement ),
    bLoops( loops ),
    SpeedProviderClassOverride( speed_provider_class_override ),
    bAttachToSpline( it_attaches_to_spline ),
    bOverrideRotationSpeed( it_overrides_rotation_speed ),
    RotationSpeedOverride( rotation_speed_override )
{}

bool FSFFollowSplineInfos::NetSerialize( FArchive & archive, UPackageMap * /* package_map */, bool & success )
{
    archive << SplineComponent;
    archive << NormalizedDistanceOnSpline;

    auto value = bEnableMovement;
    archive.SerializeBits( &value, 1 );

    archive << SpeedProviderClassOverride;

    value = bAttachToSpline;
    archive.SerializeBits( &value, 1 );

    value = bOverrideRotationSpeed;
    archive.SerializeBits( &value, 1 );

    archive << RotationSpeedOverride;

    success = true;
    return true;
}

USFSplineFollowingMovementComponent::USFSplineFollowingMovementComponent() :
    bForceSubStepping( true ),
    FollowedSplineComponent( nullptr ),
    NormalizedDistanceOnSpline( 0 )
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    MaxSimulationTimeStep = 0.05f;
    MaxSimulationIterations = 4;
    CurrentSpeed = 0.0f;
    bInvertSpeed = false;
    InitialPosition = 0.0f;
    LastProcessedMarkerIndex = INDEX_NONE;
    bStartsMovementDuringBeginPlay = true;
    bOrientRotationToMovement = false;
    bLoops = false;
    bResetLoopCountWhenStopped = true;
    DistanceOnSpline = 0.0f;
    LoopCount = 0;
    RotationSpeed = 100.0f;
}

void USFSplineFollowingMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();

    SetComponentTickEnabled( false );
}

void USFSplineFollowingMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    UpdateInitialPosition();

    ToggleSplineMovement( bStartsMovementDuringBeginPlay );
}

void USFSplineFollowingMovementComponent::TickComponent( const float delta_time, const ELevelTick tick_type, FActorComponentTickFunction * this_tick_function )
{
    if ( HasStoppedSimulation() || ShouldSkipUpdate( delta_time ) )
    {
        return;
    }

    Super::TickComponent( delta_time, tick_type, this_tick_function );

    if ( !IsValid( UpdatedComponent ) )
    {
        return;
    }

    const auto * actor_owner = UpdatedComponent->GetOwner();
    if ( actor_owner == nullptr || !CheckStillInWorld() )
    {
        return;
    }

    if ( UpdatedComponent->IsSimulatingPhysics() )
    {
        return;
    }

    if ( !IsValid( FollowedSplineComponent ) )
    {
        return;
    }

    auto remaining_time = delta_time;
    auto iterations = 0;

    const auto * followed_spline = FollowedSplineComponent;

    const auto spline_length = followed_spline->GetSplineLength();
    const auto current_world_location = UpdatedComponent->GetComponentLocation();

    while ( remaining_time >= MinTickTime && ( iterations < MaxSimulationIterations ) && IsValid( actor_owner ) && !HasStoppedSimulation() )
    {
        iterations++;

        const auto time_tick = ShouldUseSubStepping()
                                   ? GetSimulationTimeStep( remaining_time, iterations )
                                   : remaining_time;
        remaining_time -= time_tick;

        UpdateCurrentSpeed( delta_time );

        if ( bInvertSpeed )
        {
            CurrentSpeed *= -1.0f;
        }

        auto distance_on_spline = DistanceOnSpline + time_tick * CurrentSpeed;

        const auto must_wrap_around = CurrentSpeed > 0.0f ? ( distance_on_spline >= spline_length ) : ( distance_on_spline <= 0.0f );

        const auto set_distance_on_spline = [ & ]( const float distance ) {
            SetDistanceOnSplineInternal( distance );
            ProcessSplineMarkers( distance );
            ProcessPositionObservers( distance );
        };

        if ( !must_wrap_around )
        {
            set_distance_on_spline( distance_on_spline );
        }
        else if ( !bLoops )
        {
            const auto clamped_distance = CurrentSpeed > 0.0f ? spline_length : 0.0f;
            set_distance_on_spline( clamped_distance );

            // Disable tick before broadcasting the event as the event may want to re-enable the movement in the same frame
            SetComponentTickEnabled( false );
            OnSplineFollowingReachedEndDelegate.Broadcast( GetOwner() );
            // UAbilitySystemBlueprintLibrary::SendGameplayEventToActor( GetOwner(), SwarmsTag_Event_Gameplay_Spline_ReachedEnd, FGameplayEventData() );
        }
        else
        {
            // First process up to the end of the spline (or the beginning if going in reverse)
            const auto clamped_distance = CurrentSpeed > 0.0f ? spline_length : 0.0f;
            ProcessSplineMarkers( clamped_distance );
            ProcessPositionObservers( clamped_distance );

            if ( followed_spline != FollowedSplineComponent )
            {
                return;
            }

            // Reset the flag bHasBeenTriggered on all observers to make sure they can trigger again
            ResetPositionObservers();

            // Now set the distance at the correct distance past the spline end (or beginning if going in reverse)
            distance_on_spline = CurrentSpeed > 0.0f
                                     ? FMath::Fmod( distance_on_spline, spline_length )
                                     : spline_length - FMath::Abs( distance_on_spline );
            LastProcessedMarkerIndex = INDEX_NONE;

            set_distance_on_spline( distance_on_spline );

            LoopCount++;
            OnSplineFollowingLoopedDelegate.Broadcast( LoopCount );
        }

        const auto new_world_location = UpdatedComponent->GetComponentLocation();

        const auto delta = new_world_location - current_world_location;
        Velocity = delta / time_tick;

        UpdateComponentVelocity();

        if ( bOrientRotationToMovement )
        {
            const auto current_rotator = UpdatedComponent->GetComponentRotation();
            const auto target_rotator = Velocity.GetSafeNormal().Rotation().GetNormalized();

            const auto final_rotation = FMath::RInterpTo( current_rotator, target_rotator, delta_time, RotationSpeed );

            UpdatedComponent->SetWorldRotation( final_rotation );
        }
    }
}

void USFSplineFollowingMovementComponent::UpdateTickRegistration()
{
    if ( !bStartsMovementDuringBeginPlay || FollowedSplineComponent == nullptr )
    {
        SetComponentTickEnabled( false );
    }
    else
    {
        Super::UpdateTickRegistration();
    }
}

void USFSplineFollowingMovementComponent::SetUpdatedComponent( USceneComponent * new_updated_component )
{
    Super::SetUpdatedComponent( new_updated_component );

    UpdateInitialPosition();
}

void USFSplineFollowingMovementComponent::OnRegister()
{
    Super::OnRegister();

    RefreshComponents();
}

bool USFSplineFollowingMovementComponent::FollowSpline( const FSFFollowSplineInfos & follow_spline_infos )
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
    LastProcessedMarkerIndex = INDEX_NONE;
    LoopCount = 0;

    if ( follow_spline_infos.bAttachToSpline )
    {
        GetOwner()->AttachToComponent( FollowedSplineComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale );
    }

    bLoops = follow_spline_infos.bLoops;

    SetNormalizedDistanceOnSpline( follow_spline_infos.NormalizedDistanceOnSpline );
    ToggleSplineMovement( follow_spline_infos.bEnableMovement );

    if ( follow_spline_infos.SpeedProviderClassOverride != nullptr )
    {
        SpeedProviderClass = follow_spline_infos.SpeedProviderClassOverride;
        SpeedProviderClass->GetDefaultObject< USFSplineSpeedProvider >()->Setup( FollowedSplineComponent, this );
    }

    if ( follow_spline_infos.bOverrideRotationSpeed )
    {
        RotationSpeed = follow_spline_infos.RotationSpeedOverride;
    }

    return true;
}

void USFSplineFollowingMovementComponent::ToggleSplineMovement( const bool it_is_active )
{
    const auto it_is_enabled = it_is_active && FollowedSplineComponent;

    if ( it_is_enabled && bResetLoopCountWhenStopped )
    {
        LoopCount = 0;
    }

    SetComponentTickEnabled( it_is_enabled );
}

void USFSplineFollowingMovementComponent::UnFollowSpline()
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    ToggleSplineMovement( false );

    FollowedSplineComponent = nullptr;

    GetOwner()->DetachFromActor( FDetachmentTransformRules::KeepWorldTransform );
}

void USFSplineFollowingMovementComponent::SetDistanceOnSpline( const float distance_on_spline )
{
    SetDistanceOnSplineInternal( distance_on_spline );

    if ( const auto * spline_component = Cast< USFSplineComponent >( FollowedSplineComponent ) )
    {
        const auto & spline_marker_proxies = spline_component->GetSplineMarkerProxies();
        const auto spline_length = spline_component->GetSplineLength();

        UpdateCurrentSpeed( 0.0f );

        if ( CurrentSpeed > 0.0f )
        {
            for ( auto index = 0; index < spline_marker_proxies.Num(); ++index )
            {
                const auto & marker_proxy = spline_marker_proxies[ index ];
                const auto marker_distance = spline_length * marker_proxy.SplineNormalizedDistance;

                if ( marker_distance > distance_on_spline )
                {
                    break;
                }

                LastProcessedMarkerIndex = index;
            }
        }
        else
        {
            for ( auto index = spline_marker_proxies.Num() - 1; index >= 0; --index )
            {
                const auto & marker_proxy = spline_marker_proxies[ index ];
                const auto marker_distance = spline_length * marker_proxy.SplineNormalizedDistance;

                if ( marker_distance < distance_on_spline )
                {
                    break;
                }

                LastProcessedMarkerIndex = index;
            }
        }
    }
}

void USFSplineFollowingMovementComponent::SetNormalizedDistanceOnSpline( const float normalized_distance_on_spline )
{
    if ( FollowedSplineComponent == nullptr )
    {
        return;
    }

    const auto distance_on_spline = normalized_distance_on_spline * FollowedSplineComponent->GetSplineLength();

    SetDistanceOnSpline( distance_on_spline );
}

float USFSplineFollowingMovementComponent::GetNormalizedDistanceOnSpline() const
{
    if ( FollowedSplineComponent == nullptr )
    {
        return -1.0f;
    }

    return DistanceOnSpline / FollowedSplineComponent->GetSplineLength();
}

bool USFSplineFollowingMovementComponent::IsFollowingSpline() const
{
    return FollowedSplineComponent != nullptr && IsComponentTickEnabled();
}

void USFSplineFollowingMovementComponent::RegisterPositionObserver( const FSWOnSplineFollowingReachedPositionDelegate & delegate, float normalized_position, bool trigger_once /*= true*/ )
{
    FPositionObserver observer;
    observer.Callback = delegate;
    observer.NormalizedPosition = normalized_position;
    observer.bTriggerOnce = trigger_once;

    PositionObservers.Emplace( MoveTemp( observer ) );

    ProcessPositionObservers( DistanceOnSpline );
}

#if WITH_EDITOR

void USFSplineFollowingMovementComponent::PostEditChangeProperty( FPropertyChangedEvent & property_changed_event )
{
    Super::PostEditChangeProperty( property_changed_event );

    if ( property_changed_event.GetPropertyName() == GET_MEMBER_NAME_CHECKED( USFSplineFollowingMovementComponent, InitialPosition ) )
    {
        RefreshComponents();
    }
}
#endif

bool USFSplineFollowingMovementComponent::HasStoppedSimulation() const
{
    return UpdatedComponent == nullptr || IsActive() == false;
}

bool USFSplineFollowingMovementComponent::CheckStillInWorld()
{
    // copy / paste from the projectile movement component
    if ( UpdatedComponent == nullptr )
    {
        return false;
    }

    const UWorld * my_world = GetWorld();
    if ( my_world == nullptr )
    {
        return false;
    }

    // check the variations of KillZ
    const auto * world_settings = my_world->GetWorldSettings( true );
    if ( !world_settings->bEnableWorldBoundsChecks )
    {
        return true;
    }
    auto * actor_owner = UpdatedComponent->GetOwner();
    if ( !IsValid( actor_owner ) )
    {
        return false;
    }
    if ( actor_owner->GetActorLocation().Z < world_settings->KillZ )
    {
        const auto * damage_type = world_settings->KillZDamageType ? world_settings->KillZDamageType->GetDefaultObject< UDamageType >() : GetDefault< UDamageType >();
        actor_owner->FellOutOfWorld( *damage_type );
        return false;
    }

    // Check if box has poked outside the world
    if ( UpdatedComponent != nullptr && UpdatedComponent->IsRegistered() )
    {
        const auto & box = UpdatedComponent->Bounds.GetBox();
        if ( box.Min.X < -HALF_WORLD_MAX || box.Max.X > HALF_WORLD_MAX ||
             box.Min.Y < -HALF_WORLD_MAX || box.Max.Y > HALF_WORLD_MAX ||
             box.Min.Z < -HALF_WORLD_MAX || box.Max.Z > HALF_WORLD_MAX )
        {
            // UE_LOG( LogTemp, Warning, TEXT( "%s is outside the world bounds!" ), *GetNameSafe( actor_owner ) );
            actor_owner->OutsideWorldBounds();
            // not safe to use physics or collision at this point
            actor_owner->SetActorEnableCollision( false );
            ToggleSplineMovement( false );
            return false;
        }
    }
    return true;
}

float USFSplineFollowingMovementComponent::GetSimulationTimeStep( float remaining_time, const int32 iterations ) const
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
                    if ( ( warning_count++ < 100 ) || ( GFrameCounter & 15 ) == 0 )
                    {
                        UE_LOG( LogTemp, Warning, TEXT( "GetSimulationTimeStep() - Max iterations %d hit while remaining time %.6f > MaxSimulationTimeStep (%.3f) for '%s'" ), MaxSimulationIterations, remaining_time, MaxSimulationTimeStep, *GetPathNameSafe( UpdatedComponent ) );
                    }
                }
            }
#endif
        }
    }

    // no less than MIN_TICK_TIME (to avoid potential divide-by-zero during simulation).
    return FMath::Max( MinTickTime, remaining_time );
}

void USFSplineFollowingMovementComponent::UpdateInitialPosition()
{
    if ( FollowedSplineComponent != nullptr && UpdatedComponent != nullptr )
    {
        SetNormalizedDistanceOnSpline( InitialPosition );
    }
}

void USFSplineFollowingMovementComponent::ProcessSplineMarkers( const float distance_on_spline )
{
    if ( const auto * spline_component = Cast< USFSplineComponent >( FollowedSplineComponent ) )
    {
        const auto & spline_marker_proxies = spline_component->GetSplineMarkerProxies();
        const auto spline_length = spline_component->GetSplineLength();

        if ( CurrentSpeed > 0.0f )
        {
            const auto first_index = LastProcessedMarkerIndex == INDEX_NONE
                                         ? 0
                                         : LastProcessedMarkerIndex + 1;

            for ( auto index = first_index; index < spline_marker_proxies.Num(); ++index )
            {
                const auto & marker_proxy = spline_marker_proxies[ index ];
                const auto marker_distance = spline_length * marker_proxy.SplineNormalizedDistance;

                if ( distance_on_spline >= marker_distance )
                {
                    // Call the function after as it may attach the actor on another spline. this would reset LastProcessedMarkerIndex, but we would override the value after
                    LastProcessedMarkerIndex = index;
                    marker_proxy.Function( GetOwner() );
                }
            }
        }
        else
        {
            const auto first_index = LastProcessedMarkerIndex == INDEX_NONE
                                         ? spline_marker_proxies.Num() - 1
                                         : LastProcessedMarkerIndex - 1;

            for ( auto index = first_index; index >= 0; --index )
            {
                const auto & marker_proxy = spline_marker_proxies[ index ];
                const auto marker_distance = spline_length * marker_proxy.SplineNormalizedDistance;

                if ( distance_on_spline <= marker_distance )
                {
                    // Call the function after as it may attach the actor on another spline. this would reset LastProcessedMarkerIndex, but we would override the value after
                    LastProcessedMarkerIndex = index;
                    marker_proxy.Function( GetOwner() );
                }
            }
        }
    }
}

void USFSplineFollowingMovementComponent::SetDistanceOnSplineInternal( const float distance_on_spline )
{
    if ( FollowedSplineComponent == nullptr || UpdatedComponent == nullptr )
    {
        return;
    }

    const auto spline_location_at_distance = FollowedSplineComponent->GetLocationAtDistanceAlongSpline( distance_on_spline, ESplineCoordinateSpace::World );

    UpdatedComponent->SetWorldLocation( spline_location_at_distance );
    DistanceOnSpline = distance_on_spline;
    NormalizedDistanceOnSpline = DistanceOnSpline / FollowedSplineComponent->GetSplineLength();
}

void USFSplineFollowingMovementComponent::RefreshComponents()
{
    if ( !IsValid( UpdatedComponent ) && bAutoRegisterUpdatedComponent )
    {
        SetUpdatedComponent( GetOwner()->GetRootComponent() );
    }
    else
    {
        UpdateInitialPosition();
    }
}

void USFSplineFollowingMovementComponent::UpdateCurrentSpeed( float delta_time )
{
    if ( SpeedProviderClass != nullptr )
    {
        CurrentSpeed = FMath::Max( 0.0f, SpeedProviderClass->GetDefaultObject< USFSplineSpeedProvider >()->GetSpeed( GetNormalizedDistanceOnSpline(), FollowedSplineComponent, delta_time ) );
    }
    else
    {
        CurrentSpeed = 0.0f;
    }
}

void USFSplineFollowingMovementComponent::ProcessPositionObservers( float distance_on_spline )
{
    const auto normalized_position_on_spline = distance_on_spline / FollowedSplineComponent->GetSplineLength();

    for ( auto index = PositionObservers.Num() - 1; index >= 0; --index )
    {
        auto & observer = PositionObservers[ index ];

        if ( observer.bHasBeenTriggered )
        {
            continue;
        }

        bool can_execute_callback;
        if ( CurrentSpeed > 0.0f )
        {
            can_execute_callback = normalized_position_on_spline >= observer.NormalizedPosition;
        }
        else
        {
            can_execute_callback = normalized_position_on_spline <= observer.NormalizedPosition;
        }

        if ( can_execute_callback )
        {
            // Create a copy because it can be removed
            // We need to execute the callback after it has been removed to avoid it being re-triggered again if the callback calls RegisterObserver again
            const auto observer_copy = observer;

            if ( observer.bTriggerOnce )
            {
                PositionObservers.RemoveAt( index );
            }
            else
            {
                observer.bHasBeenTriggered = true;
            }

            observer_copy.Callback.ExecuteIfBound( observer.NormalizedPosition );
        }
    }
}

void USFSplineFollowingMovementComponent::ResetPositionObservers()
{
    for ( auto & observer : PositionObservers )
    {
        observer.bHasBeenTriggered = false;
    }
}
