#pragma once

#include "Components/SFSplineMarkers.h"

#include <CoreMinimal.h>

#include "SFSMA_AttachToSpline.generated.h"

class USFSplineSpeedProvider;

UENUM()
enum class ESWAttachToSplineRotationSpeedType : uint8
{
    KeepSameRotationSpeed,
    ApplyFixedRotationSpeed
};

class ASWSpline;

UCLASS()
class SPLINEFOLLOW_API USFSMA_AttachToSpline final : public USFSplineMarkerAction
{
    GENERATED_BODY()

public:
    USFSMA_AttachToSpline();

    void ExecuteAction_Implementation( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const override;
    void ExecuteStartWindowAction_Implementation( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid(FDataValidationContext & context) const override;
#endif

private:
    void FollowNewSplineInternal( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< AActor > Actor;

    UPROPERTY( EditAnywhere )
    float NormalizedDistanceOnSpline;

    UPROPERTY( EditAnywhere )
    uint8 bEnableMovement : 1;

    UPROPERTY( EditAnywhere )
    uint8 bLoops: 1;

    UPROPERTY( EditAnywhere )
    TSubclassOf< USFSplineSpeedProvider > SpeedProviderClassOverride;

    UPROPERTY( EditAnywhere )
    uint8 bAttachToSpline : 1;

    UPROPERTY( EditAnywhere )
    ESWAttachToSplineRotationSpeedType RotationSpeedType;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "RotationSpeedType == ESWAttachToSplineRotationSpeedType::ApplyFixedRotationSpeed" ) )
    float FixedRotationSpeed;
};
