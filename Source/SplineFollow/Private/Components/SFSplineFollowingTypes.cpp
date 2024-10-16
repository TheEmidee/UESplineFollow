#include "SplineFollow/Public/Components/SFSplineFollowingTypes.h"

#include "Components/SFSplineComponent.h"

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

FSFSplineMarkerProcessor::FSFSplineMarkerProcessor() :
    LastProcessedMarkerIndex( INDEX_NONE ),
    bUpdateLastProcessedMarker( false )
{
}

void FSFSplineMarkerProcessor::Initialize( USplineComponent * spline_component )
{
    SplineComponent = spline_component;
    Reset();
}

void FSFSplineMarkerProcessor::ProcessSplineMarkers( const float distance_on_spline, const float current_speed, AActor * owner )
{
    if ( current_speed == 0.0f )
    {
        return;
    }

    const auto * spline_component = Cast< USFSplineComponent >( SplineComponent );
    if ( spline_component == nullptr )
    {
        return;
    }

    const auto & spline_marker_proxies = spline_component->GetSplineMarkerProxies();
    const auto spline_length = spline_component->GetSplineLength();

    int first_index;

    if ( current_speed > 0.0f )
    {
        first_index = LastProcessedMarkerIndex == INDEX_NONE
                          ? 0
                          : LastProcessedMarkerIndex + 1;
    }
    else
    {
        first_index = LastProcessedMarkerIndex == INDEX_NONE
                          ? spline_marker_proxies.Num() - 1
                          : LastProcessedMarkerIndex - 1;
    }

    const auto step = current_speed > 0.0f ? 1 : -1;

    for ( auto index = first_index; index >= 0 && index < spline_marker_proxies.Num(); index += step )
    {
        const auto & marker_proxy = spline_marker_proxies[ index ];
        const auto marker_distance = spline_length * marker_proxy.SplineNormalizedDistance;

        if ( distance_on_spline * step >= marker_distance * step )
        {
            // Call the function after as it may attach the actor on another spline.
            // This would reset LastProcessedMarkerIndex, but we would override the value after
            LastProcessedMarkerIndex = index;
            marker_proxy.Function( owner );

            continue;
        }

        break;
    }
}

void FSFSplineMarkerProcessor::UpdateLastProcessedMarker( const float distance_on_spline, const float current_speed )
{
    if ( current_speed == 0.0f )
    {
        bUpdateLastProcessedMarker = true;
        return;
    }

    const auto * spline_component = Cast< USFSplineComponent >( SplineComponent );

    const auto & spline_marker_proxies = spline_component->GetSplineMarkerProxies();
    const auto spline_length = spline_component->GetSplineLength();

    const auto check_multiplier = current_speed > 0.0f ? 1.0f : -1.0f;

    for ( auto index = 0; index < spline_marker_proxies.Num(); ++index )
    {
        const auto & marker_proxy = spline_marker_proxies[ index ];
        const auto marker_distance = spline_length * marker_proxy.SplineNormalizedDistance;

        if ( marker_distance * check_multiplier <= distance_on_spline * check_multiplier )
        {
            LastProcessedMarkerIndex = index;

            if ( check_multiplier < 0.0f )
            {
                break;
            }
        }
        else if ( check_multiplier > 0.0f )
        {
            break;
        }
    }
}

void FSFSplineMarkerProcessor::TryUpdateLastProcessedMarker( const float distance_on_spline, const float current_speed )
{
    if ( bUpdateLastProcessedMarker && current_speed != 0.0f )
    {
        UpdateLastProcessedMarker( distance_on_spline, current_speed );
        bUpdateLastProcessedMarker = false;
    }
}

void FSFSplineMarkerProcessor::Reset()
{
    LastProcessedMarkerIndex = INDEX_NONE;
    bUpdateLastProcessedMarker = false;
}