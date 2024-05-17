#pragma once

#include <CoreMinimal.h>
#include <Curves/CurveVector.h>
#include <UObject/Object.h>

#include "SFSplineOffsetData.generated.h"

UENUM()
enum class ESFSplineOffsetType : uint8
{
    Location,
    Rotation,
    Scale
};

UCLASS()
class SPLINEFOLLOW_API USFSplineOffsetData final : public UObject
{
    GENERATED_BODY()

public:
    USFSplineOffsetData();

    void GetOffsetTransform( FTransform & transform, float delta_time );

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FRuntimeVectorCurve OffsetCurve;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bResetOnEnd : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    ESFSplineOffsetType OffsetType;

    float ElapsedTime;
};
