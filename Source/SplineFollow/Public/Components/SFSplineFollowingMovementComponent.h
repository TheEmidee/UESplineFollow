#pragma once

#include "SFSplineFollowingInterface.h"
#include "SFSplineFollowingTypes.h"

#include <Curves/CurveVector.h>
#include <GameFramework/MovementComponent.h>

#include "SFSplineFollowingMovementComponent.generated.h"

enum class ESFSplineOffsetType : uint8;
class USFSplineOffsetData;
class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSFOnSplineOffsetFinishedDelegate, USFSplineOffsetData *, offset_data );

USTRUCT( BlueprintType )
struct FSFRotationConstraints
{
    GENERATED_BODY()

    FSFRotationConstraints();

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bConstrainX : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bConstrainY : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bConstrainZ : 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnSplineFollowingReachedEndDelegate, AActor *, owner_actor );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnSplineFollowingLoopedDelegate, int, loop_count );
DECLARE_DELEGATE_OneParam( FSWOnSplineFollowingReachedPositionDelegate, float );

UCLASS( ClassGroup = ( "Custom" ), meta = ( BlueprintSpawnableComponent ) )
class SPLINEFOLLOW_API USFSplineFollowingMovementComponent final : public UMovementComponent, public ISFSplineFollowingInterface
{
    GENERATED_BODY()

public:
    USFSplineFollowingMovementComponent();

    USplineComponent * GetFollowedSpline() const;
    float GetCurrentSpeed() const override;
    void SetOrientRotationToMovement( bool it_follows_spline_rotation );
    float GetDistanceOnSpline() const override;
    USplineComponent * GetFollowedSplineComponent() const override;

    void InitializeComponent() override;
    void BeginPlay() override;
    void TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;
    void UpdateTickRegistration() override;
    void SetUpdatedComponent( USceneComponent * new_updated_component ) override;
    void OnRegister() override;

    bool FollowSpline( const FSFFollowSplineInfos & follow_spline_infos ) override;

    void ToggleSplineMovement( bool it_is_active ) override;
    void UnFollowSpline() override;

    /*
     * Sets the distance on the spline with 0.0f being the beginning of the spline and the end of the spline being returned by GetSplineLength()
     * The marker actions are not executed
     */
    void SetDistanceOnSpline( float distance_on_spline ) override;

    /*
     * Sets the distance on the spline with 0.0f being the beginning of the spline and 1.0f being the end.
     * The marker actions are not executed
     */
    void SetNormalizedDistanceOnSpline( float normalized_distance_on_spline ) override;

    float GetNormalizedDistanceOnSpline() const override;

    bool IsFollowingSpline() const override;

    UFUNCTION( BlueprintCallable )
    float AddSplineOffsetData( USFSplineOffsetData * offset_data );

    UFUNCTION( BlueprintCallable )
    void ResetMarkersUsage();

    UFUNCTION( BlueprintCallable )
    void SetInvertSpeed( bool invert );

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

    struct FSFSplineOffsetInfo
    {
        FSFSplineOffsetInfo();
        FSFSplineOffsetInfo( USFSplineOffsetData * offset_data );

        void Initialize();
        bool ApplyOffsetToTransform( FTransform & transform, float delta_time );

        TObjectPtr< USFSplineOffsetData > OffsetData;
        FRuntimeVectorCurve OffsetCurve;
        ESFSplineOffsetType OffsetType;
        bool bResetOnEnd;
        float ElapsedTime;
        float MaxTime;
    };

    bool ShouldUseSubStepping() const;

    bool HasStoppedSimulation() const;
    bool CheckStillInWorld();
    float GetSimulationTimeStep( float remaining_time, int32 iterations ) const;
    void UpdateInitialPosition();
    void SetDistanceOnSplineInternal( FVector & updated_location, FRotator & updated_rotation, float distance_on_spline );
    void RefreshComponents();
    void UpdateCurrentSpeed( float delta_time );
    void ProcessPositionObservers( float distance_on_spline );
    void ResetPositionObservers();
    void ConstrainRotation( FRotator & rotation ) const;
    void ApplyOffsetData( FTransform & transform, float delta_time );

    UPROPERTY( BlueprintAssignable )
    FSWOnSplineFollowingReachedEndDelegate OnSplineFollowingReachedEndDelegate;

    UPROPERTY( BlueprintAssignable )
    FSWOnSplineFollowingLoopedDelegate OnSplineFollowingLoopedDelegate;

    UPROPERTY( BlueprintAssignable )
    FSFOnSplineOffsetFinishedDelegate OnSplineOffsetFinishedDelegate;

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

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< USFSplineSpeedProvider > SpeedProvider;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
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

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true, EditCondition = "bOrientRotationToMovement", EditConditionHides ) )
    FSFRotationConstraints RotationConstraints;

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

    TArray< FSFSplineOffsetInfo > SplineOffsetDatas;
    TArray< FPositionObserver > PositionObservers;
    FSFSplineMarkerProcessor SplineMarkerProcessor;
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

FORCEINLINE float USFSplineFollowingMovementComponent::GetDistanceOnSpline() const
{
    return DistanceOnSpline;
}

FORCEINLINE USplineComponent * USFSplineFollowingMovementComponent::GetFollowedSplineComponent() const
{
    return FollowedSplineComponent;
}

FORCEINLINE bool USFSplineFollowingMovementComponent::ShouldUseSubStepping() const
{
    return bForceSubStepping;
}
