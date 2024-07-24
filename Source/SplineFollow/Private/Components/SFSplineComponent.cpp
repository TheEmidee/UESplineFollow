#include "Components/SFSplineComponent.h"

#include "Components/SFSplineMarkers.h"

#include <Logging/MessageLog.h>

#if WITH_EDITOR
#include <Misc/UObjectToken.h>
#endif

TArray< FSFSplineMarker > USFSplineComponent::GetMarkersByObjectType( TSubclassOf< USFSplineMarkerObject > object_type ) const
{
    TArray< FSFSplineMarker > markers;

    for ( const auto & marker : SplineMarkers )
    {
        if ( marker.Object.IsA( object_type ) )
        {
            markers.Add( marker );
        }
    }

    return markers;
}

TArray< FSFSplineMarker > USFSplineComponent::GetActionMarkers() const
{
    return GetMarkersByObjectType( USFSplineMarkerObject_Action::StaticClass() );
}

TArray< FSFSplineMarker > USFSplineComponent::GetLevelActorMarkers() const
{
    return GetMarkersByObjectType( USFSplineMarkerObject_LevelActor::StaticClass() );
}

TArray< FSFSplineMarker > USFSplineComponent::GetDataMarkers() const
{
    return GetMarkersByObjectType( USFSplineMarkerObject_Data::StaticClass() );
}

void USFSplineComponent::OnRegister()
{
    Super::OnRegister();

    const auto fill_proxies = []( TArray< FSFSplineMarkerProxy > & proxies, const auto & markers ) {
        for ( const auto & marker : markers )
        {
            if ( marker.IsValid() )
            {
                marker.AddSplineMarkerProxies( proxies );
            }
        }
    };

    fill_proxies( SplineMarkerProxies, SplineMarkers );

    SplineMarkerProxies.Sort( []( const auto & left, const auto & right ) {
        return left.SplineNormalizedDistance < right.SplineNormalizedDistance;
    } );
}

void USFSplineComponent::UpdateSpline()
{
    const auto old_length = GetSplineLength();

    Super::UpdateSpline();

    const auto new_length = GetSplineLength();

    const auto recompute_normalized_distance = [ old_length, new_length ]( const auto current_distance ) {
        return FMath::Clamp( current_distance * old_length / new_length, 0.0f, 1.0f );
    };

    const auto set_new_distances = [ recompute_normalized_distance ]( auto & infos ) {
        infos.SingleActionNormalizedSplineDistance = recompute_normalized_distance( infos.SingleActionNormalizedSplineDistance );
        infos.WindowEndNormalizedSplineDistance = recompute_normalized_distance( infos.WindowEndNormalizedSplineDistance );
        infos.WindowStartNormalizedSplineDistance = recompute_normalized_distance( infos.WindowStartNormalizedSplineDistance );
    };

    for ( auto & marker : SplineMarkers )
    {
        set_new_distances( marker.Infos );
    }
}

#if WITH_EDITOR
FSFSplineMarker USFSplineComponent::CreateMarkerFromDefault( const FSFSplineMarker & default_marker )
{
    return FSFSplineMarker(
        default_marker.Name,
        default_marker.ItIsEnabled,
        default_marker.Infos,
        NewObject< USFSplineMarkerObject >( this, default_marker.Object->GetClass(), NAME_None, RF_NoFlags, default_marker.Object ) );
}

void USFSplineComponent::SaveSplineMarkers( const TArray< FSFSplineMarker > & markers_to_save )
{
    SplineMarkers.Reset();

    for ( const auto & marker : markers_to_save )
    {
        if ( marker.Object != nullptr )
        {
            SplineMarkers.Emplace( CreateMarkerFromDefault( marker ) );
        }
    }
}

void USFSplineComponent::CheckForErrors()
{
    Super::CheckForErrors();

    if ( IsTemplate() )
    {
        return;
    }

    for ( const auto & marker : SplineMarkers )
    {
        FString error_message( "" );

        ON_SCOPE_EXIT
        {
            if ( !error_message.IsEmpty() )
            {
                FMessageLog( "MapCheck" )
                    .Error()
                    ->AddToken( FUObjectToken::Create( this ) )
                    ->AddToken( FTextToken::Create( FText::FromString( error_message ) ) );
            }
        };

        if ( marker.Object == nullptr )
        {
            error_message = TEXT( "has invalid MarkeObject" );
            continue;
        }

        if ( const auto * action_object = Cast< USFSplineMarkerObject_Action >( marker.Object ) )
        {
            if ( action_object->ActionClass == nullptr )
            {
                error_message = TEXT( "has invalid ActionClass" );
                continue;
            }
        }

        if ( const auto * action_object = Cast< USFSplineMarkerObject_LevelActor >( marker.Object ) )
        {
            if ( action_object->LevelActor == nullptr )
            {
                error_message = TEXT( "has invalid Level Actor" );
                continue;
            }
        }

        if ( const auto * action_object = Cast< USFSplineMarkerObject_Data >( marker.Object ) )
        {
            if ( action_object->Data == nullptr )
            {
                error_message = TEXT( "has invalid Data" );
                continue;
            }
        }
    }
}
#endif
