#pragma once

#include "SFSplineFollowingInterface.h"
#include "SFSplineFollowingTypes.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "SFSplineFollowingComponent.generated.h"

class USplineComponent;
class UCharacterMovementComponent;

USTRUCT( BlueprintType )
struct FSFSplineSnapAxes
{
    GENERATED_BODY()

    FSFSplineSnapAxes();

    FVector GetSnapVector() const;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bSnapX : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bSnapY : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite )
    uint8 bSnapZ : 1;
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class SPLINEFOLLOW_API USFSplineFollowingComponent : public UActorComponent, public ISFSplineFollowingInterface
{
    GENERATED_BODY()

public:
    USFSplineFollowingComponent();

    float GetDistanceOnSpline() const override;
    USplineComponent * GetFollowedSplineComponent() const override;

    float GetNormalizedDistanceOnSpline() const override;
    void ToggleSplineMovement( bool it_is_active ) override;
    void SetDistanceOnSpline( float distance_on_spline ) override;
    void SetNormalizedDistanceOnSpline( float normalized_distance_on_spline ) override;
    bool FollowSpline( const FSFFollowSplineInfos & follow_spline_infos ) override;
    void UnFollowSpline() override;
    bool IsFollowingSpline() const override;
    float GetCurrentSpeed() const override;

    void InitializeComponent() override;
    void OnRegister() override;
    void BeginPlay() override;
    void TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;

private:
    void UpdateInitialPosition();
    void UpdateDestination( float delta_time );
    void UpdateDestinationInternal( float delta_time );
    void FollowDestination() const;
    bool HasReachedDestination() const;
    bool HasStoppedSimulation() const;
    float GetSimulationTimeStep( float remaining_time, const int32 iterations ) const;
    void SetDistanceOnSplineInternal( FVector & updated_location, FRotator & updated_rotation, float distance_on_spline );
    void SetMovementComponent();

    UPROPERTY()
    TObjectPtr< UCharacterMovementComponent > MovementComponent;

    UPROPERTY( VisibleInstanceOnly, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< USplineComponent > FollowedSplineComponent;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    float DistanceOnSpline;

    UPROPERTY( EditAnywhere, meta = ( ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25" ) )
    int32 MaxSimulationIterations;

    UPROPERTY( EditAnywhere, meta = ( ClampMin = "0.0166", ClampMax = "0.50", UIMin = "0.0166", UIMax = "0.50" ) )
    float MaxSimulationTimeStep;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    float SplineSnapMultiplier;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true, EditCondition = "SplineSnapMultiplier > 0.0f" ) )
    FSFSplineSnapAxes SplineSnapAxes;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true, ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0" ) )
    float InitialPosition;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bStartsMovementDuringBeginPlay : 1;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bLoops : 1;

    UPROPERTY( VisibleInstanceOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    int LoopCount;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true, EditCondition = "bLoops" ) )
    uint8 bResetLoopCountWhenStopped : 1;

    FVector Destination;
    float DestinationDistance;
    FSFSplineMarkerProcessor SplineMarkerProcessor;
};

FORCEINLINE float USFSplineFollowingComponent::GetDistanceOnSpline() const
{
    return DistanceOnSpline;
}

FORCEINLINE USplineComponent * USFSplineFollowingComponent::GetFollowedSplineComponent() const
{
    return FollowedSplineComponent;
}
