#include "SplineFollow/Public/Components/SFSplineFollowingTypes.h"

#include <Components/SplineComponent.h>

void USFSplineSpeedProvider::Setup_Implementation( USplineComponent * followed_spline_component, USFSplineFollowingMovementComponent * spline_following_movement_component )
{
}

float USFSplineSpeedProvider::GetSpeed_Implementation( float normalized_position_on_spline, USplineComponent * followed_spline_component, USFSplineFollowingMovementComponent * /* spline_following_movement_component */, float delta_time )
{
    return 0.0f;
}

USFSplineSpeedProvider_Constant::USFSplineSpeedProvider_Constant() :
    Speed( 100.0f )
{
}

float USFSplineSpeedProvider_Constant::GetSpeed_Implementation( float /*normalized_position_on_spline*/, USplineComponent * /*followed_spline_component*/, USFSplineFollowingMovementComponent * /* spline_following_movement_component */, float /*delta_time*/ )
{
    return Speed;
}

float USFSplineSpeedProvider_CurveFloat::GetSpeed_Implementation( const float normalized_position_on_spline, USplineComponent * /*followed_spline_component*/, USFSplineFollowingMovementComponent * /* spline_following_movement_component */, float /*delta_time*/ )
{
    if ( CurveFloat == nullptr )
    {
        return 0.0f;
    }

    return CurveFloat->GetFloatValue( normalized_position_on_spline );
}

FSFFollowSplineInfos::FSFFollowSplineInfos()
{
    SplineComponent = nullptr;
    NormalizedDistanceOnSpline = 0.0f;
    bEnableMovement = false;
    bLoops = false;
    SpeedProviderClassOverride = nullptr;
    bAttachToSpline = true;
    bOverrideRotationSpeed = false;
    RotationSpeedOverride = 0.0f;
}

FSFFollowSplineInfos::FSFFollowSplineInfos( const AActor * actor, const float normalized_distance_on_spline, const bool it_enables_movement, const bool loops, TSubclassOf< USFSplineSpeedProvider > speed_provider_class_override, const bool it_attaches_to_spline, const bool it_overrides_rotation_speed, const float rotation_speed_override ) :
    SplineComponent( actor->GetComponentByClass< USplineComponent >() ),
    NormalizedDistanceOnSpline( normalized_distance_on_spline ),
    bEnableMovement( it_enables_movement ),
    bLoops( loops ),
    SpeedProviderClassOverride( speed_provider_class_override ),
    bAttachToSpline( it_attaches_to_spline ),
    bOverrideRotationSpeed( it_overrides_rotation_speed ),
    RotationSpeedOverride( rotation_speed_override )
{}

FSFFollowSplineInfos::FSFFollowSplineInfos( USplineComponent * const spline_component, const float normalized_distance_on_spline, const bool it_enables_movement, const bool loops, TSubclassOf< USFSplineSpeedProvider > speed_provider_class_override, const bool it_attaches_to_spline, const bool it_overrides_rotation_speed, const float rotation_speed_override ) :
    SplineComponent( spline_component ),
    NormalizedDistanceOnSpline( normalized_distance_on_spline ),
    bEnableMovement( it_enables_movement ),
    bLoops( loops ),
    SpeedProviderClassOverride( speed_provider_class_override ),
    bAttachToSpline( it_attaches_to_spline ),
    bOverrideRotationSpeed( it_overrides_rotation_speed ),
    RotationSpeedOverride( rotation_speed_override )
{}

bool FSFFollowSplineInfos::NetSerialize( FArchive & archive, UPackageMap * /* package_map */, bool & success )
{
    archive << SplineComponent;
    archive << NormalizedDistanceOnSpline;

    auto value = bEnableMovement;
    archive.SerializeBits( &value, 1 );

    archive << SpeedProviderClassOverride;

    value = bAttachToSpline;
    archive.SerializeBits( &value, 1 );

    value = bOverrideRotationSpeed;
    archive.SerializeBits( &value, 1 );

    archive << RotationSpeedOverride;

    success = true;
    return true;
}