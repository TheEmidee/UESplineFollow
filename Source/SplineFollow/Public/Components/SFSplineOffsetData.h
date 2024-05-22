#pragma once

#include <CoreMinimal.h>
#include <Curves/CurveVector.h>

#include "SFSplineOffsetData.generated.h"

UENUM()
enum class ESFSplineOffsetType : uint8
{
    Location,
    Rotation,
    Scale
};

UCLASS( Abstract, Blueprintable, BlueprintType )
class SPLINEFOLLOW_API USFSplineOffsetData final : public UDataAsset
{
    GENERATED_BODY()

public:
    USFSplineOffsetData();

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FRuntimeVectorCurve OffsetCurve;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bResetOnEnd : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    ESFSplineOffsetType OffsetType;
};
