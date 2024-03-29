#include "Components/SFSplineMarkers.h"

#include <Misc/DataValidation.h>

bool USFSplineMarkerActorFilter::CanSelectActor_Implementation( AActor * /* actor */ )
{
    return false;
}

UWorld * USFSplineMarkerActorFilter::GetWorld() const
{
    if ( IsTemplate() )
    {
        return nullptr;
    }

    if ( const auto * outer = GetOuter() )
    {
        return outer->GetWorld();
    }

    return nullptr;
}

bool USFSplineMarkerActorFilter_SelectAll::CanSelectActor_Implementation( AActor * /*actor*/ )
{
    return true;
}

bool USFSplineMarkerActorFilter_SelectByClass::CanSelectActor_Implementation( AActor * actor )
{
    if ( ClassToImplement == nullptr )
    {
        return true;
    }

    return actor->IsA( ClassToImplement );
}

#if WITH_EDITOR
EDataValidationResult USFSplineMarkerActorFilter_SelectByClass::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    if ( ClassToImplement == nullptr )
    {
        context.AddError( FText::FromString( TEXT( "ClassToImplement is null" ) ) );
    }

    return context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

void USFSplineMarkerAction::ProcessAction( AActor * actor, const ESFSplineMarkerProcessActionType action_type, const FSFSplineMarkerInfos & marker_infos ) const
{
    for ( const auto & actor_filter_class : ActorFilters )
    {
        if ( actor_filter_class == nullptr )
        {
            continue;
        }

        if ( !actor_filter_class->GetDefaultObject< USFSplineMarkerActorFilter >()->CanSelectActor( actor ) )
        {
            return;
        }
    }

    switch ( action_type )
    {
        case ESFSplineMarkerProcessActionType::Single:
        {
            ExecuteAction( actor, marker_infos );
        }
        break;
        case ESFSplineMarkerProcessActionType::WindowStart:
        {
            ExecuteStartWindowAction( actor, marker_infos );
        }
        break;
        case ESFSplineMarkerProcessActionType::WindowEnd:
        {
            ExecuteEndWindowAction( actor, marker_infos );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

UWorld * USFSplineMarkerAction::GetWorld() const
{
    if ( IsTemplate() )
    {
        return nullptr;
    }

    if ( const auto * outer = GetOuter() )
    {
        return outer->GetWorld();
    }

    return nullptr;
}

void USFSplineMarkerAction::ExecuteAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void USFSplineMarkerAction::ExecuteStartWindowAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void USFSplineMarkerAction::ExecuteEndWindowAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void ASFSplineMarkerLevelActor::ProcessAction( AActor * actor, const ESFSplineMarkerProcessActionType action_type, const FSFSplineMarkerInfos & marker_infos ) const
{
    for ( const auto & actor_filter_class : ActorFilters )
    {
        if ( actor_filter_class == nullptr )
        {
            continue;
        }

        if ( !actor_filter_class->GetDefaultObject< USFSplineMarkerActorFilter >()->CanSelectActor( actor ) )
        {
            return;
        }
    }

    switch ( action_type )
    {
        case ESFSplineMarkerProcessActionType::Single:
        {
            ExecuteAction( actor, marker_infos );
        }
        break;
        case ESFSplineMarkerProcessActionType::WindowStart:
        {
            ExecuteStartWindowAction( actor, marker_infos );
        }
        break;
        case ESFSplineMarkerProcessActionType::WindowEnd:
        {
            ExecuteEndWindowAction( actor, marker_infos );
        }
        break;
        default:
        {
            checkNoEntry();
            break;
        }
    }
}

void ASFSplineMarkerLevelActor::ExecuteAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void ASFSplineMarkerLevelActor::ExecuteStartWindowAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ ) const
{
}

void ASFSplineMarkerLevelActor::ExecuteEndWindowAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ ) const
{
}

bool FSFSplineMarker::IsValid() const
{
    return ItIsEnabled;
}

void FSFSplineMarker::AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & /*proxies*/ ) const
{
}

UTexture2D * FSFSplineMarker::GetSprite() const
{
    return nullptr;
}

bool FSFSplineMarker_Static::IsValid() const
{
    return Super::IsValid() && ActionClass != nullptr;
}

void FSFSplineMarker_Static::AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies ) const
{
    const auto * marker_action = ActionClass->GetDefaultObject< USFSplineMarkerAction >();

    switch ( Infos.Type )
    {
        case ESFSplineMarkerType::Single:
        {
            proxies.Emplace( FSFSplineMarkerProxy( Infos.SingleActionNormalizedSplineDistance, [ marker_action, infos = Infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESFSplineMarkerProcessActionType::Single, infos );
            } ) );
        }
        break;
        case ESFSplineMarkerType::Window:
        {
            proxies.Emplace( FSFSplineMarkerProxy( Infos.WindowStartNormalizedSplineDistance, [ marker_action, infos = Infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESFSplineMarkerProcessActionType::WindowStart, infos );
            } ) );

            proxies.Emplace( FSFSplineMarkerProxy( Infos.WindowEndNormalizedSplineDistance, [ marker_action, infos = Infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESFSplineMarkerProcessActionType::WindowEnd, infos );
            } ) );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

UTexture2D * FSFSplineMarker_Static::GetSprite() const
{
    if ( ActionClass != nullptr )
    {
        if ( const auto * marker_action = ActionClass->GetDefaultObject< USFSplineMarkerAction >() )
        {
            return marker_action->GetSprite();
        }
    }

    return Super::GetSprite();
}

FSFSplineMarker_LevelActor::FSFSplineMarker_LevelActor()
{
    LevelActor = nullptr;
}

bool FSFSplineMarker_LevelActor::IsValid() const
{
    return Super::IsValid() && LevelActor != nullptr;
}

void FSFSplineMarker_LevelActor::AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies ) const
{
    switch ( Infos.Type )
    {
        case ESFSplineMarkerType::Single:
        {
            proxies.Emplace( FSFSplineMarkerProxy( Infos.SingleActionNormalizedSplineDistance, [ level_actor = LevelActor, infos = Infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESFSplineMarkerProcessActionType::Single, infos );
            } ) );
        }
        break;
        case ESFSplineMarkerType::Window:
        {
            proxies.Emplace( FSFSplineMarkerProxy( Infos.WindowStartNormalizedSplineDistance, [ level_actor = LevelActor, infos = Infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESFSplineMarkerProcessActionType::WindowStart, infos );
            } ) );

            proxies.Emplace( FSFSplineMarkerProxy( Infos.WindowEndNormalizedSplineDistance, [ level_actor = LevelActor, infos = Infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESFSplineMarkerProcessActionType::WindowEnd, infos );
            } ) );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

UTexture2D * FSFSplineMarker_LevelActor::GetSprite() const
{
    if ( LevelActor != nullptr )
    {
        return LevelActor->GetSprite();
    }

    return Super::GetSprite();
}