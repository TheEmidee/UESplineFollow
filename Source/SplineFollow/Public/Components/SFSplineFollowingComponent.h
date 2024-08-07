#pragma once

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "SFSplineFollowingComponent.generated.h"

struct FSFFollowSplineInfos;
class USplineComponent;
class UCharacterMovementComponent;

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class SPLINEFOLLOW_API USFSplineFollowingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USFSplineFollowingComponent();

    UFUNCTION( BlueprintCallable )
    void FollowSpline( const FSFFollowSplineInfos & spline_infos );

    UFUNCTION( BlueprintPure )
    float GetDistanceOnSpline() const;

    UFUNCTION( BlueprintPure )
    float GetNormalizedDistanceOnSpline() const;

    void BeginPlay() override;
    void TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;

private:
    void UpdateDestination( float delta_time );
    void FollowDestination() const;
    bool HasReachedDestination();
    bool HasStoppedSimulation() const;
    float GetSimulationTimeStep( float remaining_time, const int32 iterations ) const;

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

    UPROPERTY( EditDefaultsOnly )
    float SplineSnapMultiplier;

    FVector Destination;
    float DestinationDistance;
};

FORCEINLINE float USFSplineFollowingComponent::GetDistanceOnSpline() const
{
    return DistanceOnSpline;
}
