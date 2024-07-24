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
    void FollowSpline(const FSFFollowSplineInfos & spline_infos);

    void BeginPlay() override;
    void TickComponent( float delta_time, ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;

private:
    void UpdateDestination( float delta_time );
    void FollowDestination();

    UPROPERTY()
    TObjectPtr< UCharacterMovementComponent > MovementComponent;

    UPROPERTY( VisibleInstanceOnly, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< USplineComponent > FollowedSplineComponent;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    float SplineDistance;

    FVector Destination;
};
