#pragma once
#include "GameFramework/MovementComponent.h"

#include "SFSplineFollowingMovementComponent.generated.h"

class USFSplineFollowingMovementComponent;
class USplineComponent;
class UCurveFloat;

UCLASS( Abstract, HideDropdown, BlueprintType, Blueprintable )
class USFSplineSpeedProvider : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintNativeEvent )
    void Setup( USplineComponent * followed_spline_component, USFSplineFollowingMovementComponent * spline_following_movement_component );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure )
    float GetSpeed( float normalized_position_on_spline, USplineComponent * followed_spline_component, USFSplineFollowingMovementComponent * spline_following_movement_component, float delta_time );
};

UCLASS( Abstract )
class USFSplineSpeedProvider_Constant final : public USFSplineSpeedProvider
{
    GENERATED_BODY()

public:
    USFSplineSpeedProvider_Constant();

    float GetSpeed_Implementation( float normalized_position_on_spline, USplineComponent * followed_spline_component, USFSplineFollowingMovementComponent * spline_following_movement_component, float delta_time ) override;

private:
    UPROPERTY( EditAnywhere )
    float Speed;
};

UCLASS( Abstract )
class USFSplineSpeedProvider_CurveFloat final : public USFSplineSpeedProvider
{
    GENERATED_BODY()

public:
    float GetSpeed_Implementation( float normalized_position_on_spline, USplineComponent * followed_spline_component, USFSplineFollowingMovementComponent * spline_following_movement_component, float delta_time ) override;

private:
    UPROPERTY( EditAnywhere )
    UCurveFloat * CurveFloat;
};

USTRUCT( BlueprintType )
struct SPLINEFOLLOW_API FSFFollowSplineInfos
{
    GENERATED_BODY()

    FSFFollowSplineInfos();
    FSFFollowSplineInfos( const AActor * actor, float normalized_distance_on_spline, bool it_enables_movement, bool loops, TSubclassOf< USFSplineSpeedProvider > speed_provider_class_override, bool it_attaches_to_spline, bool it_overrides_rotation_speed, float rotation_speed_override );
    FSFFollowSplineInfos( USplineComponent * spline_component, float normalized_distance_on_spline, bool it_enables_movement, bool loops, TSubclassOf< USFSplineSpeedProvider > speed_provider_class_override, bool it_attaches_to_spline, bool it_overrides_rotation_speed, float rotation_speed_override );

    bool NetSerialize( FArchive & archive, UPackageMap * package_map, bool & success );

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    USplineComponent * SplineComponent;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    float NormalizedDistanceOnSpline;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bEnableMovement : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bLoops : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    TSubclassOf< USFSplineSpeedProvider > SpeedProviderClassOverride;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bAttachToSpline : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bOverrideRotationSpeed : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    float RotationSpeedOverride;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnSplineFollowingReachedEndDelegate, AActor *, owner_actor );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnSplineFollowingLoopedDelegate, int, loop_count );
DECLARE_DELEGATE_OneParam( FSWOnSplineFollowingReachedPositionDelegate, float );

UCLASS( ClassGroup = ( "Custom" ), meta = ( BlueprintSpawnableComponent ) )
class SPLINEFOLLOW_API USFSplineFollowingMovementComponent final : public UMovementComponent
{
    GENERATED_BODY()

public:
    USFSplineFollowingMovementComponent();

    USplineComponent * GetFollowedSpline() const;
    float GetCurrentSpeed() const;
    void SetOrientRotationToMovement( bool it_follows_spline_rotation );

    void InitializeComponent() override;
    void BeginPlay() override;
    void TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;
    void UpdateTickRegistration() override;
    void SetUpdatedComponent( USceneComponent * new_updated_component ) override;
    void OnRegister() override;

    UFUNCTION( BlueprintCallable )
    bool FollowSpline( const FSFFollowSplineInfos & follow_spline_infos );

    UFUNCTION( BlueprintCallable )
    void ToggleSplineMovement( bool it_is_active );

    UFUNCTION( BlueprintCallable )
    void UnFollowSpline();

    /*
     * Sets the distance on the spline with 0.0f being the beginning of the spline and the end of the spline being returned by GetSplineLength()
     * The marker actions are not executed
     */
    UFUNCTION( BlueprintCallable )
    void SetDistanceOnSpline( float distance_on_spline );

    /*
     * Sets the distance on the spline with 0.0f being the beginning of the spline and 1.0f being the end.
     * The marker actions are not executed
     */
    UFUNCTION( BlueprintCallable )
    void SetNormalizedDistanceOnSpline( float normalized_distance_on_spline );

