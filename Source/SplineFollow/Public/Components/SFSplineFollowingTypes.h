#pragma once

#include "SFSplineFollowingTypes.generated.h"

class USFSplineFollowingMovementComponent;
class USplineComponent;

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

USTRUCT()
struct FSFSplineMarkerProcessor
{
    GENERATED_BODY()

public:
    FSFSplineMarkerProcessor();

    void Initialize( USplineComponent * spline_component );

    void ProcessSplineMarkers( float distance_on_spline, float current_speed, AActor * owner );
    void TryUpdateLastProcessedMarker( const float distance_on_spline, const float current_speed );
    void UpdateLastProcessedMarker( const float distance_on_spline, const float current_speed );
    void Reset();

private:
    TObjectPtr< USplineComponent > SplineComponent;
    int LastProcessedMarkerIndex;
    uint8 bUpdateLastProcessedMarker : 1;
};