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

FText USFSplineMarkerAction::GetStartDataText_Implementation() const
{
    return FText::GetEmpty();
}

FText USFSplineMarkerAction::GetEndDataText_Implementation() const
{
    return GetStartDataText();
}

void USFSplineMarkerAction::ProcessAction( AActor * actor, const ESFSplineMarkerProcessActionType action_type, const FSFSplineMarkerInfos & marker_infos )
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

void USFSplineMarkerAction::ExecuteAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ )
{
}

void USFSplineMarkerAction::ExecuteStartWindowAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ )
{
}

void USFSplineMarkerAction::ExecuteEndWindowAction_Implementation( AActor * /*actor*/, const FSFSplineMarkerInfos & /*marker_infos*/ )
{
}

FText ASFSplineMarkerLevelActor::GetStartDataText_Implementation() const
{
    return FText::GetEmpty();
}
FText ASFSplineMarkerLevelActor::GetEndDataText_Implementation() const
{
    return GetStartDataText();
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

FText USFSplineMarkerData::GetStartDataText_Implementation() const
{
    return FText::GetEmpty();
}

FText USFSplineMarkerData::GetEndDataText_Implementation() const
{
    return GetStartDataText();
}

FText USFSplineMarkerObject::GetStartDataText() const
{
    return FText::GetEmpty();
}

FText USFSplineMarkerObject::GetEndDataText() const
{
    return GetStartDataText();
}

void USFSplineMarkerObject::AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & /*proxies*/, const FSFSplineMarkerInfos & /*infos*/ ) const
{
}

FText USFSplineMarkerObject_Action::GetStartDataText() const
{
    return ActionClass != nullptr ? ActionClass->GetStartDataText() : Super::GetStartDataText();
}

FText USFSplineMarkerObject_Action::GetEndDataText() const
{
    return ActionClass != nullptr ? ActionClass->GetEndDataText() : Super::GetEndDataText();
}

void USFSplineMarkerObject_Action::AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies, const FSFSplineMarkerInfos & infos ) const
{
    if ( ActionClass == nullptr )
    {
        return;
    }

    auto * marker_action = ActionClass.Get();

    switch ( infos.Type )
    {
        case ESFSplineMarkerType::Single:
        {
            proxies.Emplace( FSFSplineMarkerProxy( infos.SingleActionNormalizedSplineDistance, [ marker_action, infos = infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESFSplineMarkerProcessActionType::Single, infos );
            } ) );
        }
        break;
        case ESFSplineMarkerType::Window:
        {
            proxies.Emplace( FSFSplineMarkerProxy( infos.WindowStartNormalizedSplineDistance, [ marker_action, infos = infos ]( AActor * actor ) {
                marker_action->ProcessAction( actor, ESFSplineMarkerProcessActionType::WindowStart, infos );
            } ) );

            proxies.Emplace( FSFSplineMarkerProxy( infos.WindowEndNormalizedSplineDistance, [ marker_action, infos = infos ]( AActor * actor ) {
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

FText USFSplineMarkerObject_LevelActor::GetStartDataText() const
{
    return LevelActor != nullptr ? LevelActor->GetStartDataText() : Super::GetStartDataText();
}

FText USFSplineMarkerObject_LevelActor::GetEndDataText() const
{
    return LevelActor != nullptr ? LevelActor->GetEndDataText() : Super::GetEndDataText();
}

void USFSplineMarkerObject_LevelActor::AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies, const FSFSplineMarkerInfos & infos ) const
{
    if ( LevelActor == nullptr )
    {
        return;
    }

    switch ( infos.Type )
    {
        case ESFSplineMarkerType::Single:
        {
            proxies.Emplace( FSFSplineMarkerProxy( infos.SingleActionNormalizedSplineDistance, [ level_actor = LevelActor, infos = infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESFSplineMarkerProcessActionType::Single, infos );
            } ) );
        }
        break;
        case ESFSplineMarkerType::Window:
        {
            proxies.Emplace( FSFSplineMarkerProxy( infos.WindowStartNormalizedSplineDistance, [ level_actor = LevelActor, infos = infos ]( AActor * actor ) {
                level_actor->ProcessAction( actor, ESFSplineMarkerProcessActionType::WindowStart, infos );
            } ) );

            proxies.Emplace( FSFSplineMarkerProxy( infos.WindowEndNormalizedSplineDistance, [ level_actor = LevelActor, infos = infos ]( AActor * actor ) {
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

FText USFSplineMarkerObject_Data::GetStartDataText() const
{
    return Data != nullptr ? Data->GetStartDataText() : Super::GetStartDataText();
}

FText USFSplineMarkerObject_Data::GetEndDataText() const
{
    return Data != nullptr ? Data->GetEndDataText() : Super::GetEndDataText();
}

bool FSFSplineMarker::IsValid() const
{
    return ItIsEnabled;
}

void FSFSplineMarker::AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies ) const
{
    if ( Object != nullptr )
    {
        Object->AddSplineMarkerProxies( proxies, Infos );
    }
}
