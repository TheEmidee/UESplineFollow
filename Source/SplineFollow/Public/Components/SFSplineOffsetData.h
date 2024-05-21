#pragma once

#include <CoreMinimal.h>
#include <Curves/CurveVector.h>
#include <UObject/Object.h>

#include "SFSplineOffsetData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSFOnSplineOffsetFinishedDelegate, USFSplineOffsetData *, offset_data );

UENUM()
enum class ESFSplineOffsetType : uint8
{
    Location,
    Rotation,
    Scale
};

UCLASS( Abstract, Blueprintable, BlueprintType )
class SPLINEFOLLOW_API USFSplineOffsetData final : public UObject
{
    GENERATED_BODY()

public:
    USFSplineOffsetData();

    void Initialize();
    bool ApplyOffsetToTransform( FTransform & transform, float delta_time );

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FRuntimeVectorCurve OffsetCurve;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bResetOnEnd : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    ESFSplineOffsetType OffsetType;

    UPROPERTY( BlueprintAssignable )
    FSFOnSplineOffsetFinishedDelegate OnSplineOffsetFinishedDelegate;

    float ElapsedTime;
    float MaxTime;
};