    UFUNCTION( BlueprintPure )
    float GetNormalizedDistanceOnSpline() const;

    UFUNCTION( BlueprintPure )
    bool IsFollowingSpline() const;

    void RegisterPositionObserver( const FSWOnSplineFollowingReachedPositionDelegate & delegate, float normalized_position, bool trigger_once = true );

#if WITH_EDITOR
    void PostEditChangeProperty( FPropertyChangedEvent & property_changed_event ) override;
#endif

private:
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

    bool HasStoppedSimulation() const;
    bool CheckStillInWorld();
    bool ShouldUseSubStepping() const;
    float GetSimulationTimeStep( float remaining_time, int32 iterations ) const;
    void UpdateInitialPosition();
    void ProcessSplineMarkers( float distance_on_spline );
    void SetDistanceOnSplineInternal( float distance_on_spline );
    void RefreshComponents();
    void UpdateCurrentSpeed( float delta_time );
    void ProcessPositionObservers( float distance_on_spline );
    void ResetPositionObservers();

    UPROPERTY( BlueprintAssignable )
    FSWOnSplineFollowingReachedEndDelegate OnSplineFollowingReachedEndDelegate;

    UPROPERTY( BlueprintAssignable )
    FSWOnSplineFollowingLoopedDelegate OnSplineFollowingLoopedDelegate;

    /**
     * If true, forces sub-stepping to break up movement into discrete smaller steps to improve accuracy of the trajectory.
     * Objects that move in a straight line typically do *not* need to set this, as movement always uses continuous collision detection (sweeps) so collision is not missed.
     * Sub-stepping is automatically enabled when under the effects of gravity or when homing towards a target.
     * @see MaxSimulationTimeStep, MaxSimulationIterations
     */
    UPROPERTY( EditAnywhere )
    uint8 bForceSubStepping : 1;

    /**
     * Max time delta for each discrete simulation step.
     * Lowering this value can address precision issues with fast-moving objects or complex collision scenarios, at the cost of performance.
     *
     * WARNING: if (MaxSimulationTimeStep * MaxSimulationIterations) is too low for the min framerate, the last simulation step may exceed MaxSimulationTimeStep to complete the simulation.
     * @see MaxSimulationIterations, bForceSubStepping
     */
    UPROPERTY( EditAnywhere, meta = ( ClampMin = "0.0166", ClampMax = "0.50", UIMin = "0.0166", UIMax = "0.50" ) )
    float MaxSimulationTimeStep;

    /**
     * Max number of iterations used for each discrete simulation step.
     * Increasing this value can address precision issues with fast-moving objects or complex collision scenarios, at the cost of performance.
     *
     * WARNING: if (MaxSimulationTimeStep * MaxSimulationIterations) is too low for the min framerate, the last simulation step may exceed MaxSimulationTimeStep to complete the simulation.
     * @see MaxSimulationTimeStep, bForceSubStepping
     */
    UPROPERTY( EditAnywhere, meta = ( ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25" ) )
    int32 MaxSimulationIterations;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    float CurrentSpeed;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    TSubclassOf< USFSplineSpeedProvider > SpeedProviderClass;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bInvertSpeed : 1;

    UPROPERTY( VisibleInstanceOnly, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    USplineComponent * FollowedSplineComponent;

    /*
     * Where to move the updated component on the spline (0.0f is the beginning of the spline, 1.0f is the end)
     */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true, ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0" ) )
    float InitialPosition;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bStartsMovementDuringBeginPlay : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bOrientRotationToMovement : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bLoops : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true, EditCondition = "bLoops" ) )
    uint8 bResetLoopCountWhenStopped : 1;

    UPROPERTY( VisibleInstanceOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    float DistanceOnSpline;

    UPROPERTY( VisibleInstanceOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    float NormalizedDistanceOnSpline;

    UPROPERTY( VisibleInstanceOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    int LoopCount;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    float RotationSpeed;

    TArray< FPositionObserver > PositionObservers;
    int LastProcessedMarkerIndex;
};

FORCEINLINE USplineComponent * USFSplineFollowingMovementComponent::GetFollowedSpline() const
{
    return FollowedSplineComponent;
}

FORCEINLINE float USFSplineFollowingMovementComponent::GetCurrentSpeed() const
{
    return CurrentSpeed;
}

FORCEINLINE void USFSplineFollowingMovementComponent::SetOrientRotationToMovement( const bool it_follows_spline_rotation )
{
    bOrientRotationToMovement = it_follows_spline_rotation;
}

FORCEINLINE bool USFSplineFollowingMovementComponent::ShouldUseSubStepping() const
{
    return bForceSubStepping;
}
